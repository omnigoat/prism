#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
#include <prism/frontend/semantic_analysis/depths.hpp>

//=====================================================================
//=====================================================================
namespace {
	void create_new_type_instance(sooty::const_parseme_ptr_ref type_definition, sooty::const_parseme_ptr_ref type_identifier)
	{
		using namespace prism;
		
		sooty::parseme_ptr member_definitions = marshall::type_definition::members(type_definition);
		sooty::parseme_container identifiers;
		sooty::depth_first_copy_if( std::back_inserter(identifiers), member_definitions, sooty::id_matches(ID::identifier) );
	}
}

//=====================================================================
// typename_matches_type_definition
//=====================================================================
prism::resolve::typename_matches_type_definition::typename_matches_type_definition( sooty::const_parseme_ptr_ref N )
	: lhs(N)
{
	switch (lhs->id)
	{
		case ID::type_identifier:
			break;
		
		case ID::pointer_type:
			lhs = marshall::pointer_type::pointee_type(N);
			break;
		
		case ID::array_type:
			lhs = marshall::pointer_type::pointee_type(N);
			break;
		
		default:
			ATMA_HALT("bad!!!");
	}
}

bool prism::resolve::typename_matches_type_definition::operator()( sooty::const_parseme_ptr_ref type_definition ) const
{
	if (type_definition->id != ID::type_definition)
		return false;
	
	switch (lhs->id)
	{
		// all type-definitions (even inbuilt ones) have a name. all instances of the primitive
		// types carry the string name of their type. therefore we can match them just as if we
		// were matching identifiers with user-defined-types.
		case ID::type_identifier:
		{
			// type's name has to match at least
			if (lhs->value.string != marshall::type_definition::name(type_definition)->value.string)
				return false;
			
			// SERIOUSLY, hack for ints
			if (marshall::type_definition::name(type_definition)->value.string == "int") {
				if ( lhs->children.empty() ) {
					lhs->children.push_back( sooty::make_parseme(lhs, ID::argument_list) );
					sooty::parseme_ptr arguments = marshall::type_identifier::arguments(lhs);
					arguments->children.push_back( sooty::make_parseme(arguments, ID::any_type, 32) );
				}
			}
			
			// if this type is an instance of a template, match the arguments
			sooty::parseme_ptr td_arguments = marshall::type_definition::arguments(type_definition);
			
			// if the type has parameters, we need to match those
			sooty::parseme_ptr parameters = marshall::type_definition::parameters(type_definition);
			
			// if the type-identifier doesn't have arguments, then if the type-defininition doesn't
			// have parameters OR arguments, then it's a match. otherwise, it's not (an error?).
			if (marshall::type_identifier::arguments(lhs)->children.empty())
				if (parameters->children.empty() && td_arguments->children.empty())
					return true;
				else
					return false;
			
			// our type-identifier has arguments
			sooty::parseme_ptr ti_arguments = marshall::type_identifier::arguments(lhs);
			
			// if the type-identifier's arguments match the arguments in the type-definition,
			// then this is a perfect match and nothing more needs to be done
			if (!td_arguments->children.empty()) {
				return std::mismatch(td_arguments->children.begin(), td_arguments->children.end(), 
				        ti_arguments->children.begin(), types_match).first == td_arguments->children.end();
			}
			// if the type-definition didn't have arguments, if could have parameters. if those
			// parameters match, then we must create a new instance of this type.
			else if (!parameters->children.empty())
			{
				if (std::mismatch(parameters->children.begin(), parameters->children.end(),
				        ti_arguments->children.begin(), types_match).first == parameters->children.end())
				{
					create_new_type_instance(type_definition, lhs);
				}
			}
			else {
				return false;
			}
			
			return true;
		}
		
		// we don't support anything else yet!
		default:
			ATMA_HALT("bad~~~!");
	}
	
	return false;
}


sooty::parseme_ptr prism::resolve::type_definition_of( sooty::const_parseme_ptr_ref N )
{
	ATMA_ASSERT(N);
	
	switch (N->id)
	{
		case ID::real_literal:
		case ID::int_literal:
			return N->children.front();
		
		case ID::identifier:
			return type_definition_of(  resolve::identifier_to_variable_definition(N)  );
		
		case ID::type_identifier:
		{
			sooty::parseme_container definitions;
			type_definitions_of(definitions, N);
			return definitions.front();
		}
		
		case ID::variable_definition:
			return type_definition_of(marshall::variable_definition::type(N));
		
		case ID::parameter:
			return type_definition_of(marshall::parameter::type(N));
		
		case ID::reference_type:
			return type_definition_of(N->children.front());
		
		case ID::type_definition:
			return N;
	}
	
	sooty::parseme_container definitions;
	type_definitions_of(definitions, N);
	
	//std::sort(definitions.begin(), definitions.end(), type_sort_pred());
	
	// currently, we simply can't have multiple definitions with the same name, at all.
	// it might be a good thing, actually.
	if (definitions.size() != 1)
		return sooty::parseme_ptr();
	
	return definitions.front();
}





void prism::resolve::type_definitions_of( sooty::parseme_container& definitions, sooty::const_parseme_ptr_ref N )
{
	sooty::upwards_bredth_first_copy_if(N, std::back_inserter(definitions), prism::resolve::typename_matches_type_definition(N), type_definition_depther());
}

namespace {
	bool is_bitwidth_type_definition(sooty::const_parseme_ptr_ref N) {
		return N->id == prism::ID::type_definition && prism::marshall::type_definition::name(N)->value.string == "bitwidth_type";
	}
}

sooty::parseme_ptr prism::resolve::bitwidth_type(sooty::const_parseme_ptr_ref N) {
	return sooty::upwards_bredth_first_find_first_if(N, is_bitwidth_type_definition);
}

sooty::parseme_ptr prism::resolve::member_of_type_definition( sooty::const_parseme_ptr_ref td, const std::string& name )
{
	ATMA_ASSERT( td->id == ID::type_definition );
	sooty::parseme_ptr members = marshall::type_definition::members(td);
	ATMA_ASSERT( members->id == ID::member_definitions );
	for (sooty::parseme_container::const_iterator i = members->children.begin(); i != members->children.end(); ++i) {
		sooty::const_parseme_ptr_ref member = *i;
		if (member->id == ID::variable_definition) {
			if ( marshall::variable_definition::name(member)->value.string == name ) {
				return member;
			}
		}
	}
	
	return sooty::parseme_ptr();
}

sooty::parseme_ptr prism::resolve::parameter_of_type_definition( sooty::const_parseme_ptr_ref td, const std::string& name )
{
	ATMA_ASSERT( td->id == ID::type_definition );
	sooty::parseme_ptr parameters = marshall::type_definition::parameters(td);
	ATMA_ASSERT( parameters->id == ID::parameter_list );
	for (sooty::parseme_container::const_iterator i = parameters->children.begin(); i != parameters->children.end(); ++i) {
		sooty::const_parseme_ptr_ref parameter = *i;
		if (parameter->id == ID::parameter) {
			if ( marshall::parameter::name(parameter)->value.string == name ) {
				return parameter;
			}
		}
	}
	
	return sooty::parseme_ptr();
}