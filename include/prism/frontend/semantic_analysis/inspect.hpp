//=====================================================================
//
//=====================================================================
#ifndef PRISM_INSPECT_HPP
#define PRISM_INSPECT_HPP
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parseme.hpp>
//=====================================================================
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
namespace prism {
namespace inspect {
//=====================================================================
	
	inline int size_of(sooty::const_parseme_ptr_ref N)
	{
		return -1;
	}
	
	inline bool can_fit_in_register(sooty::const_parseme_ptr_ref N)
	{
		//ATMA_ASSERT(N->id == ID::type_definition);
		
		switch (N->id)
		{
			case ID::pointer_type:
			case ID::reference_type:
				return true;
			
			case ID::type_definition:
			{
				sooty::const_parseme_ptr_ref name = marshall::type_definition::name(N);
			
				if (name->value.string == "int" || name->value.string == "real" || name->value.string == "bool")
					return true;
				else if (name->value.string == "void")
					return false;
				else
					return can_fit_in_register( marshall::type_definition::members(N) );
			}
			
			case ID::type_identifier:
				return can_fit_in_register( resolve::type_of(N) );
			
			default:
				return false;
		}
	}
	
	bool is_intrinsic_function(sooty::parseme_ptr_ref);
	
//=====================================================================
} // namespace inspect
} // namespace prism
//=====================================================================
#endif
//=====================================================================
