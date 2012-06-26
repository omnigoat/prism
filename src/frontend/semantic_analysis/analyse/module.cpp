#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parser.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
#include <prism/frontend/semantic_analysis/resolve/primitive.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
//=====================================================================
using namespace prism;

namespace {
	bool remove_function(sooty::cuil::matching_context_ref context)
	{
		sooty::parseme_ptr_ref N = *context.begin;
		
		// must be a function
		if (N->id != ID::function)
			return false;
		// must not be the main function
		else if ( marshall::function::name(N)->value.string == "main" )
			return false;
		// not an intrinsic function
		else if ( N->value.integer == 1 )
			return false;
		// not used
		else if ( marshall::function::semantics::use_count(N).integer > 0 )
			return false;
		
		N->parent.lock()->children.erase( sooty::position_of(N) );
		return true;
	}
}

namespace {
	bool remove_templated_type(sooty::cuil::matching_context_ref context)
	{
		sooty::parseme_ptr_ref N = *context.begin;
		if ( N->id != ID::type_definition )
			return false;
		else if ( marshall::type_definition::parameters(N)->children.empty() )
			return false;
		N->parent.lock()->children.erase( sooty::position_of(N) );
		return true;
	}
}

namespace {
	bool is_member_function(sooty::const_parseme_ptr_ref N)
	{
		return N->id == ID::function && N->parent.lock()->id == ID::member_definitions;
	}
}

namespace {
	void add_implicit_this(sooty::parseme_ptr_ref function)
	{
		using namespace prism;
		
		sooty::parseme_ptr member_definitions = function->parent.lock();
		if (member_definitions->id != ID::member_definitions)
			return;
		
		sooty::parseme_ptr type_definition = member_definitions->parent.lock();
		
		sooty::parseme_ptr ti = sooty::make_parseme(sooty::parseme_ptr(), ID::type_identifier, marshall::type_definition::name(type_definition)->value.string);
		ti->children.push_back( sooty::make_parseme(ti, ID::argument_list) );
		std::for_each(marshall::type_definition::arguments(type_definition)->children.begin(),
		              marshall::type_definition::arguments(type_definition)->children.end(),
		              boost::bind(&sooty::parseme_container::push_back, boost::ref(ti->children), _1));
		              
		sooty::parseme_ptr_ref parameter_list = marshall::function::parameter_list(function);
		sooty::immediate_assign(parameter_list, parameter_list->children, parameter_list->children.begin()) (
			sooty::insert(ID::parameter) [
				sooty::insert(ID::identifier, "this"),
				sooty::insert(ID::reference_type) [
					sooty::insert(ti)
				]
			]
		);
	}
}

namespace {
	using namespace prism::semantic_analysis;
	void type_definition_interface_action(semantic_info& si, sooty::parseme_ptr_ref N) {
		if ( marshall::type_definition::parameters(N)->children.empty() )
			analyse::type_definition_interface(si, N);
	}
}

namespace {
	using namespace prism::semantic_analysis;
	void type_definition_action(semantic_info& si, sooty::parseme_ptr_ref N) {
		if ( marshall::type_definition::parameters(N)->children.empty() )
			analyse::type_definition(si, N);
	}
}

void prism::semantic_analysis::analyse::module(semantic_info& si, sooty::parseme_ptr_ref N)
{
	// add the integral operators!
	module_integral_operators(N);
	
	
	// sort by ID, so that we have all similar things together!
	std::sort(N->children.begin(), N->children.end(), sooty::id_less_than_pred());
	
	// add the implicit "this" parameter to all member functions
	//sooty::parseme_container member_functions;
	//sooty::depth_first_copy_if( std::back_inserter(member_functions), N, is_member_function );
	//std::for_each(member_functions.begin(), member_functions.end(), add_implicit_this);
	
	
	
	// analyse all type-definitions
		std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> type_definitions_range = 
			std::equal_range(N->children.begin(), N->children.end(), prism::ID::type_definition, sooty::id_less_than_pred());
		sooty::parseme_container type_definitions(type_definitions_range.first, type_definitions_range.second);
		// *all* type-definitions have their signature analysed
		std::for_each(type_definitions.begin(), type_definitions.end(), boost::bind(analyse::type_definition_signature, boost::ref(si), _1));
		// only "regular" types get their interfaces and bodies analysed atm
		std::for_each(type_definitions.begin(), type_definitions.end(), boost::bind(type_definition_interface_action, boost::ref(si), _1));
		std::for_each(type_definitions.begin(), type_definitions.end(), boost::bind(type_definition_action, boost::ref(si), _1));
	
	// analyse all function-signatures and then all functions
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> range = 
		std::equal_range(N->children.begin(), N->children.end(), prism::ID::function, sooty::id_less_than_pred());
	sooty::parseme_container functions(range.first, range.second);
	
	/*for (int i = std::distance(N->children.begin(), range.first); i != std::distance(N->children.begin(), range.second); ) {
		int before = N->children.
		analyse::function_signature(si, N->children[i]);
		if (
	*/
	
	std::for_each(functions.begin(), functions.end(), boost::bind(function_signature, boost::ref(si), _1));
	std::for_each(functions.begin(), functions.end(), boost::bind(function, boost::ref(si), _1));
	
	
	// remove all unused functions
	//tree_pattern(N, remove_function, no_pattern());	
	
	// remove all template-types
	sooty::cuil::tree_pattern(N, remove_templated_type, sooty::cuil::no_pattern());
}