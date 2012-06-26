//=====================================================================
//
//
//=====================================================================
#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
//=====================================================================
using namespace prism;

// if a type can fit into a register, then simply use a register. otherwise,
// we must use a const-reference! :D
void llvm::parameter(sooty::const_parseme_ptr_ref N)
{
	ATMA_ASSERT(N->id == ID::parameter);
	
	if ( inspect::can_fit_in_register(resolve::type_of(N)) ) {
		output_stream() << logical_type_name(N) << " " << lvalue_name(N);
	}
	else {
		output_stream() << storage_type_name( marshall::parameter::type(N) ) << " nocapture " << lvalue_name(N);
	}
}