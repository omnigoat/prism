#include <prism/backend/analysis/inline.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
//=====================================================================
using namespace prism;


bool prism::inlining::should_intrinsically_inline(sooty::const_parseme_ptr_ref call)
{
	if (call->id != ID::function_call && call->id != ID::member_function_call)
		return false;
	
	sooty::parseme_ptr function = resolve::function_of_function_call(call);
	ATMA_ASSERT(function);
	return function->value.integer == 1;
}


//=====================================================================
//
//  replace_identifier_to_parameter_with_argument
//  -------------------------------------------
//    we rely on the fact that when we insert the statements from the
//    function to be inlined (Fi), their parent node still ultimately
//    points to Fi, and that all the copied identifiers' semantics still
//    point to Fi's parameters. we can then replace these with the
//    appropriate arguments. afterwards, we rectify the parent nodes for
//    the top-level statements, to point to the function-block they
//    now belong to.
//
//=====================================================================
namespace {
	struct replace_identifier_to_parameter_with_argument {
		sooty::parseme_ptr parameter_list, argument_list;
		
		replace_identifier_to_parameter_with_argument(sooty::const_parseme_ptr_ref pl, sooty::const_parseme_ptr_ref al)
			: parameter_list(pl), argument_list(al)
		{
		}
		
		// we are going to find an identifier that references a parameter for a given function, and replace that
		// identifier with the corresponding argument from the function-call. remember, N is a *reference*, so
		// we can assign directly to N and it will modify the memory directly.
		bool operator ()(sooty::cuil::const_matching_context_ref context) const
		{
			sooty::parseme_ptr_ref N = *context.begin;
			
			// only identifiers
			if (N->id != ID::identifier)
				return false;
			
			// no lhs of member-operators
			if (N->parent.lock() && N->parent.lock()->id == ID::member_operator 
			  && marshall::binary_expression::lhs(N->parent.lock()) == N)
				return false;
			
			// we really need a variable-definition
			sooty::parseme_ptr parameter = resolve::identifier_to_variable_definition(N);
			if (!parameter)
				return false;
			
			// the variable-definition must be one of the parameters
			sooty::parseme_container::iterator pi = std::find(parameter_list->children.begin(), parameter_list->children.end(), parameter);
			if (pi == parameter_list->children.end())
				return false;
			
			// locate the correct argument
			size_t d = std::distance(parameter_list->children.begin(), pi);
			sooty::parseme_container::iterator argument_iter = argument_list->children.begin();
			std::advance(argument_iter, d);
			sooty::const_parseme_ptr_ref argument = *argument_iter;
			
			// make the switch!
			sooty::parseme_ptr copy_of_argument(new sooty::parseme(*argument));
			replace_parseme_with_another(N, copy_of_argument);
			return false;
		}
	};
}

namespace {
	struct replace_reference_and_identifier_to_parameter_with_argument
	{
		sooty::parseme_ptr parameter_list, argument_list;

		replace_reference_and_identifier_to_parameter_with_argument(sooty::const_parseme_ptr_ref pl, sooty::const_parseme_ptr_ref al)
			: parameter_list(pl), argument_list(al)
		{
		}
		
		bool operator ()(sooty::cuil::const_matching_context_ref context)
		{
			sooty::parseme_ptr_ref N = *context.begin;
			ATMA_ASSERT(N->id == ID::un_reference);
			sooty::cuil::matching_context_t child_context(N->children, N->children.begin(), N->children.end());
			bool passed = replace_identifier_to_parameter_with_argument(parameter_list, argument_list)(child_context);
			if (!passed) {
				return false;
			}
			
			// get rid of un-reference
			replace_parseme_with_another(N, N->children.front());
			return true;
		}
	};
}



namespace {
	// finds a statement
	sooty::parseme_ptr first_class_statement(sooty::const_parseme_ptr_ref N)
	{
		sooty::parseme_ptr current = N;
		sooty::parseme_ptr parent;
		while ( (parent = current->parent.lock()) )
		{
			if (parent->id == ID::block || parent->id == ID::if_statement || parent->id == ID::init_list)
			{
				if (current->id == ID::variable_definition) {
					sooty::parseme_container::iterator current_position = sooty::position_of(current);
					++current_position;
					current = *current_position;
				}
				
				return current;
			}
			
			current = parent;
		}
		
		return sooty::parseme_ptr();
	}
}





prism::inlining::results prism::inlining::build_inline_context(sooty::const_parseme_ptr_ref call)
{
	prism::inlining::results results;
	
	// find:
	//  - the function it references
	//  - the body of the function
	sooty::const_parseme_ptr_ref function = resolve::function_of_function_call(call);
	sooty::const_parseme_ptr_ref function_body = marshall::function::body(function);

	// we can't modify the actual function, so clone its body
	sooty::parseme_ptr cloned_function_body = sooty::clone_tree(function_body);
	cloned_function_body->parent = function_body->parent;

	// a non-void function has special considerations
	bool nonvoid_return = marshall::function::return_type(function)->value.string != "void";

	// find:
	//  - the parent node of the call (and its iterator)
	//  - the top-most statement node and its iterator (ie, the statement in the block)
	//  - if the above two are the same thing
	//sooty::parseme_ptr call_parent = call->parent.lock();
	//sooty::parseme_ptr statement = first_class_statement(call);
	//bool call_is_statement = call->parent.lock() == statement->parent.lock();

	// for each inserted statement, find an identifier that references a parameter from the old
	// function, and replace it with an argument from the original function call
	sooty::const_parseme_ptr_ref argument_list = marshall::function_call::argument_list(call);
	sooty::const_parseme_ptr_ref parameter_list = marshall::function::parameter_list(function);
	
	sooty::cuil::tree_pattern
		(
			// start from here
			cloned_function_body->children.front(), 
			
			// on the way down
			sooty::cuil::eq(ID::un_reference) [
				sooty::cuil::eq(ID::identifier)
			]
			.perform(replace_reference_and_identifier_to_parameter_with_argument(parameter_list, argument_list))
			|
			sooty::cuil::eq(ID::identifier)
			.perform(replace_identifier_to_parameter_with_argument(parameter_list, argument_list))
			
			,
			sooty::cuil::no_pattern()
		);

	sooty::cuil::tree_pattern
		(
			// start from here
			cloned_function_body->children.front(), 
			
			// don't do anything on the way down
			sooty::cuil::no_pattern(), 
			
			
			sooty::cuil::eq(ID::member_operator) [
				sooty::cuil::placeholders::_1 = sooty::cuil::any(),
				sooty::cuil::eq(ID::un_reference) [
					sooty::cuil::placeholders::_2 = sooty::cuil::any()
				]
			]
			.rewrite (
				sooty::cuil::mk(ID::un_reference) [
					sooty::cuil::mk(ID::member_operator) [
						sooty::cuil::placeholders::_1,
						sooty::cuil::placeholders::_2
					]
				]
			)
		);
		
	
	// build a list of statements to insert into the block
	results.statements_to_insert.assign(cloned_function_body->children.begin(), cloned_function_body->children.end());

	// we only need to rewrite the return statements if there's more than one return-statement
	// in the function we're inlining
	sooty::parseme_container return_statements;
	sooty::depth_first_copy_if(std::back_inserter(return_statements), cloned_function_body, sooty::id_matches(ID::return_statement));

	// if there's 
	bool require_return_rewriting = return_statements.size() > 1;
	
	std::string return_identifier = atma::string_builder("tmp")(call->position.guid());
	
	// if we're a non-void-returning function, and we're rewriting our returns (which
	// means there's been several returns in the callee), then we simply use a
	// temporary identifier to store the value of all the possible outcomes
	if (require_return_rewriting)
	{
		if (nonvoid_return)
		{
			sooty::immediate_assign(results.statements_to_insert, results.statements_to_insert.begin()) (
				sooty::insert(ID::variable_definition) [
					sooty::insert(ID::identifier, return_identifier),
						sooty::insert(prism::marshall::function::return_type(function))
				]
			);
			
			// rewrite all return statements as assignments and jumps
			tree_pattern(results.statements_to_insert, sooty::cuil::no_pattern(),
				sooty::cuil::eq(ID::return_statement) [
					sooty::cuil::placeholders::_1 = sooty::cuil::any()
				]
				.rewrite ((
					sooty::cuil::mk(ID::assignment) [
						sooty::cuil::mk(ID::identifier, return_identifier),
						sooty::cuil::placeholders::_1
					],
					sooty::cuil::mk(ID::jump, "inline_exit")
				))
			);
			
			results.expression_to_replace_call_with = sooty::make_parseme(sooty::parseme_ptr(), ID::identifier, return_identifier);
		}
		
		tree_pattern(results.statements_to_insert, sooty::cuil::no_pattern(),
			sooty::cuil::eq(ID::return_statement)
			.rewrite (
				sooty::cuil::mk(ID::jump, "inline_exit")
			)
		);
		
		results.statements_to_insert.push_back( sooty::make_parseme(sooty::parseme_ptr(), ID::label, "inlined_exit") );
	}
	// perform some deductive reasoning:
	//  - all functions must terminate at least once - at the end
	//  - any function that only terminates once has terminated at the end
	//  - any function that terminates at the end is a return-statement
	else
	{
		//sooty::parseme_container::iterator i = sooty::position_of(return_statements.front());
		//i = return_statements.front()->parent.lock()->children.erase(i);
		results.statements_to_insert.erase(
			std::remove_if(results.statements_to_insert.begin(), results.statements_to_insert.end(), sooty::id_matches(ID::return_statement)),
			results.statements_to_insert.end()
		);
		
		if (nonvoid_return) {
			results.expression_to_replace_call_with = return_statements.front()->children.front();
		}
	}
	
	return results;
}

void prism::inlining::inline_nonvoid_function_call_at(sooty::const_parseme_ptr_ref parent, const sooty::parseme_container::iterator& position, sooty::const_parseme_ptr_ref call, results& IC)
{
	ATMA_ASSERT(IC.expression_to_replace_call_with);
	sooty::replace_parseme_with_another(call, IC.expression_to_replace_call_with);
	
	sooty::parseme_ptr call_parent = call->parent.lock();
	sooty::parseme_container::iterator call_position = position_of(call);
	sooty::parseme_ptr statement = first_class_statement(call);
	sooty::parseme_ptr enclosing_scope = statement->parent.lock();
	sooty::parseme_container::iterator statement_position = position_of(statement);
	
	// must never inline stuff before the variable is actually declared! :O
	if (statement->id == ID::variable_definition)
		++statement_position;
	
	bool call_is_statement = (call_parent == statement->parent.lock());

	// remove the function-call from its parent node
	call_position = call_parent->children.erase(call_position);
	if (call_is_statement)
		statement_position = call_position;
	
	std::for_each(IC.statements_to_insert.begin(), IC.statements_to_insert.end(), sooty::set_parent_to(enclosing_scope));
}


void prism::inlining::inline_void_function_call_at(sooty::const_parseme_ptr_ref parent, const sooty::parseme_container::iterator& position, results& IC)
{
	// we definitely can't perform this sort of inlining on a function that returns a result
	ATMA_ASSERT(!IC.expression_to_replace_call_with);
	
	std::for_each(IC.statements_to_insert.begin(), IC.statements_to_insert.end(), sooty::set_parent_to(parent));
	
	parent->children.insert(position, IC.statements_to_insert.begin(), IC.statements_to_insert.end());
}

void prism::inlining::inline_function_call(sooty::const_parseme_ptr_ref call)
{
	sooty::parseme_ptr call_parent = call->parent.lock();
	ATMA_ASSERT(call_parent);
	sooty::parseme_container::iterator call_position = position_of(call);
	ATMA_ASSERT(call_position != call_parent->children.end());
	sooty::parseme_ptr statement = first_class_statement(call);
	ATMA_ASSERT(statement);
	sooty::parseme_ptr enclosing_scope = statement->parent.lock();
	ATMA_ASSERT(enclosing_scope);
	sooty::parseme_container::iterator statement_position = position_of(statement);
	ATMA_ASSERT(statement_position != enclosing_scope->children.end());
	
	results R = build_inline_context(call);
	
	// insert stuff before the statement
	enclosing_scope->children.insert(statement_position, R.statements_to_insert.begin(), R.statements_to_insert.end());
	std::for_each(R.statements_to_insert.begin(), R.statements_to_insert.end(), sooty::set_parent_to(enclosing_scope));
	
	// replace the call with another expression
	if (R.expression_to_replace_call_with) {
		sooty::replace_parseme_with_another(call, R.expression_to_replace_call_with);
	}
	else {
		call_parent->children.erase(call_position);
	}
}

