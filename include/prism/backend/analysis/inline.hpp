//=====================================================================
//
//    note: there are two types of inline functions. ones that are
//          inlined because they are small functions, etc. the other
//          type is functions that are specially marked to always be
//          inlined. these include constructors for integers, etc.
//
//=====================================================================
#ifndef PRISM_INLINE_HPP
#define PRISM_INLINE_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
namespace prism {
namespace inlining {
//=====================================================================
	
	bool should_intrinsically_inline(sooty::const_parseme_ptr_ref);
	
	//=====================================================================
	// results from inlining
	//=====================================================================
	struct results
	{
		// these statements should get inserted *somehwere* - the place
		// will be different depending on where in the expression the
		// function-call was located
		sooty::parseme_container statements_to_insert;
		
		// the function-call expression itself must be replace with
		// something. this is that something.
		sooty::parseme_ptr expression_to_replace_call_with;
	};
	
	//=====================================================================
	// build_inline_context
	// ---------------------
	//  returns the results of inlining the function-call, but does exactly
	//  nothing in terms of side-effects.
	//=====================================================================
	results build_inline_context(sooty::const_parseme_ptr_ref);
	
	//=====================================================================
	// inline_xxx_function_call_(at)
	// ----------------------
	//  performs the actual inlining, using the results obtained from 
	//=====================================================================
	void inline_void_function_call_at(sooty::const_parseme_ptr_ref parent, const sooty::parseme_container::iterator& position, results&);
	void inline_nonvoid_function_call_at(sooty::const_parseme_ptr_ref parent, const sooty::parseme_container::iterator& position, sooty::const_parseme_ptr_ref call, results&);
	void inline_function_call(sooty::const_parseme_ptr_ref call);
	
	
//=====================================================================
} // namespace inlining
} // namespace prism
//=====================================================================
#endif
//=====================================================================
