#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <sstream>
//=====================================================================
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/decorate.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/backend/analysis/inline.hpp>
#include <prism/backend/generation.hpp>




//=====================================================================
// using and abusing
//=====================================================================
using namespace prism;




//=====================================================================
// begin!
//=====================================================================
void prism::generate_llvm(std::ostream& out_stream, sooty::const_parseme_ptr_ref N)
{
	llvm::detail::outstream() = &out_stream;
	llvm::module(N);
}

namespace {
	void llvm_typedef(sooty::const_parseme_ptr_ref td)
	{
		using namespace prism::llvm;
		if (marshall::type_definition::name(td)->value.string == "void")
			return;
		else if (marshall::type_definition::name(td)->value.string == "varags")
			return;
		else if (td->value.integer == 0)
			return;
		output_stream() << "%" << marshall::type_definition::name(td)->value.string << "_t = type " 
			<< logical_type_name( marshall::type_definition::members(td) ) << std::endl;
	}
}

void llvm::module(sooty::const_parseme_ptr_ref N)
{
	std::sort(N->children.begin(), N->children.end(), sooty::id_less_than_pred());
	
	// find all string-constants!
	output_stream() << "; string-constants" << std::endl;
	sooty::parseme_container string_constants;
	sooty::depth_first_copy_if( std::back_inserter(string_constants), N, sooty::id_matches(ID::string_literal) );
	for (sooty::parseme_container::iterator sci = string_constants.begin(); sci != string_constants.end(); ++sci)
	{
		sooty::const_parseme_ptr_ref string_constant = *sci;
		std::string temp = string_constant->value.string;
		boost::algorithm::replace_all(temp, "\n", "\\0d\\0a");
		output_stream() << lvalue_name(string_constant) << " = internal constant " 
			<< logical_type_name(string_constant) << " c\"" << temp << "\\00\"" << std::endl;
	}
	output_stream() << std::endl << std::endl;
	
	
	// aliases
	output_stream() << "; type-aliases" << std::endl;
	sooty::parseme_container type_definitions;
	sooty::depth_first_copy_if( std::back_inserter(type_definitions), N, sooty::id_matches(ID::type_definition) );
	std::for_each(type_definitions.begin(), type_definitions.end(), llvm_typedef);
	output_stream() << std::endl << std::endl;
	
	
	output_stream() << "; functions" << std::endl;
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> bounds =
		std::equal_range(N->children.begin(), N->children.end(), ID::type_definition, sooty::id_less_than_pred());
	std::for_each(bounds.first, bounds.second, llvm::type_definition);
	output_stream() << std::endl << std::endl;
	

	// functions
	bounds = std::equal_range(N->children.begin(), N->children.end(), ID::function, sooty::id_less_than_pred());
	std::for_each(bounds.first, bounds.second, llvm::function);
}

void llvm::parameter_list(sooty::const_parseme_ptr_ref N)
{
	
}

void llvm::type_definition(sooty::const_parseme_ptr_ref definition)
{
	ATMA_ASSERT(definition->id == ID::type_definition);
	
	sooty::const_parseme_ptr_ref members = marshall::type_definition::members(definition);
	
	for (sooty::parseme_container::const_iterator member_iter = members->children.begin(); 
		member_iter != members->children.end(); ++member_iter)
	{
		sooty::const_parseme_ptr_ref member = *member_iter;
		
		if (member->id == ID::function) {
			llvm::function(member);
		}
	}
	
}



bool is_block(sooty::const_parseme_ptr_ref N)
{
	return N->id == ID::block;
}



bool is_return_statement(sooty::const_parseme_ptr_ref N)
{
	return N->id == ID::return_statement;
}

void llvm::statement(sooty::const_parseme_ptr_ref N)
{
	//sooty::const_parseme_ptr_ref N = *Ni;
	
	switch (N->id)
	{
		case ID::variable_definition:
		{
			sooty::parseme_ptr type = resolve::type_of( marshall::variable_definition::type(N) );
			sooty::const_parseme_ptr_ref name = marshall::variable_definition::name(N);
			sooty::parseme_ptr arguments = marshall::variable_definition::arguments(N);
			
			output_stream()
				<< "%" << name->value.string << " = alloca " << logical_type_name(type) << std::endl;
			
			sooty::parseme_ptr cloned_arguments = sooty::clone_tree(arguments);
			
			// if 
			/*
			if ( type->id != ID::reference_type && type->id != ID::pointer_type && type->id != ID::array_type )
			{
				sooty::parseme_ptr fc = sooty::make_parseme(N, ID::member_function_call);
				(sooty::immediate(fc)) ((
					sooty::insert(ID::identifier, "__constructor__"),
					sooty::insert(ID::argument_list) [
						sooty::insert(ID::identifier, name->value.string),
						sooty::insert(cloned_arguments->children.begin(), cloned_arguments->children.end())
					]
				));
				if ( prism::inlining::should_intrinsically_inline(fc) ) {
					//prism::inlining::inline_function_call(fc);
					prism::inlining::results context = prism::inlining::build_inline_context(fc);
					prism::inlining::inline_void_function_call_at(N->parent.lock(), sooty::position_of(N), context);
				}
				else {
					member_function_call(fc);
				}
			}
			*/
			
			/*
			SEROIUSLY, add this back in
			if ( marshall::variable_definition::has_defvalue(N) )
			{
				sooty::const_parseme_ptr_ref defvalue = marshall::variable_definition::defvalue(N);
				pre_access_expression(defvalue);
			}
			*/
			break;
		}
		
		case ID::return_statement:
		{
			if (!N->children.empty())
			{
				sooty::const_parseme_ptr_ref expression = marshall::return_statement::expression(N);
				pre_access_expression(expression);
				
				//if ( marshall::function::name(function)->value.string == "main" || function->value.integer == 1 ) {	
				//}
				
				// the main function returns an int, that's fine. others do a RNVO
				sooty::parseme_ptr function = sooty::direct_upwards_find_first_if(N, sooty::id_matches(ID::function));
				sooty::const_parseme_ptr_ref return_type = marshall::function::return_type(function);
				if ( inspect::can_fit_in_register(return_type) ) {
					output_stream() << "ret " + logical_type_name(expression) + " " << rvalue_name(expression) << '\n';
				}
				else {
					output_stream() << "store " << logical_type_name(expression) + " " << rvalue_name(expression) << ", " << storage_type_name(return_type) << " %retv\n";
					output_stream() << "ret void\n";
				}
			}
			else {
				output_stream() << "ret void\n";
			}

			break;
		}
	
		case ID::if_statement:
		{
			sooty::const_parseme_ptr_ref expression = marshall::if_statement::expression(N);
			sooty::const_parseme_ptr_ref true_block = marshall::if_statement::true_block(N);
			sooty::const_parseme_ptr_ref false_block = marshall::if_statement::false_block(N);
			
			pre_access_expression(expression);
			
			std::string true_block_name = true_block->value.string; //atma::string_builder("block_")(true_block->position.guid());
			std::string false_block_name = false_block->value.string; //atma::string_builder("block_")(false_block->position.guid());
			std::string cont_block_name = atma::string_builder("if_")(N->position.guid())("_cont");
			
			output_stream() << "br i1 " << rvalue_name(expression) << ", label %" << true_block_name << ", label %" << false_block_name << "\n";
			
			block(true_block, true_block_name);
			bool true_found = false;
			for (sooty::parseme_container::iterator i = true_block->children.begin(); i != true_block->children.end(); ++i) {
				if ( (*i)->id == ID::return_statement ) {
					true_found = true;
				}
			}
			if (!true_found)
				output_stream() << "\tbr label %" << cont_block_name << "\n";
			
			block(false_block, false_block_name);
			bool false_found = false;
			for (sooty::parseme_container::iterator i = false_block->children.begin(); i != false_block->children.end(); ++i) {
				if ( (*i)->id == ID::return_statement ) {
					false_found = true;
				}
			}
			if (!false_found)
				output_stream() << "\tbr label %" << cont_block_name << "\n";
			
			
			if (!false_found || !true_found)
				output_stream() << cont_block_name << ":\n";
			break;
		}
		
		case ID::loop_statement:
		{
			//if (marshall::loop_statement::variable(N)->id != ID::not_present) {
			//	output_stream() << 
			//}
			
			loop(N);
			break;
		}
		
		
		case ID::delete_:
		{
			output_stream()
				<< "free " << storage_type_name(N->children.front()) << " " << lvalue_name(N->children.front()) << "\n";
			break;
		}
		
		case ID::function_call:
		case ID::member_function_call:
			pre_access_expression(N);
			break;
		
		case ID::assignment:
			pre_access_expression(N);
			break;
	}
	
	output_stream(false) << std::flush;
	//return ++Ni;
}

