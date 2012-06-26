//=====================================================================
//
//    reducing
//    ----------
//        reducing is the act of simplifying an AST branch at compile-
//    time. this is usually template related, such as reducing
//    identifiers to the actual template arguments.
//
//=====================================================================
#ifndef PRISM_SEMANTIC_ANALYSIS_REDUCE_HPP
#define PRISM_SEMANTIC_ANALYSIS_REDUCE_HPP
//=====================================================================
#include <string>
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
//=====================================================================
namespace prism {
namespace reduce {
//=====================================================================
	
	void reduce_with_template_arguments(sooty::parseme_container&, const sooty::parseme_container&, const sooty::parseme_container&);
	
//=====================================================================
} // namespace reduce
} // namespace prism
//=====================================================================
#endif
//=====================================================================
