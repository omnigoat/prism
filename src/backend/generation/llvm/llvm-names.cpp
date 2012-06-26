#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <atma/string_builder.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/operator.hpp>
//=====================================================================
using namespace prism;
//=====================================================================


namespace
{
	std::string lvalue_dereference_name(sooty::const_parseme_ptr_ref N)
	{
		ATMA_ASSERT(N);
		switch (N->id)
		{
			case ID::identifier:
			{
				sooty::const_parseme_ptr_ref definition = resolve::identifier_to_variable_definition(N);
				if (marshall::identifier::definition_is_parameter(N)) {
					return llvm::lvalue_name(definition);
				}
				else {
					return llvm::rvalue_name(N);
				}
			}
		}
		
		return std::string("BAD BAD BAD");
	}
}

//=====================================================================
//
//
//
//=====================================================================
std::string llvm::lvalue_name(sooty::const_parseme_ptr_ref N)
{
	ATMA_ASSERT(N);
	
	//return atma::string_builder("%lv")(N->position.guid());
	switch (N->id)
	{
		case ID::string_literal:
			return atma::string_builder("@.str")(N->position.guid());
		
		case ID::int_literal:
			return atma::string_builder("%lit")(N->position.guid());
			
		case ID::identifier:
			return lvalue_name( resolve::identifier_to_variable_definition(N) );
		
		case ID::variable_definition:
			return atma::string_builder("%")( marshall::variable_definition::name(N)->value.string );
		
		case ID::parameter:
			return atma::string_builder("%")( marshall::parameter::name(N)->value.string );
		
		case ID::un_reference:
		{
			if ( N->children.front()->id == ID::identifier && marshall::identifier::definition_is_parameter(N->children.front()) )
				return lvalue_name(N->children.front());
			else
				return atma::string_builder("%r")(N->position.guid())(".un_ref");
		}
		
		case ID::address_of:
			return lvalue_name(N->children.front());
			
		// the lvalue name of dereferencing is the identity of the rvalue name of the child
		case ID::dereference:
			return rvalue_name(N->children.front());
		
		case ID::member_operator:
		case ID::index_operator:
			return rvalue_name(N) + ".gt";
		
		case ID::member_function_call:
		case ID::function_call:
			return rvalue_name(N) + ".lv";
		
		case ID::add:
		case ID::sub:
		case ID::mul:
		case ID::div:
		case ID::new_:
			return rvalue_name(N);
	}
	
	ATMA_HALT("bad~~~");
	return "lvalue_name::UNKNOWN_VALUE";
}

namespace
{
	std::string float_to_hex_string(float f)
	{
		union {
			double f;
			unsigned char c[sizeof(double)];
		} v;
		v.f = f;

		static char buffer[18] = "0x\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

		std::fill(buffer + 2, buffer + 16, '\0');

		int written = 2;
		for ( int i = sizeof(v.c) - 1; i >= 0; --i ) {
			written += sprintf(buffer + written, "%02x", static_cast<int>(v.c[i]));
		}

		return std::string(buffer, buffer + 18);
	}
}

//=====================================================================
//
//
//
//=====================================================================
std::string llvm::rvalue_name(sooty::const_parseme_ptr_ref N)
{
	ATMA_ASSERT(N);
	
	switch (N->id)
	{
		case ID::int_literal:
			return boost::lexical_cast<std::string>(N->value.integer);
		
		case ID::bool_literal:
			return (N->value.string == "true") ? "1" : "0";
		
		case ID::string_literal:
			return atma::string_builder("%lit")(N->position.guid()); //atma::string_builder("c\"")(N->value.string)("\"");
		
		// real values must be put into hexadecimal notation
		case ID::real_literal:
			return float_to_hex_string(N->value.real);
			
		case ID::identifier:
			if ( marshall::identifier::definition_is_parameter(N) )
				return atma::string_builder("%")(N->value.string);
			
		case ID::add:
		case ID::sub:
		case ID::mul:
		case ID::div:
		case ID::dereference:
		case ID::un_reference:
		case ID::new_:
		case ID::function_call:
		case ID::member_function_call:
		case ID::equ:
		case ID::lt:
		case ID::member_operator:
		case ID::index_operator:
		case ID::intrinsic_int_add:
		case ID::intrinsic_int_sub:
		case ID::intrinsic_int_mul:
		case ID::intrinsic_int_div:
		case ID::intrinsic_int_eq:
		case ID::intrinsic_int_lt:
			return atma::string_builder("%r")(N->position.guid());
		
		case ID::address_of:
		{
			//return atma::string_builder("%")(N->children[0]->value.string);
			return lvalue_name(N->children.front());
		}
		
		case ID::variable_definition:
		case ID::parameter:
			ATMA_HALT("can't have variable_definitions or parameters being used for rvalues!");
			break;

	}
	
	ATMA_HALT("bad~~~!");
	return std::string();
}





