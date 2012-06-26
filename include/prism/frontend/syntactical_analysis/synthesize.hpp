//=====================================================================
//
//=====================================================================
#ifndef PRISM_SYTHESIZE_HPP
#define PRISM_SYTHESIZE_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/parser.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
namespace prism {
namespace synthesize {
//=====================================================================
	
	inline sooty::parseme_ptr int_type(sooty::const_parseme_ptr_ref parent) {
		// SERIOUSLY, we need better defaults
		sooty::parseme_ptr the_int = sooty::make_parseme( parent, ID::type_identifier, "int" );
		the_int->children.push_back( sooty::make_parseme(the_int, ID::argument_list) );
		the_int->children.back()->children.push_back( sooty::make_parseme(the_int->children.back(), ID::bitwidth, 32) );
		semantic_analysis::analyse::type_identifier(semantic_analysis::semantic_info(), the_int);
		return the_int;
	}
	
	inline sooty::parseme_ptr real_type(sooty::const_parseme_ptr_ref parent) {
		return sooty::make_parseme( parent, ID::type_identifier, "real" );
	}
	
	inline sooty::parseme_ptr void_type(sooty::const_parseme_ptr_ref parent = sooty::parseme_ptr()) {
		return sooty::make_parseme( parent, ID::type_identifier, "void" );
	}
	
	inline sooty::parseme_ptr bool_type(sooty::const_parseme_ptr_ref parent) {
		return resolve::type_of( sooty::make_parseme(parent, ID::type_identifier, "bool") );
	}
	
	inline sooty::parseme_ptr string_type(sooty::const_parseme_ptr_ref parent) {
		sooty::parseme_ptr p = sooty::make_parseme(parent, ID::pointer_type);
		p->children.push_back( sooty::make_parseme(p, ID::type_identifier, "char") );
		return resolve::type_of(p);
	}
	
	inline sooty::parseme_ptr type_identifier(const std::string& iden) {
		sooty::parseme_ptr ti = sooty::make_parseme(sooty::parseme_ptr(), ID::type_identifier, iden);
		ti->children.push_back( sooty::make_parseme(ti, ID::argument_list) );
		return ti;
	}
	inline sooty::parseme_ptr type_identifier(sooty::const_parseme_ptr_ref parent, const std::string& iden) {
		sooty::parseme_ptr ti = sooty::make_parseme(parent, ID::type_identifier, iden);
		ti->children.push_back( sooty::make_parseme(ti, ID::argument_list) );
		return ti;
	}
	inline sooty::parseme_ptr function_call(sooty::const_parseme_ptr_ref parent, const std::string& name, sooty::const_parseme_ptr_ref arg1) {
		sooty::parseme_ptr function_call = sooty::make_parseme(parent, ID::function_call, "sure is");
		(sooty::immediate(function_call)) (
			(sooty::insert(ID::identifier, name),
			sooty::insert(ID::argument_list) [
				sooty::insert(arg1)
			])
		);
		return function_call;
	}
	
	inline sooty::parseme_ptr function_call(sooty::const_parseme_ptr_ref parent, const std::string& name, sooty::const_parseme_ptr_ref arg1, sooty::const_parseme_ptr_ref arg2) {
		sooty::parseme_ptr function_call = sooty::make_parseme(parent, ID::function_call, "function-call");
		(sooty::immediate(function_call))(
			(sooty::insert(ID::identifier, name),
			sooty::insert(ID::argument_list) [
				sooty::insert(arg1),
				sooty::insert(arg2)
			])
		);
		return function_call;
	}
	
	inline sooty::parseme_ptr member_function_call(sooty::const_parseme_ptr_ref parent, const std::string& name, sooty::const_parseme_ptr_ref arg_type) {
		sooty::parseme_ptr function_call = sooty::make_parseme(parent, ID::member_function_call, "member-function-call");
		(sooty::immediate(function_call)) (
			(sooty::insert(ID::identifier, name),
			sooty::insert(ID::argument_list) [
				sooty::insert(arg_type)
			])
		);
		return function_call;
	}
	
	inline sooty::parseme_ptr member_function_call(sooty::const_parseme_ptr_ref parent, const std::string& name, sooty::const_parseme_ptr_ref arg_type, sooty::const_parseme_ptr_ref arg1) {
		sooty::parseme_ptr function_call = sooty::make_parseme(parent, ID::member_function_call, "member-function-call");
		(sooty::immediate(function_call))(
			(sooty::insert(ID::identifier, name),
			sooty::insert(ID::argument_list) [
				sooty::insert(arg_type),
				sooty::insert(arg1)
			])
		);
		return function_call;
	}
	
	inline sooty::parseme_ptr instance_of_argument(sooty::parseme_ptr parent, sooty::const_parseme_ptr_ref arg)
	{
		switch ( marshall::argument::type(arg)->id )
		{
			case ID::int_literal:
			case ID::real_literal:
			case ID::bool_literal:
				return sooty::make_parseme( parent, marshall::argument::type(arg)->id, arg->value );
				
			case ID::identifier:
				return sooty::make_parseme( parent, ID::identifier, marshall::argument::name(arg)->value.string );
		}
		
		return sooty::parseme_ptr();
	}
	
	/*
	inline sooty::parseme_ptr identifier_from_argument(sooty::parseme_ptr_ref parent, sooty::const_parseme_ptr_ref arg) {
		std::string name;
		switch (arg->id) {
			case ID::int_type:
			case ID::int_literal: name = "int"; break;
			case ID::identifier: name = marshall::argument::name(arg)->value.string; break;
		}
		
		sooty::parseme_ptr N = 
		//marshall::identifier::semantics::variable_definition(N) = arg;
		return N;
	}
	*/
	
//=====================================================================
} // namespace synthesize
} // namespace prism
//=====================================================================
#endif
//=====================================================================
