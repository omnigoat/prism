//=====================================================================
//
//=====================================================================
#ifndef PRISM_SEMANTIC_ANALYSIS_ANALYSE_HPP
#define PRISM_SEMANTIC_ANALYSIS_ANALYSE_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
#include <prism/frontend/semantic_analysis/semantic_info.hpp>
//=====================================================================
namespace prism {
namespace semantic_analysis {
namespace analyse {
//=====================================================================
	
	//=====================================================================
	// these functions simply analyse a given branch
	//=====================================================================
	void module(semantic_info&, sooty::parseme_ptr_ref);
	void module_integral_operators(sooty::parseme_ptr_ref);
	void function(semantic_info&, sooty::parseme_ptr_ref);
	void function_signature(semantic_info&, sooty::parseme_ptr_ref);
	void function_call(semantic_info&, sooty::parseme_ptr_ref);
	void member_function_call(semantic_info&, sooty::parseme_ptr_ref);
	void type(semantic_info&, sooty::parseme_ptr_ref);
	void type_identifier(semantic_info&, sooty::parseme_ptr_ref);
	void parameter(semantic_info&, sooty::parseme_ptr_ref);
	void block(semantic_info&, sooty::parseme_ptr_ref);
	void statement(semantic_info&, sooty::parseme_ptr_ref);
	void type_definition_signature(semantic_info&, sooty::parseme_ptr_ref);
	void type_definition_interface(semantic_info&, sooty::parseme_ptr_ref);
	void type_definition(semantic_info&, sooty::parseme_ptr_ref);
	void expression(semantic_info&, sooty::parseme_ptr_ref);
	void mutating_expression(semantic_info&, sooty::parseme_ptr_ref);
	void init_element(semantic_info&, sooty::parseme_ptr_ref);
	
	// SERIOUSLY, these need a better place
	struct error
	{
		sooty::const_parseme_ptr_ref node;
		std::string reason;
		error(sooty::const_parseme_ptr_ref n, const std::string& reason) : node(n), reason(reason) {}
	};
	
	template <typename E, typename T>
	inline std::basic_ostream<E, T>& operator << (std::basic_ostream<E, T>& _Ostr, const error& rhs)
	{
		_Ostr << "error (" << (rhs.node->position.row + 1) << "): " << rhs.reason << std::endl;
		return _Ostr;
	}
	
//=====================================================================
} // namespace analyse
} // namespace semantic_analysis
} // namespace prism
//=====================================================================
#endif // inclusion guard
//=====================================================================
