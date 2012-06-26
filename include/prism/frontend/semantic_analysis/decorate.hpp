//=====================================================================
//
//=====================================================================
#ifndef KALEIDOSCOPE_COMMON_DECORATE_HPP
#define KALEIDOSCOPE_COMMON_DECORATE_HPP
//=====================================================================
#include <string>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
namespace prism {
//=====================================================================

	std::string decorate_function(sooty::const_parseme_ptr_ref);
	std::string decorate_type(sooty::const_parseme_ptr_ref);
	
	//std::string undecorated_type(sooty::const_parseme_ptr_ref);
	
//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
