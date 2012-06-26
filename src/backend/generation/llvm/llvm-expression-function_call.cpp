#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/decorate.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/operator.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
#include <prism/backend/generation.hpp>
//=====================================================================
using namespace prism;


//=====================================================================
// function_call_arguments_expressions
// -------------------------------------
//   evaluates the arguments for the function call, at the same time
//   building the string that we will output for llvm (use it later!)
//=====================================================================
namespace {
	using namespace llvm;
	
	std::string function_call_arguments_expressions(const sooty::parseme_container& arguments, const sooty::parseme_container& parameters)
	{
		std::string result;
		
		sooty::parseme_container::const_iterator argument_iter = arguments.begin();
		sooty::parseme_container::const_iterator parameter_iter = parameters.begin();
		for (; argument_iter != arguments.end(); ++argument_iter)
		{
			sooty::const_parseme_ptr_ref argument = *argument_iter;
			sooty::const_parseme_ptr_ref argument_type = resolve::type_of(argument);
			sooty::const_parseme_ptr_ref parameter = *parameter_iter;
			sooty::const_parseme_ptr_ref parameter_type = resolve::type_of(parameter);
			bool parameter_is_varags = marshall::parameter::type(parameter)->value.string == "varags";

			if (parameter_type->id == ID::reference_type) {
				// if argument is a reference too, don't worry! :D
				if (argument_type->id == ID::reference_type) {
					pre_access_expression(argument);
				}
				// else we must pass the lvalue of the argument to the reference-parameter
				else {
					pre_mutate_expression(argument);
				}
			}
			// parameter not a reference
			else {
				// if we fit into register, pass by rvalue
				if ( prism::inspect::can_fit_in_register(argument_type) || parameter_is_varags ) {
					pre_access_expression(argument);
				}
				// we don't fit! pass by lvalue!
				else {
					pre_mutate_expression(argument);
				}
			}
			
			if (argument_iter != arguments.begin())
				result += ", ";
			
			result += llvm::argument(argument, resolve::type_of(parameter));
			
			if (!parameter_is_varags)
				++parameter_iter;
		}
		
		return result;
	}
}

//=====================================================================
// function_call_functionsig
//=====================================================================
namespace {
	using namespace llvm;
	
	std::string function_call_functionsig(const sooty::parseme_container& parameters, sooty::const_parseme_ptr_ref return_type)
	{
		std::string result = "(";
		
		
		if (!inspect::can_fit_in_register(return_type) && return_type->value.string != "void") {
			result += storage_type_name(return_type);
			if (!parameters.empty())
				result += ", ";
		}

		for (sooty::parseme_container::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			if (i != parameters.begin())
				result += ", ";

			if ( marshall::parameter::type(*i)->value.string == "varags" ) {
				result += "...";
			}
			else if ( inspect::can_fit_in_register(resolve::type_of(*i)) ) {
				result += logical_type_name(*i);
			}
			else {
				result += storage_type_name( marshall::parameter::type(*i) );
			}
		}
		result += ")*";
		
		return result;
	}
}

//=====================================================================
// function_call_impl
//=====================================================================
namespace {
	using namespace llvm;
	
	void function_call_impl(sooty::const_parseme_ptr_ref call, sooty::const_parseme_ptr_ref function)
	{
		sooty::const_parseme_ptr_ref AL = marshall::function_call::argument_list(call);
		sooty::const_parseme_ptr_ref PL = marshall::function::parameter_list(function);

		// visit the arguments first
		std::string llvm_argument_list = function_call_arguments_expressions(AL->children, PL->children);


		// name of the function
		std::string function_name = ( marshall::function::calling(function)->id == ID::cc_c )
			? marshall::function::name(function)->value.string : decorate_function(function);

		bool has_varags = !PL->children.empty() && marshall::parameter::type(PL->children.back())->value.string == "varags";

		// return-variable
		sooty::parseme_ptr return_type = marshall::function::return_type(function);
		bool return_type_fits_in_register = inspect::can_fit_in_register(return_type);
		bool return_type_is_void = return_type->value.string == "void";

		if (return_type_is_void) {
			output_stream() << "call void ";
		}
		else {
			if (return_type_fits_in_register) {
				output_stream() << rvalue_name(call) << " = call " << logical_type_name(call) << " ";
			}
			else {
				output_stream() << lvalue_name(call) << " = alloca " << logical_type_name(call) << "\n";
				output_stream() << "call void ";
			}
		}
		
		// function signature only if we have varags (unnecessary otherwise)
		if (has_varags) {
			output_stream(false) << function_call_functionsig(PL->children, return_type);
		}
		
		// actual arguments go here! :O
		output_stream(false) << " @" << function_name << "(";
		if (!return_type_fits_in_register && !return_type_is_void) {
			output_stream(false) << storage_type_name(call) << " " << lvalue_name(call);
		}
		
		output_stream(false) << llvm_argument_list;
		
		output_stream(false) << ")" << std::endl;
	}
}

void llvm::function_call(sooty::const_parseme_ptr_ref call)
{
	sooty::parseme_ptr function = sooty::upwards_bredth_first_find_first_if(call, prism::resolve::function_matches_function_call_pred(call));
	ATMA_ASSERT(function);
	if (!function)
		return;
	
	function_call_impl(call, function);
}



void llvm::member_function_call(sooty::const_parseme_ptr_ref call)
{
	sooty::parseme_ptr function = resolve::function_of_member_function_call(call);
	ATMA_ASSERT(function);
	if (!function)
		return;
	
	function_call_impl(call, function);
}
