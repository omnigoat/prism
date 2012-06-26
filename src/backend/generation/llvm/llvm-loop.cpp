#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
using namespace prism;

namespace {
	sooty::parseme_ptr type_of_control(sooty::const_parseme_ptr_ref N)
	{
		if ( marshall::loop_statement::control(N)->id != ID::not_present )
		{
			sooty::const_parseme_ptr_ref control = marshall::loop_statement::control(N);
			
			if (control->id == ID::in_control)
			{
				//return resolve::type_of(
				ATMA_HALT("nyi");
			}
			else
			{
				sooty::parseme_ptr from_type, to_type;
				if (control->children[0]->id != ID::not_present) {
					return resolve::type_of(control->children[0]->children.back());
				}
				else {
					return resolve::type_of(control->children[1]->children.back());
				}
			}
			
		}
		
		return sooty::parseme_ptr();
	}
	
	sooty::parseme_ptr initial_value(sooty::const_parseme_ptr_ref control)
	{
		// nyi
		ATMA_ASSERT(control->id != ID::in_control);
		
		sooty::parseme_ptr from_type, to_type;
		if (control->children[0]->id != ID::not_present) {
			return control->children[0]->children.back();
		}
		else {
			// synthesize a default value here. we don't have default values though.
			// conundrum!
		}

		return sooty::parseme_ptr();
	}
}

void prism::llvm::loop(sooty::const_parseme_ptr_ref loop)
{
	sooty::const_parseme_ptr_ref variable = marshall::loop_statement::variable(loop);
	sooty::const_parseme_ptr_ref control = marshall::loop_statement::control(loop);
	sooty::const_parseme_ptr_ref if_expression = marshall::loop_statement::condition(loop);
	
	if (variable->id != ID::not_present)
	{
		// the variable definition
		sooty::parseme_ptr variable_type = type_of_control(loop);
		output_stream() << "%" << variable->value.string << " = alloca " << logical_type_name(variable_type) << std::endl;
		
		// the initial value
		sooty::parseme_ptr from_value = initial_value(control);
		output_stream()
			<< "store " << logical_type_name(variable_type) << " " << rvalue_name(from_value)
			<< ", " << storage_type_name(variable) << " " << lvalue_name(variable) << "\n";
		
	}
	
	std::string head_block_name = atma::string_builder("loop_")(loop->position.guid())("_head");
	std::string body_block_name = atma::string_builder("loop_")(loop->position.guid())("_body");
	std::string escape_block_name = atma::string_builder("loop_")(loop->position.guid())("_escape");
	output_stream() << "br label %" << head_block_name << std::endl;
	--detail::tabs();
	output_stream() << head_block_name << ":" << std::endl;
	++detail::tabs();


	

	if (if_expression->id != ID::not_present)
	{
		sooty::const_parseme_ptr_ref if_expression_condition = marshall::if_statement::expression(if_expression);
		pre_access_expression(if_expression_condition);
		output_stream() << "br i1 " << rvalue_name(if_expression_condition) << ", label %" << body_block_name << ", label %" << escape_block_name << "\n";
		block(marshall::loop_statement::block(loop), body_block_name);
		++detail::tabs();
		output_stream() << "br label %" << head_block_name << std::endl;
	}

	output_stream() << escape_block_name << ":" << std::endl;
}
