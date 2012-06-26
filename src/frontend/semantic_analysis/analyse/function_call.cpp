#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
//=====================================================================

namespace {
	std::string undecorated_function_call(sooty::const_parseme_ptr_ref N)
	{
		using namespace prism;
		
		std::string result;
		sooty::const_parseme_ptr_ref name = marshall::function_call::name(N);
		sooty::const_parseme_ptr_ref argument_list = marshall::function_call::argument_list(N);
		result += name->value.string;
		result += "(";
		for (sooty::parseme_container::const_iterator i = argument_list->children.begin(); i != argument_list->children.end(); ++i) {
			if (i != argument_list->children.begin())
				result += ", ";
			//sooty::parseme_ptr td = resolve::type_definition_of(*i);
			//result += marshall::type_definition::name(td)->value.string;
			result += "[type]";
		}
		result += ")";
		return result;
	}
}

void prism::semantic_analysis::analyse::function_call(semantic_info& si, sooty::parseme_ptr_ref N)
{
	sooty::parseme_ptr_ref argument_list = marshall::function_call::argument_list(N);
	std::for_each(argument_list->children.begin(), argument_list->children.end(), boost::bind(&expression, boost::ref(si), _1) );
	sooty::parseme_ptr function_matching_N = sooty::upwards_bredth_first_find_first_if(N, prism::resolve::function_matches_function_call_pred(N));
	if (!function_matching_N) {
		++si.errors;
		std::cerr << "error (" << N->position << "): " << " no function found for function-call:\n\t" 
			<< undecorated_function_call(N) << std::endl;
		return;
	}
	
	// increase the use-count of the function
	++marshall::function::semantics::use_count(function_matching_N).integer;
}


void prism::semantic_analysis::analyse::member_function_call(semantic_info& si, sooty::parseme_ptr_ref call)
{
	sooty::parseme_ptr_ref argument_list = marshall::member_function_call::argument_list(call);
	ATMA_ASSERT( !argument_list->children.empty() );
	std::for_each(argument_list->children.begin() + 1, argument_list->children.end(), boost::bind(&expression, boost::ref(si), _1) );
	
	// lookup type-definition
	sooty::parseme_ptr type_definition = resolve::type_of(marshall::member_function_call::argument_list(call)->children[0]);
	if (type_definition->id == ID::reference_type) {
		type_definition = resolve::type_of(type_definition->children.front());
	}
	
	// find all matching function definitions
	sooty::parseme_container matching_functions;
	sooty::depth_first_copy_if(std::back_inserter(matching_functions), type_definition, resolve::function_matches_function_call_pred(call));
	if (matching_functions.empty()) {
		++si.errors;
		std::cerr << error(call, "no function found for function-call:\n\t") << undecorated_function_call(call) << std::endl;
		return;
	}
	else if (matching_functions.size() > 1) {
		++si.errors;
		std::cerr << error(call, "too many definitions found!") << std::endl;
		return;
	}
	
	// sort the functions from most applicable to least
	std::sort(matching_functions.begin(), matching_functions.end(), prism::depth_pred());

	// take the most applicable and make sure that it's the only one of its kind. if it's not,
	// that means we have an ambiguity, and that's too bad
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> bounds =
		std::equal_range(matching_functions.begin(), matching_functions.end(), matching_functions.front(), prism::depth_pred());
	if ( std::distance(bounds.first, bounds.second) != 1 )
		return;

	// increase the use-count of the function
	++marshall::function::semantics::use_count(matching_functions.front()).integer;
}