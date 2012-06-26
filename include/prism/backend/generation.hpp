//=====================================================================
//
//=====================================================================
#ifndef PRISM_GENERATION_HPP
#define PRISM_GENERATION_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
namespace prism {
//=====================================================================
	
	void generate(std::ostream* out_stream, sooty::const_parseme_ptr_ref);
	
	void llvm_prepass(sooty::parseme_ptr_ref);
	void generate_llvm(std::ostream& out_stream, sooty::const_parseme_ptr_ref);
	
//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
