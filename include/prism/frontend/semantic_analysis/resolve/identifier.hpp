#ifndef KALEIDOSCOPE_RESOLVE_HPP
#define KALEIDOSCOPE_RESOLVE_HPP
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
	// true if the node is a matching variable definition
	//=====================================================================
	struct identifier_matches_pred {
		identifier_matches_pred(sooty::const_parseme_ptr_ref caller);
		bool operator ()(sooty::const_parseme_ptr_ref) const;
	private:
		sooty::parseme_ptr caller;
	};
	
	
	
	//=====================================================================
	//
	//  identifier_to_variable_definition
	//  -----------------------------------
	//    takes an identifier, and finds where it is defined. it includes
	//    local variable-definitions and parameters
	//
	//    NOTE: this identifier must not be a typename. that will not work.
	//
	//=====================================================================
	namespace detail {
		inline bool is_function(sooty::const_parseme_ptr_ref n) { return n->id == ID::function; }
	}
	
	sooty::parseme_ptr identifier_to_variable_definition(sooty::const_parseme_ptr_ref N);
	
//=====================================================================
} // namespace resolve
} // namespace prism
//=====================================================================
#endif
//=====================================================================
