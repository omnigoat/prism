#include <prism/frontend/semantic_analysis/reduce.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
//=====================================================================
using namespace prism;
//=====================================================================

namespace {
	struct identifier_to_argument
	{
		const sooty::parseme_container& arguments;
		const sooty::parseme_container& parameters;
		
		identifier_to_argument(const sooty::parseme_container& parameters, const sooty::parseme_container& arguments)
			: parameters(parameters), arguments(arguments)
		{
		}
		
		bool operator ()(sooty::cuil::const_matching_context_ref context)
		{
			sooty::const_parseme_ptr_ref N = *context.begin;
			ATMA_ASSERT(N->id == ID::identifier);
			
			sooty::parseme_container::const_iterator ai = arguments.begin();
			sooty::parseme_container::const_iterator pi = parameters.begin();
			for (; pi != parameters.end(); ++pi, ++ai) {
				sooty::const_parseme_ptr_ref argument = *ai;
				sooty::const_parseme_ptr_ref parameter = *pi;
				if ( marshall::identifier::name(N)->value.string == marshall::parameter::name(parameter)->value.string ) {
					sooty::replace_parseme_with_another(N, sooty::clone_tree(argument));
					return false;
				}
			}
			
			return false;
		}
	};
}

void prism::reduce::reduce_with_template_arguments(sooty::parseme_container& list, const sooty::parseme_container& parameters, const sooty::parseme_container& arguments)
{
	using namespace sooty::cuil;
	
	tree_pattern(list, no_pattern(),
		
		eq(ID::identifier).perform(identifier_to_argument(parameters, arguments))
	
	);
}


