//=====================================================================
//
//=====================================================================
#ifndef KALEIDOSCOPE_RESOLVE_TYPENAME_HPP
#define KALEIDOSCOPE_RESOLVE_TYPENAME_HPP
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
	//  type_definitions_of
	//  ---------------------
	//    takes a typename, and finds all type-definitions for it, at the
	//    appropriate scopes :D
	//
	//    can not "fail" in that zero definitions isn't good, but a valid
	//    result nonetheless.
	//
	//=====================================================================
	void type_definitions_of(sooty::parseme_container& definitions, sooty::const_parseme_ptr_ref);


	//=====================================================================
	//
	//  type_definition_of
	//  -----------------------------------
	//    takes a typename, and finds the correct type-definition, given
	//    scoping and semantics. returns a null-ptr if:
	//      a) no type-definitions were found
	//      b) more than one could fit (ie, ambiguity)
	//
	//
	//=====================================================================
	sooty::parseme_ptr type_definition_of(sooty::const_parseme_ptr_ref);
	
	
	struct typename_matches_type_definition {
		typename_matches_type_definition(sooty::const_parseme_ptr_ref lhs);
		bool operator ()(sooty::const_parseme_ptr_ref rhs) const;
	private:
		sooty::parseme_ptr lhs;
	};
	
	sooty::parseme_ptr bitwidth_type(sooty::const_parseme_ptr_ref);
	
	sooty::parseme_ptr member_of_type_definition(sooty::const_parseme_ptr_ref, const std::string&);
	sooty::parseme_ptr parameter_of_type_definition(sooty::const_parseme_ptr_ref, const std::string&);
	
//=====================================================================
} // namespace resolve
} // namespace prism
//=====================================================================
#endif
//=====================================================================
