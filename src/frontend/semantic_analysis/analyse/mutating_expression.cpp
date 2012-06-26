#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
//=====================================================================

void prism::semantic_analysis::analyse::mutating_expression(semantic_info& si, sooty::parseme_ptr_ref N)
{
	switch (N->id)
	{
		// these ones we don't care about
		case ID::add:
		case ID::sub:
		case ID::mul:
		case ID::div:
		case ID::equ:
		case ID::new_:
		case ID::function_call:
			expression(si, N);
			break;
		
		case ID::member_operator:
		{
			// lhs is an expression, rhs is an identifier (which we *don't* want to expand)
			mutating_expression(si, marshall::binary_expression::lhs(N));
			
			
			sooty::parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);

			// rhs! we need to find get the type-definition of the lhs and find the rhs's type in it
			ATMA_ASSERT( rhs->id == ID::identifier );
			sooty::parseme_ptr lhs_type = resolve::type_of(lhs, resolve::remove_refs());

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
		
		case ID::dereference:
		case ID::address_of:
			mutating_expression(si, N->children.front());
			break;
		
		case ID::identifier:
		{
			sooty::parseme_ptr P = resolve::identifier_to_variable_definition(N);
			if (!P) {
				++si.errors;
				std::cerr << error(N, "couldn't find variable-definition for " + marshall::identifier::name(N)->value.string) << std::endl;
				return;
			}
			
			sooty::parseme_ptr parent = N->parent.lock();
			bool bad_mutation = false;
			if (P->id == ID::parameter)
			{
				sooty::parseme_ptr type = resolve::type_of(P);
				ATMA_ASSERT(type);
				if (type->id == ID::pointer_type) {
					if (parent->id != ID::dereference) {
						bad_mutation = true;
					}
				}
				else if (type->id != ID::reference_type) {
					bad_mutation = true;
				}
			}
			
			if (bad_mutation)
			{
				++si.errors;
				std::cerr << error(N, "can't mutate a parameter!");
			}
			
			if ( resolve::type_of(N)->id == ID::reference_type ) {
				sooty::parseme_ptr us = N;
				N = sooty::make_parseme(us->parent.lock(), ID::un_reference, sooty::value_t());
				N->children.push_back(us);
			}
			
			break;
		}
	}
}


