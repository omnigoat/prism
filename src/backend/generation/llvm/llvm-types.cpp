#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <numeric>
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/operator.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
//=====================================================================
using namespace prism;
//=====================================================================

// :'(
bool sad_flag_about_uptype = false;

int value_of_character(int lhs, char c) {
	if (c == '\n')
		return lhs + 2;
	else
		return lhs + 1;
}

//=====================================================================
//
//
//
//=====================================================================
std::string llvm::logical_type_name(sooty::const_parseme_ptr_ref N)
{
	switch (N->id)
	{
		// void
		case ID::void_type:
			return "void";
		
		// integer
		case ID::int_literal:
		case ID::intrinsic_int_add:
		case ID::intrinsic_int_sub:
		case ID::intrinsic_int_mul:
		case ID::intrinsic_int_div:
			return "i32";
		
		case ID::int_type:
		{
			if (N->children.empty())
				return "i32";
			else {
				return atma::string_builder("i")(N->children.front()->value.integer);
			}
		}
		
		// real
		case ID::real_type:
		case ID::real_literal:
			return "float";
		
		// boolean
		case ID::bool_type:
		case ID::bool_literal:
		case ID::intrinsic_int_eq:
			return "i1";
		
		case ID::string_literal:
		{
			const std::string& str = N->value.string;
			int size = std::accumulate(str.begin(), str.end(), 0, value_of_character);
			return atma::string_builder("[")(size + 1)(" x i8]");
		}
		
		case ID::reference_type:
			//return logical_type_name(N->children.front())
		case ID::pointer_type:
			return logical_type_name(N->children.front()) + "*";

		case ID::identifier:
			return logical_type_name( resolve::identifier_to_variable_definition(N) );
		
		//=====================================================================
		// we have to make sure we don't start a cyclic lookup going. llvm doesn't like it either
		// (well they need to stop somewhere!), by referencing enclosing types as "\2". so:
		//
		//    type giraffe
		//        var age as int
		//        var next as giraffe*
		//
		// becomes: {i32, \2*}
		//=====================================================================
		case ID::type_identifier:
		{
			// get all type-definitions that we are a child of
			sooty::parseme_container tds;
			sooty::direct_upwards_copy_if(std::back_inserter(tds), N, sooty::id_matches(ID::type_definition));
			
			// now find out if our type_identifier is their name
			sooty::parseme_container::iterator referred_type = tds.begin();
			for (; referred_type != tds.end(); ++referred_type) {
				if ( marshall::type_definition::name(*referred_type)->value.string == N->value.string )
					break;
			}
			
			// if it's not, go as usual
			if ( !sad_flag_about_uptype || referred_type == tds.end() ) {
				return logical_type_name( resolve::type_of(N) );
			}
			// if it is, then we figure out our depth (depth of type-definitions, not tree-depth),
			// and add two - one because we technically count from before begin(), and one because
			// llvm uses \1 to reference itself. \2 is used to reference a parent type-definition,
			// as shown above
			else {
				int depth = std::distance(tds.begin(), referred_type) + 2;
				return atma::string_builder("\\")(depth);
			}
		}
		
		
		case ID::variable_definition:
		{
			return logical_type_name( marshall::variable_definition::type(N) );
		}
		
		case ID::composite_type:
		{
			sad_flag_about_uptype = true;
			std::string result = "{";
			for (sooty::parseme_container::const_iterator i = N->children.begin(); i != N->children.end(); ++i)
			{
				if ( i != N->children.begin() )
					result += ", ";
				
				ATMA_ASSERT( (*i)->id == ID::variable_definition );
				result += logical_type_name(*i);
			}
			result += "}";
			sad_flag_about_uptype = false;
			return result;
		}
		
		case ID::array_type:
			return atma::string_builder("[")(marshall::array_type::arity(N)->value.integer)(" x ")(logical_type_name(marshall::array_type::type(N)))("]");
		
		case ID::type_definition:
		{
			sooty::const_parseme_ptr_ref name = marshall::type_definition::name(N);
			if (N->value.integer == 1)
				return atma::string_builder("%")(name->value.string)("_t");
			else if (name->value.string == "int")
			{
				sooty::parseme_ptr members = marshall::type_definition::members(N);
				sooty::parseme_container::iterator intrinsic_int_type =
					std::find_if(members->children.begin(), members->children.end(), sooty::id_matches(ID::int_type));
				ATMA_ASSERT(intrinsic_int_type != members->children.end());
				return logical_type_name(*intrinsic_int_type);
			}
			else if (name->value.string == "char")
				return "i8";
			else if (name->value.string == "real")
				return "float";
			else if (name->value.string == "bool")
				return "i1";
			else if (name->value.string == "void")
				return "void";
			else if (name->value.string == "varags")
				return "...";
			else
				return logical_type_name( marshall::type_definition::members(N) );
		}
		
		case ID::parameter:
			return logical_type_name( marshall::parameter::type(N) );
		
		case ID::un_reference:
		case ID::dereference:
		{
			std::string s = logical_type_name(N->children.front());
			s.erase(s.find_last_of('*'), 1);
			return s;
		}
		
		case ID::address_of:
			return logical_type_name( N->children.front() ) + "*";
			
		case ID::new_:
		{
			if (marshall::new_::type(N)->id == ID::array_type) {
				return logical_type_name( marshall::array_type::type(marshall::new_::type(N)) ) + "*";
			}
			return logical_type_name( marshall::new_::type(N) ) + "*";
		}
		
		case ID::member_function_call:
		case ID::function_call:
			return logical_type_name( resolve::type_of(N) );
		
		case ID::function:
		{
			return logical_type_name( marshall::function::return_type(N) );
		}
		
		case ID::mul:
		case ID::div:
		case ID::add:
		case ID::sub:
		case ID::eq_operator:
			return logical_type_name( marshall::binary_expression::lhs(N) );
		
		
		
		case ID::equ:
			return logical_type_name( resolve::operator_matches(N) );
		
		case ID::member_operator:
			return logical_type_name( resolve::type_of(N) );
		
		case ID::index_operator:
			return logical_type_name( resolve::type_of(N) );
		
		// member-definitions with a value of 1 are intrinisc types. they do
		// not have braces around their type (and they must only have ONE
		// listed member)
		case ID::member_definitions:
		{
			std::string result;
			result += "{";
			
			int vds = 0;
			for (sooty::parseme_container::const_iterator i = N->children.begin(); i != N->children.end(); ++i)
			{
				sooty::const_parseme_ptr_ref member = *i;
				if (member->id != ID::variable_definition)
					continue;
				if (vds > 0)
					result += ", ";
				result += logical_type_name(*i);
				++vds;
			}
			
			result += "}";
			return result;
		}
		
		default:
			return std::string("logical_type_name::UNKNOWN");
	}
}



//=====================================================================
//
//
//
//=====================================================================
std::string prism::llvm::storage_type_name( sooty::const_parseme_ptr_ref N )
{
	switch (N->id)
	{
		case ID::int_literal:
		case ID::int_type:
			return "i32*";

		case ID::real_type:
			return "float*";
		
		case ID::bool_type:
			return "i1*";
		
		case ID::string_literal:
			return logical_type_name(N) + "*";
		
		case ID::array_type:
		case ID::composite_type:
			return logical_type_name(N) + "*";
		
		case ID::identifier:
			return storage_type_name( resolve::identifier_to_variable_definition(N) ); //marshall::identifier::semantics::variable_definition(N).parseme );

		case ID::variable_definition:
			return storage_type_name( marshall::variable_definition::type(N) );
		
		case ID::parameter:
			if ( inspect::can_fit_in_register(marshall::parameter::type(N)) )
				return logical_type_name( marshall::parameter::type(N) );
			else
				return storage_type_name( marshall::parameter::type(N) );
		
		
		
		case ID::reference_type:
			return storage_type_name(N->children.front()) + "*";
		
		case ID::pointer_type:
			return logical_type_name(N) + "*";
		
		case ID::type_definition:
		{
			sooty::const_parseme_ptr_ref name = marshall::type_definition::name(N);
			if (name->value.string == "int")
				return "i32*";
			else if (name->value.string == "real")
				return "float*";
			else if (name->value.string == "bool")
				return "i1*";
			else if (name->value.string == "void")
				return "void*";
			else
				return logical_type_name( marshall::type_definition::members(N) ) + "*";
		}
		
		
		// un-reference is for references, dereference is for pointers... hmm...
		case ID::un_reference:
		case ID::dereference:
		{
			// our child is a pointer to something (has to be!)
			// therefore, our type is the logical-type-name of the pointer
			return logical_type_name(N->children.front());
		}
		
		
		//=====================================================================
		// we have to make sure we don't start a cyclic lookup going. llvm doesn't like it either
		// (well they need to stop somewhere!), by referencing enclosing types as "\2". so:
		//
		//    type giraffe
		//        var age as int
		//        var next as giraffe*
		//
		// becomes: {i32, \2*}
		//=====================================================================
		case ID::type_identifier:
		{
			// get all type-definitions that we are a child of
			sooty::parseme_container tds;
			sooty::direct_upwards_copy_if(std::back_inserter(tds), N, sooty::id_matches(ID::type_definition));
			
			// now find out if our type_identifier is their name
			sooty::parseme_container::iterator referred_type = tds.begin();
			for (; referred_type != tds.end(); ++referred_type) {
				if ( marshall::type_definition::name(*referred_type)->value.string == N->value.string )
					break;
			}
			
			// if it's not, go as usual
			if ( !sad_flag_about_uptype || referred_type == tds.end() ) {
				return logical_type_name( resolve::type_of(N) ) + "*";
			}
			// if it is, then we figure out our depth (depth of type-definitions, not tree-depth),
			// and add two - one because we technically count from before begin(), and one because
			// llvm uses \1 to reference itself. \2 is used to reference a parent type-definition,
			// as shown above
			else {
				int depth = std::distance(tds.begin(), referred_type) + 2;
				return atma::string_builder("\\")(depth);
			}
		}
		
		case ID::member_function_call:
		case ID::function_call:
			return logical_type_name(N) + "*";
		
		case ID::member_operator:
		case ID::index_operator:
			return storage_type_name( resolve::type_of(N) );
		
		default:
			return std::string("storage_type_name::UNKNOWN");
	}
}


