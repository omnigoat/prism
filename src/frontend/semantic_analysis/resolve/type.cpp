#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>

//=====================================================================
// type_of
//=====================================================================
sooty::parseme_ptr prism::resolve::type_of(sooty::const_parseme_ptr_ref N)
{
	ATMA_ASSERT(N);
	
	switch (N->id)
	{
		// intrinsics
		case ID::int_type:
		case ID::real_type:
		case ID::bool_type:
		case ID::void_type:
		case ID::pointer_type:
		case ID::array_type:
		case ID::reference_type:
		case ID::any_type:
			return N;
		
		case ID::bitwidth:
			//return type_of( prism::synthesize::type_identifier(N, "bitwidth_type") );
			return prism::resolve::bitwidth_type(N);
		
		case ID::identifier:
		{
			sooty::parseme_ptr D = prism::resolve::identifier_to_variable_definition(N);
			ATMA_ASSERT(D);
			if (!D)
				return type_of( prism::resolve::identifier_to_variable_definition(N) );
			else
				return type_of(D);
		}
		
		case ID::type_identifier:
		{
			//return type_definition_of(N);
			sooty::parseme_ptr result = marshall::type_identifier::semantics::type_definition(N).parseme;
			ATMA_ASSERT(result);
			return result;
		}
		
		case ID::type_definition:
			return N;
		
		case ID::dereference:
		{
			sooty::parseme_ptr p = type_of(N->children[0]);
			ATMA_ASSERT(p->id == ID::pointer_type);
			return type_of(p->children[0]);
		}

		case ID::int_literal:
			return type_of( synthesize::int_type(N) );
		
		case ID::real_literal:
			return type_of( synthesize::real_type(N) );
		
		case ID::bool_literal:
			return synthesize::bool_type(N);
		
		case ID::string_literal:
			return synthesize::string_type(N);
			
		case ID::eq_operator:
			return type_of( N->children[0] );
		
		case ID::add_operator:
		case ID::function:
			return type_of( marshall::function::return_type(N) );
			
		case ID::function_call:
			return type_of( sooty::upwards_bredth_first_find_first_if(N, resolve::function_matches_function_call_pred(N)) );
		
		case ID::member_function_call:
			return resolve::function_of_member_function_call( N );
		
		case ID::variable_definition:
			return type_of( marshall::variable_definition::type(N) );
		
		case ID::parameter:
			return type_of( marshall::parameter::type(N) );
		
		case ID::un_reference:
			return type_of( N->children.front() );
		
		case ID::address_of:
		{
			// we need to synthesize a new type
			sooty::parseme_ptr childtype = type_of(N->children[0]);
			sooty::parseme_ptr ptype = sooty::parseme::create( sooty::parseme_ptr(), ID::pointer_type, sooty::value_t(), sooty::lexical_position() );
			ptype->children.push_back(childtype);
			return ptype;
		}
		
		case ID::new_:
		{
			// we need to synthesize a new type
			sooty::parseme_ptr childtype = type_of(N->children[0]);
			sooty::parseme_ptr ptype = sooty::parseme::create( sooty::parseme_ptr(), ID::pointer_type, sooty::value_t(), sooty::lexical_position() );
			ptype->children.push_back(childtype);
			return ptype;
		}
		
		
		case ID::add:
		case ID::sub:
		case ID::mul:
		case ID::div:
		case ID::equ:
			return type_of( marshall::binary_expression::lhs(N) );
		
		// expression of lhs (something), and rhs (the index)
		case ID::index_operator:
		{
			// currently we only support arrays and pointers. in the future we'll support
			// user-defined types having the member operator
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			sooty::parseme_ptr lhs_type = resolve::type_of(lhs);
			sooty::const_parseme_ptr_ref lhs_type_structure = resolve::type_structure_of(lhs_type);
			ATMA_ASSERT(lhs_type_structure->id == ID::array_type || lhs_type_structure->id == ID::pointer_type);
			
			if (lhs_type_structure->id == ID::pointer_type) {
				return resolve::type_of(marshall::pointer_type::pointee_type(lhs_type_structure));
			}
			else if (lhs_type_structure->id == ID::array_type) {
				return resolve::type_of(marshall::array_type::type(lhs_type_structure));
			}
			
			ATMA_HALT("bad!");
			return sooty::parseme_ptr();
		}
		
		
		case ID::member_operator:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);

			// lhs! find its type-structure
			sooty::parseme_ptr lhs_type = resolve::type_of(lhs, resolve::remove_refs());
			ATMA_ASSERT(lhs_type);
			
			// rhs! we need to find get the type-definition of the lhs and find the rhs's type in it
			ATMA_ASSERT( rhs->id == ID::identifier );
			const std::string& rhs_identifier = marshall::binary_expression::rhs(N)->value.string;
			sooty::parseme_ptr member = resolve::member_of_type_definition( lhs_type, rhs_identifier );
			if (member)
				return resolve::type_of(member);
			
			ATMA_HALT("bad!");
			return sooty::parseme_ptr();
		}
		
		case ID::intrinsic_int_add:
		case ID::intrinsic_int_sub:
		case ID::intrinsic_int_mul:
		case ID::intrinsic_int_div:
			return type_of(synthesize::int_type(N));
		
		default: 
		{
			ATMA_ASSERT(false && "not defined");
			return sooty::parseme_ptr();
		}
	}
}

sooty::parseme_ptr prism::resolve::type_structure_of(sooty::const_parseme_ptr_ref N)
{
	sooty::parseme_ptr type_of_N = type_of(N);
	
	switch (type_of_N->id) {
		case ID::int_type:
		case ID::real_type:
		case ID::bool_type:
		case ID::void_type:
		case ID::pointer_type:
		case ID::array_type:
			return type_of_N;
		
		case ID::type_definition:
			return marshall::type_definition::members(type_of_N);
		
		default:
			ATMA_HALT("bad~~~");
			return N;
	}
}

bool prism::resolve::types_match( sooty::const_parseme_ptr_ref lhsi, sooty::const_parseme_ptr_ref rhsi )
{
	ATMA_ASSERT(lhsi && rhsi);
	sooty::parseme_ptr lhs = type_of(lhsi);
	sooty::parseme_ptr rhs = type_of(rhsi);
	
	if (lhs->id == ID::any_type || rhs->id == ID::any_type)
		return true;
	
	// pointers and arrays are different!
	if (lhs->id == ID::pointer_type)
		return rhs->id == ID::pointer_type && types_match(marshall::pointer_type::pointee_type(lhs), marshall::pointer_type::pointee_type(rhs));
	else if (lhs->id == ID::array_type)
		return rhs->id == ID::array_type && types_match(marshall::array_type::type(lhs), marshall::array_type::type(rhs));
	else if (lhs->id == ID::reference_type)
		return rhs->id == ID::reference_type && types_match(marshall::array_type::type(lhs), marshall::array_type::type(rhs));
		
	if (rhs->id == ID::pointer_type || rhs->id == ID::array_type || rhs->id == ID::reference_type)
		return false;
	
	ATMA_ASSERT(rhs->id == ID::type_definition);
	return marshall::type_definition::name(lhs)->value.string == marshall::type_definition::name(rhs)->value.string;
}


//=====================================================================
// type_matches_pred
//=====================================================================
prism::resolve::type_matches_pred::type_matches_pred( sooty::const_parseme_ptr_ref lhs )
	: lhs(lhs)
{
}

bool prism::resolve::type_matches_pred::operator()( sooty::const_parseme_ptr_ref rhs ) const
{
	return types_match(lhs, rhs);
}






bool prism::resolve::type_can_convert_to( sooty::const_parseme_ptr_ref lhsi, sooty::const_parseme_ptr_ref rhsi )
{
	ATMA_ASSERT(lhsi && rhsi);
	sooty::parseme_ptr lhs = type_of(lhsi);
	sooty::parseme_ptr rhs = type_of(rhsi);
	
	if (lhs->id == ID::reference_type)
		if (rhs->id == ID::reference_type)
			return types_match(lhs->children.front(), rhs->children.front());
		else
			return types_match(lhs->children.front(), rhs);
	else
		if (rhs->id == ID::reference_type)
			return types_match(lhs, rhs->children.front());
		else
			return types_match(lhs, rhs);
}


prism::resolve::type_converts_pred::type_converts_pred( sooty::const_parseme_ptr_ref lhs )
	: lhs(lhs)
{
}


bool prism::resolve::type_converts_pred::operator()( sooty::const_parseme_ptr_ref rhs ) const
{
	return type_can_convert_to(lhs, rhs);
}

/*
bool prism::resolve::types_can_convert::operator()( sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs ) const
{
	return type_can_convert_to(lhs, rhs);
}
*/
