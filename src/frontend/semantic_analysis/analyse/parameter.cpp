#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
//=====================================================================

void prism::semantic_analysis::analyse::parameter(semantic_info& si, sooty::parseme_ptr_ref N)
{
	sooty::parseme_ptr_ref name = marshall::parameter::name(N);
	if (name->value.string == "#unused")
		name->value.string = std::string(".unused") + boost::lexical_cast<std::string>(N->position.guid());
	type(si, prism::marshall::parameter::type(N));
}