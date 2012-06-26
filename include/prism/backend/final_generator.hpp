//=====================================================================
//
//
//
//=====================================================================
#ifndef KALEIDOSCOPE_GENERATOR_FINAL_GENERATOR_HPP
#define KALEIDOSCOPE_GENERATOR_FINAL_GENERATOR_HPP
//=====================================================================
#include <sooty/common/value_t.hpp>
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/abstract_parser.hpp>
#include <sooty/frontend/semantic_analysis/symbol_table.hpp>
//=====================================================================
#include <prism/prism.hpp>
#include <prism/common/channels.hpp>
#include <prism/frontend/semantic_analysis/scope.hpp>
//=====================================================================
PRISM_BEGIN
namespace generator {
//=====================================================================
	
	struct type_proxy
	{
		size_t id;
		sooty::value_t value;
		size_t size() const;
	};


	struct function_proxy
	{
		type_proxy return_type;
		std::string name;
		
		typedef std::vector<type_proxy> parameter_container;
		parameter_container parameters;

		size_t parameter_stack_size();
		size_t return_type_stack_size();
	};
	
	
	
	class final_generator
	{
		sooty::parseme_ptr root_;
		std::ostream* output_stream_;
		typedef sooty::parseme_ptr_ref parseme_ptr_ref;
		
	public:
		final_generator(parseme_ptr_ref root);
		
		void go(std::ostream*);
		
	protected:
		std::ostream& output_stream() { return *output_stream_; }
		void write(const std::string&);
	
	protected:
		void start_rule(parseme_ptr_ref);
		
		
		void function(parseme_ptr_ref);
		void function_return_type(parseme_ptr_ref, function_proxy&);
		void function_symbol(parseme_ptr_ref, function_proxy&);
		void function_parameter_list(parseme_ptr_ref, function_proxy&);
		
		void statement(parseme_ptr_ref);
		void type(parseme_ptr_ref);
		void symbol(parseme_ptr_ref);
		void return_statement(parseme_ptr_ref, size_t);
		void function_call(parseme_ptr_ref);
		void expression(parseme_ptr_ref);
		void rvalue(parseme_ptr_ref);
		
	protected:
		size_t calculate_required_stack_space(parseme_ptr_ref);
		size_t calculate_parameter_type_id(parseme_ptr_ref);

	};

//=====================================================================
} // namespace generator
PRISM_CLOSE
//=====================================================================
#endif
//=====================================================================
