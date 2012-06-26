#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
//=====================================================================



void prism::semantic_analysis::analyse::expression(semantic_info& si, sooty::parseme_ptr_ref N)
{
	switch (N->id)
	{
		case ID::add:
		case ID::sub:
		case ID::mul:
		case ID::div:
		case ID::equ:
		case ID::intrinsic_int_add:
		case ID::intrinsic_int_sub:
		case ID::intrinsic_int_mul:
		case ID::intrinsic_int_div:
		case ID::intrinsic_int_eq:
			expression( si, marshall::binary_expression::lhs(N) );
			expression( si, marshall::binary_expression::rhs(N) );
			if (!resolve::types_match( marshall::binary_expression::lhs(N), marshall::binary_expression::rhs(N) )) {
				std::cerr << error(N, "arguments are not the same type!");
				++si.errors;
			}
			break;
		
		case ID::dereference:
		case ID::address_of:
			expression(si, N->children.front());
			break;
		
		case ID::index_operator:
		{
			// expression of lhs (something), and rhs (the index)
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			expression(si, lhs);
			expression(si, marshall::binary_expression::rhs(N));
			
			// find out the resultant type. currently we only support arrays and pointers.
			// in the future we'll support user-defined types having the member operator
			sooty::parseme_ptr lhs_type = resolve::type_of(lhs);
			sooty::const_parseme_ptr_ref lhs_type_structure = resolve::type_structure_of(lhs_type);
			ATMA_ASSERT(lhs_type_structure->id == ID::array_type || lhs_type_structure->id == ID::pointer_type);
			break;
		}
		
		case ID::new_:
			type(si, marshall::new_::type(N));
			std::for_each(
				marshall::new_::arguments(N)->children.begin(),
				marshall::new_::arguments(N)->children.end(),
				boost::bind(analyse::expression, boost::ref(si), _1)
			);
			break;
		
		case ID::member_operator:
		{
			sooty::parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			
			// lhs is an expression, rhs is an identifier (which we *don't* want to expand)
			expression(si, lhs);
			
			// rhs! we need to find get the type-definition of the lhs and find the rhs's type in it
			ATMA_ASSERT( rhs->id == ID::identifier );
			sooty::parseme_ptr lhs_type = resolve::type_of(lhs);
			
			// references are okay!
			if (lhs_type->id == ID::reference_type)
				lhs_type = resolve::type_of(lhs_type->children.front());
			
			sooty::parseme_ptr lhs_members = marshall::type_definition::members(lhs_type);
			
			ATMA_ASSERT( lhs_members->id == ID::member_definitions );
			for (sooty::parseme_container::const_iterator i = lhs_members->children.begin(); i != lhs_members->children.end(); ++i)
			{
				sooty::const_parseme_ptr_ref member = *i;
				if (member->id == ID::variable_definition)
				{
					if ( marshall::variable_definition::name(member)->value.string == marshall::identifier::name(rhs)->value.string ) {
						// find the type-definition for this variable-definition
						sooty::parseme_ptr member_td = marshall::variable_definition::type(member);
						if (member_td->id == ID::reference_type) {
							sooty::parseme_ptr old_N = N;
							N = sooty::make_parseme(N->parent.lock(), ID::un_reference);
							N->children.push_back(old_N);
							old_N->parent = N;
						}
					}
				}
			}
			break;
		}
		
		case ID::identifier:
		{
			sooty::parseme_ptr P = resolve::identifier_to_variable_definition(N);
			if (!P) {
				++si.errors;
				std::cerr << "error (" << N->position << "): identifier " << N->value.string << " is unknown" << std::endl;
			}
			
			if ( resolve::type_of(N)->id == ID::reference_type ) {
				sooty::parseme_ptr us = N;
				N = sooty::make_parseme(us->parent.lock(), ID::un_reference, sooty::value_t());
				N->children.push_back(us);
			}
			
			break;
		}
		
		case ID::member_function_call:
		{
			member_function_call(si, N);
			break;
		}
		
		case ID::function_call:
		{
			function_call(si, N);
			break;
		}
	}
}


