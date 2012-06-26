#ifndef KALEIDOSCOPE_RESOLVE_OPERATOR_HPP
#define KALEIDOSCOPE_RESOLVE_OPERATOR_HPP
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
namespace prism {
namespace resolve {
//=====================================================================
	
	namespace detail {
		struct is_operator {
			size_t id;
			is_operator(size_t id) : id(id) {}
			bool operator ()(sooty::const_parseme_ptr_ref rhs) const {
				return this->id == rhs->id;
			}
		};
		
		struct operator_return_values_match {
			sooty::parseme_ptr lhs;
			operator_return_values_match(sooty::const_parseme_ptr_ref lhs) : lhs(lhs) {}
			bool operator ()(sooty::const_parseme_ptr_ref rhs) const {
				return type_matches_pred(lhs)(rhs->children.front());
			}
		};
	}
	
	inline bool parent_is_operator(sooty::const_parseme_ptr_ref N)
	{
		sooty::parseme_ptr parent(  N->parent.lock()  );
		switch (parent->id) {
			case ID::member_operator:
			case ID::index_operator:
				return true;
		
			default:
				return false;
		}
	}
	
	inline sooty::parseme_ptr operator_matches(sooty::const_parseme_ptr_ref N)
	{
		switch (N->id)
		{
			case ID::equ:
			{
				sooty::parseme_container possibilities;
				upwards_bredth_first_copy_if(N, std::back_inserter(possibilities), detail::is_operator(ID::eq_operator) );
				
				sooty::parseme_container not_matching;
				std::remove_copy_if(possibilities.begin(), possibilities.end(), std::back_inserter(not_matching), type_matches_pred(N->children.front()));
				return possibilities.front();
			}
		}
		
		return sooty::parseme_ptr();
	}
	
//=====================================================================
} // namespace resolve
} // namespace prism
//=====================================================================
#endif
//=====================================================================
