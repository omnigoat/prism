//=====================================================================
//
//=====================================================================
#ifndef PRISM_PARSE_HPP
#define PRISM_PARSE_HPP
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
namespace prism {
//=====================================================================
	
	// maybe not just yet
	//#define id_gen(a) const size_t a = __LINE__;
	
	static const size_t STRUCT = 10000;
	static const size_t KEYWORD = 20000;
	static const size_t LITERAL = 30000;
	static const size_t STMT = 40000;
	static const size_t EXPR = 50000;
	static const size_t INTEGRAL = 60000;
	static const size_t OTHER = 70000;
	static const size_t OPER = 80000;
	static const size_t INTRINSIC = 90000;
	
	// creates a 32-bit id that encapsulates some info
	// type must not exceed 4 bits
	// permutation must not exceed 12 bits
	// subperm must not exceed 16 bits
	#define ID_GEN(type, permutation, subperm) (type + 100 * permutation + subperm)     
	 
	 
	 //1L << (31 - type)) | (1L << (23 - permutation)) | (1L << (7 - subperm))
	 
	 
	namespace parsid
	{
		const size_t undefined = size_t(-1);
		
		// root!
		const size_t root = ID_GEN(STRUCT, 0, 0);
		
		// control flow
		const size_t block = ID_GEN(STRUCT, 1, 0);
		const size_t member_definitions = ID_GEN(STRUCT, 1, 1);
		const size_t jump = ID_GEN(STRUCT, 1, 2);
		const size_t label = ID_GEN(STRUCT, 1, 3);
		
		// literals
		const size_t int_literal = ID_GEN(LITERAL, 0, 0);
		const size_t real_literal = ID_GEN(LITERAL, 1, 0);
		const size_t bool_literal = ID_GEN(LITERAL, 2, 0);
		const size_t string_literal = ID_GEN(LITERAL, 3, 0);
		
		// integral types
		const size_t identifier = ID_GEN(INTEGRAL, 0, 0);
		const size_t type_identifier = ID_GEN(INTEGRAL, 0, 1);
		const size_t pointer_type = ID_GEN(INTEGRAL, 1, 0);
		const size_t typeof = ID_GEN(INTEGRAL, 2, 0);
		const size_t int_type = ID_GEN(INTEGRAL, 3, 0);
		const size_t real_type = ID_GEN(INTEGRAL, 4, 0);
		const size_t bool_type = ID_GEN(INTEGRAL, 5, 0);
		const size_t void_type = ID_GEN(INTEGRAL, 6, 0);
		const size_t char_type = ID_GEN(INTEGRAL, 7, 0);
		const size_t reference_type = ID_GEN(INTEGRAL, 8, 0);
		const size_t varags_type = ID_GEN(INTEGRAL, 9, 0);
		const size_t any_type = ID_GEN(INTEGRAL, 10, 0);
		
		const size_t bitwidth = ID_GEN(INTEGRAL, 11, 0);
		
		// cvs
		const size_t external = ID_GEN(STRUCT, 6, 0);
		const size_t internal = ID_GEN(STRUCT, 6, 1);
		
		// function-declaration
		const size_t function_declaration = ID_GEN(STRUCT, 5, 0);
		const size_t cc_fast = ID_GEN(STRUCT, 5, 1);
		const size_t cc_c = ID_GEN(STRUCT, 5, 2);

		// function
		const size_t function = ID_GEN(STRUCT, 2, 0);
		const size_t parameter_list = ID_GEN(STRUCT, 2, 1);
		const size_t parameter = ID_GEN(STRUCT, 2, 2);
		const size_t init_list = ID_GEN(STRUCT, 2, 3);
		const size_t init_element = ID_GEN(STRUCT, 2, 4);
		const size_t type_parameter = ID_GEN(STRUCT, 2, 5);
		
		// function call
		const size_t function_call = ID_GEN(STRUCT, 3, 0);
		const size_t argument_list = ID_GEN(STRUCT, 3, 1);
		const size_t member_function_call = ID_GEN(STRUCT, 3, 2);
		
		// type!
		const size_t type_definition = ID_GEN(STRUCT, 4, 0);
		const size_t composite_type = ID_GEN(STRUCT, 4, 1);
		const size_t array_type = ID_GEN(STRUCT, 4, 2);
		const size_t array_dimension = ID_GEN(STRUCT, 4, 3);
		const size_t dynamic_array_type = ID_GEN(STRUCT, 4, 4);
		
		
		
		
		// statements
		const size_t return_statement = ID_GEN(STMT, 0, 0);
		const size_t variable_definition = ID_GEN(STMT, 1, 0);
		const size_t if_statement = ID_GEN(STMT, 2, 0);
		const size_t loop_statement = ID_GEN(STMT, 3, 0);
		const size_t not_present = ID_GEN(STMT, 4, 0);
		const size_t in_control = ID_GEN(STMT, 4, 1);
		const size_t from_to_control = ID_GEN(STMT, 4, 2);
		
		
		// expressions
		const size_t add = ID_GEN(EXPR, 0, 0);
		const size_t sub = ID_GEN(EXPR, 1, 0);
		const size_t mul = ID_GEN(EXPR, 2, 0);
		const size_t div = ID_GEN(EXPR, 3, 0);
		const size_t dereference = ID_GEN(EXPR, 4, 0);
		const size_t un_reference = ID_GEN(EXPR, 4, 1);
		const size_t address_of = ID_GEN(EXPR, 5, 0);
		const size_t new_ = ID_GEN(EXPR, 6, 0);
		const size_t delete_ = ID_GEN(EXPR, 7, 0);
		const size_t equ = ID_GEN(EXPR, 8, 0);
		const size_t lt = ID_GEN(EXPR, 8, 1);
		const size_t assignment = ID_GEN(EXPR, 9, 0);
		const size_t phi = ID_GEN(EXPR, 10, 0);
		const size_t postfix = ID_GEN(EXPR, 11, 0);
		const size_t in_expression = ID_GEN(EXPR, 12, 0);
		const size_t from_expression = ID_GEN(EXPR, 12, 1);
		const size_t to_expression = ID_GEN(EXPR, 12, 2);
		const size_t via_expression = ID_GEN(EXPR, 12, 3);
		
		//=====================================================================
		// operators
		//=====================================================================
		// - equality
		const size_t eq_operator = ID_GEN(OPER, 0, 0);
		
		// - additive
		const size_t add_operator = ID_GEN(OPER, 1, 0);
		
		const size_t member_operator = ID_GEN(OPER, 2, 0);
		const size_t index_operator = ID_GEN(OPER, 2, 1);
		const size_t deref_member_operator = ID_GEN(OPER, 2, 2);

		// intrinsics
		const size_t intrinsic_int_add = ID_GEN(INTRINSIC, 0, 0);
		const size_t intrinsic_int_sub = ID_GEN(INTRINSIC, 0, 1);
		const size_t intrinsic_int_mul = ID_GEN(INTRINSIC, 0, 2);
		const size_t intrinsic_int_div = ID_GEN(INTRINSIC, 0, 3);
		const size_t intrinsic_int_eq  = ID_GEN(INTRINSIC, 0, 4);
		const size_t intrinsic_int_neq = ID_GEN(INTRINSIC, 0, 5);
		const size_t intrinsic_int_lt  = ID_GEN(INTRINSIC, 0, 6);
		const size_t intrinsic_int_lte = ID_GEN(INTRINSIC, 0, 7);
		const size_t intrinsic_int_gt  = ID_GEN(INTRINSIC, 0, 8);
		const size_t intrinsic_int_gte = ID_GEN(INTRINSIC, 0, 9);
		const size_t intrinsic_int_default = ID_GEN(INTRINSIC, 0, 10);
		const size_t intrinsic_set_reference = ID_GEN(INTRINSIC, 1, 0);
		
		inline bool is_statement(size_t id) {
			return (id >= STMT && id < 50000);
		}
	}
	
	void parse(sooty::lexeme_list::const_iterator begin, sooty::lexeme_list::const_iterator end, sooty::parseme_ptr& dest);
	
	namespace ID = parsid;
	
	
//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
