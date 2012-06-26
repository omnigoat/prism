//=====================================================================
//
//=====================================================================
#ifndef PRISM_SEMANTIC_ANALYSIS_FUNCTION_HPP
#define PRISM_SEMANTIC_ANALYSIS_FUNCTION_HPP
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
//=====================================================================

	namespace semantic_analysis {
		void add_implicit_this(sooty::parseme_ptr_ref);
	}
	
	//=====================================================================
	// predicate for sorting functions in order of scope resolution.
	//  - if we find any equivalence, then we have an ambiguity at that level.
	//=====================================================================
	struct function_scope_sort_pred
	{
		bool operator ()(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs) const
		{
			ATMA_ASSERT(lhs->id == prism::ID::function);
			ATMA_ASSERT(rhs->id == prism::ID::function);

			// first, sort by depth of tree
			int lhs_depth = sooty::depth(sooty::parseme_ptr(), lhs);
			int rhs_depth = sooty::depth(sooty::parseme_ptr(), rhs);

			if (lhs_depth < rhs_depth)
				return true;
			else if (rhs_depth < lhs_depth)
				return false;
			else
				return false;

			return false;
		}
	};

	struct depth_pred {
		bool operator ()(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs) const {
			return sooty::depth(sooty::parseme_ptr(), lhs) < sooty::depth(sooty::parseme_ptr(), rhs);
		}
	};


//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
