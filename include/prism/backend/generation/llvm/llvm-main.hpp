//=====================================================================
//
//=====================================================================
#ifndef PRISM_EMISSION_HPP
#define PRISM_EMISSION_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
namespace prism {
//=====================================================================

	struct add_temporary_information_to_expressions
	{
		int& tmpvar;
		add_temporary_information_to_expressions(int& tmpvar) : tmpvar(tmpvar) {}
		void operator ()(sooty::parseme_ptr_ref);
	};
	
	inline int& usage_count(sooty::const_parseme_ptr_ref N) {
		static std::map<sooty::parseme_ptr, int> _;
		return _[N];
	}
	
	namespace llvm
	{
		//=====================================================================
		// output stream
		//=====================================================================
		namespace detail {
			inline int& tabs() { static int _ = 0; return _; }
			inline std::ostream*& outstream() { static std::ostream* _; return _; }
		}
		
		inline std::ostream& output_stream(bool tabbed = true) {
			if (tabbed)
				for (int i = 0; i < detail::tabs(); ++i)
					(*detail::outstream()) << "\t";
			return *detail::outstream();
		}

		inline void add_tab() { ++detail::tabs(); }
		inline void sub_tab() { --detail::tabs(); }


		//=====================================================================
		// tree functions
		//=====================================================================
		void module(sooty::const_parseme_ptr_ref);
		void function_declaration(sooty::const_parseme_ptr_ref);
		void function(sooty::const_parseme_ptr_ref);
		void parameter_list(sooty::const_parseme_ptr_ref);
		void parameter(sooty::const_parseme_ptr_ref);
		void block(sooty::const_parseme_ptr_ref, const std::string&);
		void statement(sooty::const_parseme_ptr_ref);
		std::string argument(sooty::const_parseme_ptr_ref, sooty::const_parseme_ptr_ref);
		void argument_list(sooty::const_parseme_ptr_ref);
		void pre_access_expression(sooty::const_parseme_ptr_ref);
		void pre_mutate_expression(sooty::const_parseme_ptr_ref);
		void function_call(sooty::const_parseme_ptr_ref);
		void member_function_call(sooty::const_parseme_ptr_ref);
		void post_mutate_expression(sooty::const_parseme_ptr_ref);
		void loop(sooty::const_parseme_ptr_ref);
		void type_definition(sooty::const_parseme_ptr_ref);
		void initialiser_element(sooty::const_parseme_ptr_ref elem);
		
		void inline_function_call(sooty::const_parseme_ptr_ref);
		
		inline void unreference(sooty::parseme_ptr_ref type) {
			if (type->id == prism::ID::reference_type) {
				type = prism::resolve::type_of(type->children.front());
			}
		}
		
		//=====================================================================
		// functions that make sense for recursion
		//=====================================================================
		std::string rvalue_name(sooty::const_parseme_ptr_ref);
		std::string lvalue_name(sooty::const_parseme_ptr_ref);
		
		std::string storage_type_name(sooty::const_parseme_ptr_ref);
		std::string logical_type_name(sooty::const_parseme_ptr_ref);
		
		const size_t LLVM = 16;
		const size_t br = ID_GEN(LLVM, 0, 1);
		const size_t label = ID_GEN(LLVM, 0, 2);
		
		//std::string address_name(sooty::const_parseme_ptr_ref);
		//std::string deref_name(sooty::const_parseme_ptr_ref);
	}
	
//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
