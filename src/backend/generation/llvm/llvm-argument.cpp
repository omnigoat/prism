//=====================================================================
//
//    argument
//    ----------
//      so, arguments require some logic
//
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
using namespace prism;


std::string llvm::argument(sooty::const_parseme_ptr_ref argument, sooty::const_parseme_ptr_ref parameter_type)
{
	sooty::const_parseme_ptr_ref type_name = marshall::argument::type(argument);
	sooty::const_parseme_ptr_ref name = marshall::argument::name(argument);
	sooty::parseme_ptr argument_type = resolve::type_of(type_name);

	// if parameter is a reference...
	if (parameter_type->id == ID::reference_type) {
		// if argument is a reference too, cancels out! :D
		if (argument_type->id == ID::reference_type) {
			return atma::string_builder() << logical_type_name(argument_type) << " " << rvalue_name(name);
		}
		// else we must pass the lvalue of the argument to the reference-parameter
		else {
			return atma::string_builder() << storage_type_name(argument_type) << " " << lvalue_name(name);
		}
	}
	// parameter not a reference
	else {
		// if we fit into register, pass by rvalue
		if ( prism::inspect::can_fit_in_register(argument_type) ) {
			return atma::string_builder() << llvm::logical_type_name(argument_type) << " " << rvalue_name(name);
		}
		// we don't fit! pass by lvalue!
		else {
			return atma::string_builder() << storage_type_name(argument_type) << " " << lvalue_name(name);
		}
	}
}