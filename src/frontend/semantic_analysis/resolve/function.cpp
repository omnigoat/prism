#include <prism/frontend/semantic_analysis/resolve/function.hpp>
//=====================================================================
#include <algorithm>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>

//=====================================================================
// function_matches_function_call_pred
//=====================================================================
prism::resolve::function_matches_function_call_pred::function_matches_function_call_pred( sooty::const_parseme_ptr_ref call_node )
	: call_node(call_node)
{
	ATMA_ASSERT(call_node->id == ID::function_call || call_node->id == ID::member_function_call);
}

bool prism::resolve::function_matches_function_call_pred::operator()( sooty::const_parseme_ptr_ref candidate ) const
{
	// must be a function, lol
	if (candidate->id != ID::function) return false;
	
	// must have the same name
	if (marshall::function_call::name(call_node)->value.string != marshall::function::name(candidate)->value.string) return false;

	// compare the list of arguments to the list of parameters
	const sooty::parseme_container& arguments = marshall::function_call::argument_list(call_node)->children;
	const sooty::parseme_container& parameters = marshall::function::parameter_list(candidate)->children;
	
	sooty::parseme_container::const_iterator pli = parameters.begin();
	sooty::parseme_container::const_iterator ali = arguments.begin();
	for ( ; ali != arguments.end() && pli != parameters.end(); ++ali )
	{
		sooty::const_parseme_ptr_ref parameter = *pli;
		
		// varags must be at the end, and are allowed to consume all remaining arguments
		if ( marshall::parameter::type(parameter)->value.string == "varags" ) {
			ATMA_ASSERT( pli + 1 == parameters.end() );
			continue;
		}
		
		if (!type_can_convert_to(*ali, parameter)) {
			break;
		}
		
		++pli;
	}
	
	return ali == arguments.end();
}


//=====================================================================
// function_matches_function_pred
//=====================================================================
prism::resolve::function_matches_function_pred::function_matches_function_pred( sooty::const_parseme_ptr_ref N )
	: N(N)
{
	ATMA_ASSERT(N->id == ID::function);
}

bool prism::resolve::function_matches_function_pred::operator()( sooty::const_parseme_ptr_ref candidate ) const
{
	if (candidate->id != ID::function) return false;
	if (marshall::function_call::name(N)->value.string != marshall::function::name(candidate)->value.string) return false;

	const sooty::parseme_container& call_pl = marshall::function_call::argument_list(N)->children;
	const sooty::parseme_container& candidate_pl = marshall::function::parameter_list(candidate)->children;
	
	// immediately rule the function out if the size is different
	if (call_pl.size() != candidate_pl.size()) return false;
	// the size is the same, and the argument list is empty, so we have a match!
	if (call_pl.empty()) return true;
	
	// little-used stl algorithm
	return std::mismatch(call_pl.begin(), call_pl.end(), candidate_pl.begin(), types_match).first == call_pl.end();
}



sooty::parseme_ptr prism::resolve::function_of_function_call( sooty::const_parseme_ptr_ref call )
{
	if (call->id == ID::member_function_call)
		return function_of_member_function_call(call);
	
	// first, find all matching function definitions
	sooty::parseme_container matching_functions;
	sooty::upwards_bredth_first_copy_if(call, std::back_inserter(matching_functions), function_matches_function_call_pred(call));
	
	// if we've found none, well that's too bad
	if (matching_functions.empty())
		return sooty::parseme_ptr();
	
	// sort the functions from most applicable to least
	std::sort(matching_functions.begin(), matching_functions.end(), prism::depth_pred());
	
	// take the most applicable and make sure that it's the only one of its kind. if it's not,
	// that means we have an ambiguity, and that's too bad
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> bounds =
		std::equal_range(matching_functions.begin(), matching_functions.end(), matching_functions.front(), prism::depth_pred());
	if ( std::distance(bounds.first, bounds.second) != 1 )
		return sooty::parseme_ptr();
	
	// return the most applicable function
	return matching_functions.front();
}


sooty::parseme_ptr prism::resolve::function_of_member_function_call( sooty::const_parseme_ptr_ref call )
{
	// lookup type-definition
	ATMA_ASSERT(marshall::member_function_call::argument_list(call)->children.size() > 0);
	sooty::parseme_ptr type_definition = resolve::type_of(marshall::member_function_call::argument_list(call)->children[0]);
	if (type_definition->id == ID::reference_type) {
		type_definition = resolve::type_of(type_definition->children.front());
	}
	
	// first, find all matching function definitions
	sooty::parseme_container matching_functions;
	sooty::depth_first_copy_if(std::back_inserter(matching_functions), type_definition, function_matches_function_call_pred(call));

	// if we've found none, well that's too bad
	if (matching_functions.empty())
		return sooty::parseme_ptr();

	// sort the functions from most applicable to least
	std::sort(matching_functions.begin(), matching_functions.end(), prism::depth_pred());

	// take the most applicable and make sure that it's the only one of its kind. if it's not,
	// that means we have an ambiguity, and that's too bad
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> bounds =
		std::equal_range(matching_functions.begin(), matching_functions.end(), matching_functions.front(), prism::depth_pred());
	if ( std::distance(bounds.first, bounds.second) != 1 )
		return sooty::parseme_ptr();

	// return the most applicable function
	return matching_functions.front();
}

