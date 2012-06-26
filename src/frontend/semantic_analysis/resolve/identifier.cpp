#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/depths.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
//=====================================================================
using namespace prism;

//=====================================================================
// identifier_matches_pred
//=====================================================================
prism::resolve::identifier_matches_pred::identifier_matches_pred( sooty::const_parseme_ptr_ref caller )
	: caller(caller)
{
	ATMA_ASSERT(caller->id == ID::identifier);
}

bool prism::resolve::identifier_matches_pred::operator()( sooty::const_parseme_ptr_ref candidate ) const
{
	return (candidate->id == ID::variable_definition && marshall::variable_definition::name(candidate)->value.string == caller->value.string) ||
	       (candidate->id == ID::parameter && marshall::parameter::name(candidate)->value.string == caller->value.string)
	       ;
}

namespace {
	bool is_valid_vd_location(sooty::const_parseme_ptr_ref N, sooty::const_parseme_ptr_ref I)
	{
		switch (N->id) {
			case ID::function:
			case ID::type_definition:
				return true;
			
			case ID::member_operator:
				return marshall::binary_expression::rhs(N) == I;
			
			default:
				return false;
		}
	}
}

sooty::parseme_ptr prism::resolve::identifier_to_variable_definition(sooty::const_parseme_ptr_ref N)
{
	ATMA_ASSERT(N && N->id == ID::identifier);
	
	// blah
	sooty::parseme_container matches;
	
	// find all ancestors that could house our variable-definition
	sooty::parseme_container locations;
	sooty::direct_upwards_copy_if(std::back_inserter(locations), N, boost::bind(is_valid_vd_location, _1, N));
	
	for (sooty::parseme_container::iterator i = locations.begin(); i != locations.end(); ++i)
	{
		size_t id = (*i)->id;
		
		if ( id == ID::type_definition ) {
			sooty::parseme_ptr possible_parameter = resolve::parameter_of_type_definition(*i, N->value.string);
			if (possible_parameter)
				matches.push_back(possible_parameter);
			sooty::parseme_ptr possible_match = resolve::member_of_type_definition(*i, N->value.string);
			if (possible_match)
				matches.push_back(possible_match);
		}
		else if ( id == ID::function ) {
			// try parameters first
			sooty::const_parseme_ptr_ref parameters = marshall::function::parameter_list(*i);
			sooty::depth_first_copy_if( std::back_inserter(matches), parameters, identifier_matches_pred(N) );
			
			// then try the body of the function
			sooty::const_parseme_ptr_ref body = marshall::function::body(*i);
			sooty::depth_first_copy_if( std::back_inserter(matches), body, identifier_matches_pred(N) );
		}
		else if ( id == ID::member_operator ) {
			sooty::parseme_ptr type_of_lhs = resolve::type_of(marshall::binary_expression::lhs(*i), resolve::remove_refs());
			sooty::parseme_ptr possible_match = resolve::member_of_type_definition(type_of_lhs, N->value.string);
			if (possible_match)
				matches.push_back(possible_match);
		}
	}
	
	if (matches.empty())
		return sooty::parseme_ptr();
	
	return matches.front();
}