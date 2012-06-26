#ifndef PRISM_RESOLVE_TYPE_HPP
#define PRISM_RESOLVE_TYPE_HPP
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
	//
	//   guys, guys, guys:
	//
	//   a TYPE, is either a pointer, array, or a type-definition. k?
	//
	//=====================================================================

	//=====================================================================
	// returns the node that is the type of the given thing
	//=====================================================================
	struct remove_refs {};
	sooty::parseme_ptr type_of(sooty::const_parseme_ptr_ref N);
	inline sooty::parseme_ptr type_of(sooty::const_parseme_ptr_ref N, remove_refs) {
		sooty::parseme_ptr type = type_of(N);
		ATMA_ASSERT(type);
		if (type->id == ID::reference_type)
			type = type_of(type->children.front());
		return type;
	}
	
	struct type_of_pred {
		type_of_pred(sooty::const_parseme_ptr_ref N) : N(N)	{}
		sooty::parseme_ptr operator ()() const {
			return type_of(N);
		}
	private:
		sooty::parseme_ptr N;
	};
	
	//=====================================================================
	// returns the structure of a type
	//=====================================================================
	sooty::parseme_ptr type_structure_of(sooty::const_parseme_ptr_ref);
	
	

	//=====================================================================
	// true if the types (assumed to be defining a type) match. both versions.
	//=====================================================================
	bool types_match(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs);
	
	struct type_matches_pred {
		type_matches_pred(sooty::const_parseme_ptr_ref lhs);
		bool operator ()(sooty::const_parseme_ptr_ref rhs) const;
	private:
		sooty::parseme_ptr lhs;
	};
	
	//=====================================================================
	// true if we can convert lhs to rhs
	//=====================================================================
	bool type_can_convert_to(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs);
	
	struct type_converts_pred {
		type_converts_pred(sooty::const_parseme_ptr_ref );
		bool operator ()(sooty::const_parseme_ptr_ref rhs) const;
	private:
		sooty::parseme_ptr lhs;
	};

	//=====================================================================
	// binary version of the above
	//=====================================================================
	struct types_match_pred {
		bool operator ()(sooty::const_parseme_ptr_ref lhs, sooty::const_parseme_ptr_ref rhs) const;
	};
	
//=====================================================================
} // namespace resolve
} // namespace prism
//=====================================================================
#endif
//=====================================================================
