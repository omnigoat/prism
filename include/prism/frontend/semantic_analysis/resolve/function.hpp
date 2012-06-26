#ifndef PRISM_RESOLVE_FUNCTION_HPP
#define PRISM_RESOLVE_FUNCTION_HPP
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
//=====================================================================
namespace prism {
namespace resolve {
//=====================================================================

	//=====================================================================
	// true if the node is a function matching the function-call (call_node)
	//=====================================================================
	struct function_matches_function_call_pred {
		function_matches_function_call_pred(sooty::const_parseme_ptr_ref);
		bool operator ()(sooty::const_parseme_ptr_ref) const;
	private:
		sooty::parseme_ptr call_node;
	};
	
	
	//=====================================================================
	// true if the node is a function matching another function definition
	//=====================================================================
	struct function_matches_function_pred {
		function_matches_function_pred(sooty::const_parseme_ptr_ref);
		bool operator ()(sooty::const_parseme_ptr_ref) const;
	private:
		sooty::parseme_ptr N;
	};
	
	sooty::parseme_ptr function_of_function_call(sooty::const_parseme_ptr_ref);
	sooty::parseme_ptr function_of_member_function_call(sooty::const_parseme_ptr_ref call);
	sooty::parseme_ptr function_declaration_of_function_call(sooty::const_parseme_ptr_ref);
	
//=====================================================================
} // namespace resolve
} // namespace prism
//=====================================================================
#endif
//=====================================================================
