#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>

void prism::semantic_analysis::analyse::statement(semantic_info& si, sooty::parseme_ptr_ref N)
{
	switch (N->id)
	{
		case ID::if_statement:
		{
			expression( si, marshall::if_statement::expression(N) );
			
			sooty::parseme_ptr_ref true_block = marshall::if_statement::true_block(N);
			true_block->value.string = atma::string_builder("if_")(true_block->position.guid())("_true");
			block( si, marshall::if_statement::true_block(N) );
			
			sooty::parseme_ptr_ref false_block = marshall::if_statement::false_block(N);
			false_block->value.string = atma::string_builder("if_")(true_block->position.guid())("_false");
			block( si, marshall::if_statement::false_block(N) );
			
			break;
		}
		
		
		case ID::return_statement:
		{
			if (N->children.empty())
				break;
			
			expression(si, N->children.front());
			sooty::parseme_ptr owning_function = sooty::linear_upwards_find_first_if(N, sooty::id_matches(ID::function));
			if ( owning_function->value.integer == 1 ) {
				break;
			}
			sooty::const_parseme_ptr_ref return_type = marshall::function::return_type(owning_function);
			if ( !resolve::types_match(return_type, resolve::type_of(marshall::return_statement::expression(N), resolve::remove_refs())) ) {
				std::cerr << error(N, "return value doesn't match function definition");
				++si.errors;
			}
			break;
		}
		
		
		case ID::variable_definition:
		{
			// note: it's important to do the type first, as the expression may may reference
			// the type! :O
			type( si, marshall::variable_definition::type(N) );
			
			std::for_each(marshall::variable_definition::arguments(N)->children.begin(),
				marshall::variable_definition::arguments(N)->children.end(),
				boost::bind(analyse::expression, boost::ref(si), _1));
				
			//argument_list(si, marshall::variable_definition::arguments(N));
			
			//if ( marshall::variable_definition::has_defvalue(N) ) {
			//	expression( si, marshall::variable_definition::defvalue(N) );
			//}
			
			break;
		}
		
		
		case ID::assignment:
		{
			int previous_errors = si.errors;
			expression( si, N->children[1] );
			if (si.errors > previous_errors) {
				return;
			}
			
			previous_errors = si.errors;
			mutating_expression( si, N->children[0] );
			if (si.errors > previous_errors) {
				return;
			}
			
			//if ( !resolve::type_matches_pred(resolve::type_of(marshall::assignment::lhs(N)))(resolve::type_of(marshall::assignment::rhs(N))) ) {
			if ( !resolve::type_can_convert_to(marshall::assignment::rhs(N), marshall::assignment::lhs(N)) ) {
				std::cerr << error(N, "assignment values don't match");
				++si.errors;
			}
			break;
		}
		
		
		case ID::delete_:
			expression(si, N->children[0]);
			break;
		
		
		case ID::loop_statement:
		{
			marshall::loop_statement::block(N)->value.string = atma::string_builder("loop_")(N->position.guid())("_body");
			
			if ( marshall::loop_statement::control(N)->id != ID::not_present )
			{
				sooty::const_parseme_ptr_ref control = marshall::loop_statement::control(N);
				if (control->id == ID::in_control) {
					expression(si, control->children.back()->children.back());
				}
				else
				{
					sooty::parseme_ptr from_type, to_type;
					if (control->children[0]->id != ID::not_present) {
						expression(si, control->children[0]->children.back());
						from_type = resolve::type_of(control->children[0]->children.back());
					}
					if (control->children[1]->id != ID::not_present) {
						expression(si, control->children[1]->children.back());
						to_type = resolve::type_of(control->children[1]->children.back());
					}
					
					if (from_type && to_type)
					{
						if (!resolve::types_match(from_type, to_type)) {
							std::cerr << error(N, "types of 'from expression' and 'to expression' do not match") << std::endl;
						}
					}
				}
			}

			
			break;
		}
		
		
		default:
			expression(si, N);
			break;
	}
}


