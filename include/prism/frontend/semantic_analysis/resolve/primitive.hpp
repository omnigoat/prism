#ifndef KALEIDOSCOPE_RESOLVE_PRIMITIVE_HPP
#define KALEIDOSCOPE_RESOLVE_PRIMITIVE_HPP
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
//=====================================================================
namespace prism {
namespace resolve {
//=====================================================================

	//=====================================================================
	//
	//  primitive_to_definition
	//  -------------------------
	//    takes a primitive (int-literal/real-literal/etc), and returns its
	//    definition.
	//
	//=====================================================================
	namespace detail {
		inline bool is_root(sooty::const_parseme_ptr_ref n) {
			return n->id == ID::root;
		}
	}
	
	inline sooty::parseme_ptr primitive_to_definition(sooty::const_parseme_ptr_ref N)
	{
		ATMA_ASSERT(N);
		//ATMA_ASSERT(N->id == ID::int_literal || N->id == ID::int_type);
		size_t id_to_match = 0;
		switch (N->id) {
			case ID::int_type:
			case ID::int_literal:
				id_to_match = ID::int_type;
				break;
			
			case ID::bool_type:
			case ID::bool_literal:
				id_to_match = ID::bool_type;
				break;
			
			case ID::real_type:
			case ID::real_literal:
				id_to_match = ID::real_type;
				break;
				
			default:
				ATMA_HALT("bad~~~!");
		}
		
		// go up to top.
		sooty::parseme_ptr root = sooty::direct_upwards_find_first_if(N, detail::is_root);
		ATMA_ASSERT(root);
		
		// we know that all primitive definitions are at top-levelness
		//for (sooty::parseme_container::const_iterator i = root->children.begin(); i != root->children.end(); ++i)
		//{
			//if ((*i)->id == ID::type_definition) {
				//if ( marshall::type_definition::(*i)->children[0]->id == id_to_match )
					//return *i;
			//}
		//}
		
		ATMA_ASSERT(false);
		return sooty::parseme_ptr();
	}
	
//=====================================================================
} // namespace resolve
} // namespace prism
//=====================================================================
#endif
//=====================================================================
