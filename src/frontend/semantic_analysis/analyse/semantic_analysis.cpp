#if 0
#include <prism/frontend/semantic_analysis/semantic_analysis.hpp>
//=====================================================================
#include <set>
#include <map>
//=====================================================================
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
#include <prism/frontend/semantic_analysis/resolve/primitive.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
//=====================================================================
using namespace prism;



struct are_same_sort_pred {
	bool operator ()(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs) const {
		ATMA_ASSERT(lhs->id == ID::identifier);
		ATMA_ASSERT(rhs->id == ID::identifier);
		return false; //marshall::identifier::semantics::variable_definition(lhs).parseme < marshall::identifier::semantics::variable_definition(rhs).parseme;
	}
};
typedef std::set<sooty::parseme_ptr, are_same_sort_pred> parseme_set_t;


struct is_identifier
{
	bool operator ()(sooty::const_parseme_ptr_ref N) const
	{
		return N->id == ID::identifier;
	}
};

void find_mutated_identifiers(parseme_set_t& results, sooty::const_parseme_ptr_ref N)
{
	switch (N->id)
	{
		case ID::variable_definition:
			results.erase(N);
			results.insert(N);
			break;
			
		case ID::block:
			std::for_each(N->children.begin(), N->children.end(), boost::bind(find_mutated_identifiers, boost::ref(results), _1));
			break;
		
		case ID::assignment:
		{
			sooty::parseme_ptr iden = sooty::depth_first_find_first_if(N->children[0], is_identifier());
			if (iden) {
				results.erase(iden);
				results.insert(iden);
			}
			break;
		}
		
		case ID::if_statement:
			find_mutated_identifiers(results, N->children[1]);
			find_mutated_identifiers(results, N->children[2]);
			find_mutated_identifiers(results, N->children[3]);
			break;
		
		case ID::phi:
		{
			sooty::parseme_ptr iden = N->children[0];
			if (iden) {
				results.erase(iden);
				results.insert(iden);
			}
			break;
		}
		
		case ID::return_statement:
		{
			results.clear();
		}
	}
}


typedef std::map<sooty::parseme_ptr, std::pair<int, sooty::parseme_ptr> > use_count_t;

struct ssa_converter_global_scope
{
	use_count_t identifiers;
};

typedef boost::shared_ptr<ssa_converter_global_scope> ssa_converter_global_scope_ptr;

namespace convert_action {
	enum Enum {
		update,
		mutate
	};
};

struct ssa_identifier_converter
{
	convert_action::Enum action;
	use_count_t scoped;
	ssa_converter_global_scope_ptr global_scope_;
	
	ssa_identifier_converter(convert_action::Enum action = convert_action::update)
		: action(action)
	{
		global_scope_ = ssa_converter_global_scope_ptr(new ssa_converter_global_scope);
		scoped = global_scope_->identifiers;
	}
	
	ssa_identifier_converter(ssa_identifier_converter* parent, convert_action::Enum action)
		: action(action), scoped(parent->scoped), global_scope_(parent->global_scope_)
	{
	}
	
	ssa_identifier_converter& change_action(convert_action::Enum action) {
		this->action = action;
		return *this;
	}
	
	ssa_identifier_converter branch(convert_action::Enum action = convert_action::update) {
		return ssa_identifier_converter(this, action);
	}
	
	void operator ()(sooty::const_parseme_ptr_ref N)
	{
		ATMA_ASSERT(N->id == ID::identifier);
		sooty::const_parseme_ptr_ref vd = resolve::identifier_to_variable_definition(N);
		
		if (this->action == convert_action::mutate) {
			N->value.integer = ++global_scope_->identifiers[vd].first;
			global_scope_->identifiers[vd].second = N;
			scoped[vd] = global_scope_->identifiers[vd];
		}
		else {
			N->value.integer = scoped[vd].first;
		}
	}
};

void convert_to_ssa_impl( sooty::parseme_ptr_ref N, ssa_identifier_converter& iden_converter )
{
	switch (N->id)
	{
		case prism::ID::root:
			std::for_each(N->children.begin(), N->children.end(), boost::bind(convert_to_ssa_impl, _1, boost::ref(iden_converter) ) );
			break;
			
		case prism::ID::block:
		{
			if (N->value.string.empty())
				N->value.string = std::string("block_") + boost::lexical_cast<std::string>(static_cast<unsigned int>(N->position.stream));
			
			for (sooty::parseme_container::iterator i = N->children.begin(); i != N->children.end(); )
			{
				sooty::parseme_ptr_ref child = *i;
				if ( child->id == ID::variable_definition && marshall::variable_definition::has_defvalue(child) ) {
					sooty::parseme_ptr assignment = sooty::make_parseme(N, ID::assignment, sooty::value_t());
					// add lvalue to assignment and add vd to lvalue
					assignment->children.push_back( sooty::make_parseme(assignment, ID::identifier, sooty::value_t( marshall::variable_definition::name(child)->value.string )) );
					assignment->children[0]->children.push_back(child);
					// add rvalue to assignment
					assignment->children.push_back( child->children[2] );
					child->children.pop_back();
					i = N->children.insert(i + 1, assignment);
				}
				else {
					convert_to_ssa_impl(*i, iden_converter);
					++i;
				}
			}
			break;
		}
		
		case prism::ID::function:
			convert_to_ssa_impl( marshall::function::body(N), iden_converter );
			break;
		
		case prism::ID::function_call:
			std::for_each(  marshall::function_call::argument_list(N)->children.begin(), marshall::function_call::argument_list(N)->children.end(), boost::bind(convert_to_ssa_impl, _1, boost::ref(iden_converter.change_action(convert_action::mutate))) );
			break;
		
		case prism::ID::address_of:
			convert_to_ssa_impl(N->children[0], iden_converter);
			break;
		
		case prism::ID::assignment:
			// do the rhs first so that we can do stuff like "a = a + 1"
			convert_to_ssa_impl( N->children[1], iden_converter.change_action(convert_action::update) );
			convert_to_ssa_impl( N->children[0], iden_converter.change_action(convert_action::mutate) );
			iden_converter.change_action(convert_action::update);
			break;
			
		case prism::ID::return_statement:
			if (!N->children.empty())
				convert_to_ssa_impl( N->children[0], iden_converter.change_action(convert_action::update) );
			iden_converter.change_action(convert_action::update);
			break;
		
		case prism::ID::dereference:
			convert_to_ssa_impl( N->children[0], iden_converter );
			break;
		
		case prism::ID::loop_statement:
			if (N->children[1]->id == ID::block) {
				N->children[1]->value.string = atma::string_builder("loop_")(N->position.stream)("_body");
				convert_to_ssa_impl(N->children[1], iden_converter);
			}
			break;
		
		// gotta add phi nodes
		case prism::ID::if_statement:
		{
			convert_to_ssa_impl( N->children[0], iden_converter );
			N->children[1]->value.string = atma::string_builder("if_")(N->position.stream)("_true");
			convert_to_ssa_impl( N->children[1], iden_converter.branch() );
			N->children[2]->value.string = atma::string_builder("if_")(N->position.stream)("_false");
			convert_to_ssa_impl( N->children[2], iden_converter.branch() );
			
			N->children.push_back( sooty::parseme::create(N, ID::block, sooty::value_t(atma::string_builder("if_")(N->position.stream)("_cont")), sooty::lexical_position()) );
			
			sooty::parseme_ptr_ref ifcont = N->children[3];
			
			parseme_set_t true_block;
			find_mutated_identifiers(true_block, N->children[1]);
			
			parseme_set_t false_block;
			find_mutated_identifiers(false_block, N->children[2]);
			
			parseme_set_t::iterator true_block_iter = true_block.begin();
			parseme_set_t::iterator false_block_iter = false_block.begin();
			
			while ( true_block_iter != true_block.end() && false_block_iter != false_block.end() )
			{
				sooty::parseme_ptr true_block_iden, false_block_iden;
				
				if (marshall::identifier::semantics::variable_definition(*true_block_iter).parseme < marshall::identifier::semantics::variable_definition(*false_block_iter).parseme) {
					++true_block_iter;
					continue;
				}
				else if (marshall::identifier::semantics::variable_definition(*false_block_iter).parseme < marshall::identifier::semantics::variable_definition(*true_block_iter).parseme) {
					++false_block_iter;
					continue;
				}
				else {
					true_block_iden = *true_block_iter;
					false_block_iden = *false_block_iter;
					++true_block_iter;
					++false_block_iter;
				}
				
				// phi node
				sooty::parseme_ptr phi = make_parseme(N->children[3], ID::phi, sooty::value_t());
				
				// identifier
				sooty::parseme_ptr phi_iden = make_parseme(phi, ID::identifier, true_block_iden->value);
				phi_iden->children.push_back( marshall::identifier::semantics::variable_definition(true_block_iden).parseme );
				iden_converter.change_action(convert_action::mutate)(phi_iden);
				iden_converter.change_action(convert_action::update);
				
				phi->children.push_back( phi_iden );
				phi->children.push_back( true_block_iden );
				phi->children.push_back( false_block_iden );
				
				N->children[3]->children.push_back(phi);
			}
			
			break;
		}
		
		
		case prism::ID::mul:
		case prism::ID::div:
		case prism::ID::add:
		case prism::ID::sub:
		case prism::ID::equ:
		case prism::ID::lt:
			convert_to_ssa_impl( N->children[0], iden_converter );
			convert_to_ssa_impl( N->children[1], iden_converter );
			break;
		
		case prism::ID::identifier:
		{
			//iden_converter(N);
			sooty::const_parseme_ptr_ref vd = resolve::identifier_to_variable_definition(N);
			N->value.integer = ++vd->value.integer;
			break;
		}
	}
}

void prism::convert_to_ssa( sooty::parseme_ptr_ref N )
{
	//sooty::depth_first_for_each(N, setup_semantic_information);
	//sooty::depth_first_for_each(N, convert_to_ssa_impl);
	//convert_to_ssa_impl(N, ssa_identifier_converter());
}
#endif