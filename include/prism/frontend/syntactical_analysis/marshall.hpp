#ifndef PRISM_MARSHALL_HPP
#define PRISM_MARSHALL_HPP
//=====================================================================
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
//=====================================================================
namespace prism {
//=====================================================================

	namespace marshall
	{
		//=====================================================================
		// the ugly macro code
		//=====================================================================
		#if defined(DEF) || defined(CHILD) || defined(NODE)
			#error 'DEF', 'CHILD', or 'NODE' already defined
		#else
			#define CHILD(num) \
				sooty::parseme_container::iterator i = n->children.begin(); \
				std::advance(i, num); \
				return *i
			
			
			/*#define CHILD(num) \
				return n->children[num]*/
			
			#define NODE() \
				return n
			
			#define DEF(name, way) \
				inline sooty::const_parseme_ptr_ref name(sooty::const_parseme_ptr_ref n) { way; } \
				inline sooty::parseme_ptr_ref name(sooty::parseme_ptr_ref n) { way; }
			
			#define SEMANT(name) \
				namespace semantics { \
					inline sooty::semant_ref name(sooty::parseme_ptr_ref N) { return N->semantics[#name]; } \
					inline sooty::const_semant_ref name(sooty::const_parseme_ptr_ref N) { return N->semantics[#name]; } \
				}
		#endif
		
		
		//=====================================================================
		//
		//  if you ever wanted an easy way to view how the tree would be flat,
		//  then this file is probably your best bet.
		//
		//=====================================================================
		namespace function_declaration
		{
			DEF( linkage,			CHILD(0) )
			DEF( calling,			CHILD(1) )
			DEF( name,				CHILD(2) )
			DEF( parameter_list,	CHILD(3) )
			DEF( return_type,		CHILD(4) )
		}
		
		namespace function
		{
			DEF( linkage,			CHILD(0) )
			DEF( calling,			CHILD(1) )
			DEF( name,				CHILD(2) )
			DEF( parameter_list,	CHILD(3) )
			DEF( return_type,		CHILD(4) )
			DEF( init_list,         CHILD(5) )
			DEF( body,				CHILD(6) )
			
			SEMANT(use_count)
		}
		
		namespace function_call
		{
			DEF( name,				CHILD(0) )
			DEF( argument_list,		CHILD(1) )
		}
		
		namespace member_function_call
		{
			DEF( name,				CHILD(0) )
			DEF( argument_list,		CHILD(1) )
			DEF( type_definition,	CHILD(2) )
		}
		
		namespace argument
		{
			DEF( name,				NODE() )
			DEF( type,				NODE() )
		}
		
		namespace return_statement
		{
			DEF( expression,		CHILD(0) )
		}
		
		namespace variable_definition
		{
			DEF( name,				CHILD(0) )
			DEF( type,				CHILD(1) )
			DEF( arguments,         CHILD(2) )
			//DEF( defvalue,			CHILD(2) )
			//inline bool has_defvalue(sooty::const_parseme_ptr_ref N) { return N->children.size() == 3; }
		}
		
		namespace type_identifier
		{
			DEF( identifier, NODE() )
			DEF( arguments, CHILD(0) )
			
			SEMANT( type_definition )
		}
		
		namespace parameter
		{
			DEF( name,				CHILD(0) )
			DEF( type,				CHILD(1) )
			DEF( defvalue,			CHILD(2) )
			inline bool has_defvalue(sooty::const_parseme_ptr_ref N) { return N->children.size() == 3; }
		}
		
		namespace identifier
		{
			DEF( name,				NODE()		)
			//DEF( definition,		CHILD(0)	)
			
			SEMANT( variable_definition )
			SEMANT( type_definition )
			
			inline bool definition_is_parameter(sooty::const_parseme_ptr_ref N) {
				sooty::parseme_ptr p = resolve::identifier_to_variable_definition(N);
				ATMA_ASSERT(p);
				return p->id == ID::parameter;
			}
		}
		
		namespace type_definition
		{
			DEF( name,				CHILD(0)    )
			DEF( parameters,		CHILD(1)    )
			DEF( arguments,         CHILD(2)    )
			DEF( members,			CHILD(3)    )
			
			SEMANT( use_count )
		}
		
		namespace if_statement
		{
			DEF( expression,        CHILD(0)    )
			DEF( true_block,        CHILD(1)    )
			DEF( false_block,		CHILD(2)    )
		}
		
		namespace binary_expression
		{
			DEF( lhs, CHILD(0) )
			DEF( rhs, CHILD(1) )
			
			SEMANT( type )
		}
		
		namespace new_
		{
			DEF( type, CHILD(0) )
			DEF( arguments, CHILD(1) )
			//inline bool has_defvalue(sooty::const_parseme_ptr_ref N) { return N->children.size() == 2; }
		}
		
		namespace loop_statement
		{
			DEF( variable, CHILD(0) )
			DEF( control, CHILD(1) )
			DEF( condition, CHILD(2) )
			DEF( via, CHILD(3) )
			DEF( block, CHILD(4) )
		}
		
		namespace array_type
		{
			DEF( type, CHILD(0) )
			DEF( arity, CHILD(1) )
		}
		
		namespace pointer_type
		{
			DEF( pointee_type, CHILD(0) )
		}
		
		namespace assignment
		{
			DEF( lhs, CHILD(0) )
			DEF( rhs, CHILD(1) )
		}
	}
	
	#undef DEF
	#undef CHILD
	#undef NODE

//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
