#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parser.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/function.hpp>


void prism::semantic_analysis::add_implicit_this(sooty::parseme_ptr_ref function)
{
	using namespace prism;
	
	sooty::parseme_ptr member_definitions = function->parent.lock();
	if (member_definitions->id != ID::member_definitions)
		return;
	
	sooty::parseme_ptr type_definition = member_definitions->parent.lock();
	
	// create a type-identifier that corresponds with the "this"
	sooty::parseme_ptr ti = sooty::make_parseme(sooty::parseme_ptr(), ID::type_identifier, marshall::type_definition::name(type_definition)->value.string);
	ti->children.push_back( sooty::make_parseme(ti, ID::argument_list) );
	
	// add the arguments that made up this type-definition
	for (sooty::parseme_container::iterator i = marshall::type_definition::arguments(type_definition)->children.begin();
	 i != marshall::type_definition::arguments(type_definition)->children.end(); ++i)
	{
		ti->children.back()->children.push_back( sooty::clone_tree(*i) );
		ti->children.back()->children.back()->parent = ti->children.back();
	}
	              
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

void prism::semantic_analysis::analyse::type_definition_signature(semantic_info& si, sooty::parseme_ptr_ref N)
{
	// parameters
	sooty::parseme_ptr_ref parameters = marshall::type_definition::parameters(N);
	std::for_each( parameters->children.begin(), parameters->children.end(), boost::bind(analyse::parameter, boost::ref(si), _1) );
}

void prism::semantic_analysis::analyse::type_definition_interface(semantic_info& si, sooty::parseme_ptr_ref N)
{
	sooty::parseme_ptr_ref members = marshall::type_definition::members(N);
	
	std::sort(members->children.begin(), members->children.end(), sooty::id_less_than_pred());
	
	// 1) all functions must have an implicit "this" parameter inserted at the front
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> member_functions = 
		std::equal_range(members->children.begin(), members->children.end(), prism::ID::function, sooty::id_less_than_pred());
	std::for_each(member_functions.first, member_functions.second, boost::bind(add_implicit_this, _1));
	
	// 3) all variable definitions
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> member_variables = 
		std::equal_range(members->children.begin(), members->children.end(), prism::ID::variable_definition, sooty::id_less_than_pred());
	std::for_each( member_variables.first, member_variables.second, boost::bind(statement, boost::ref(si), _1) );
}

void prism::semantic_analysis::analyse::type_definition(semantic_info& si, sooty::parseme_ptr_ref N)
{
	ATMA_ASSERT( marshall::type_definition::parameters(N)->children.empty() );
	
	sooty::parseme_ptr_ref members = marshall::type_definition::members(N);
	std::sort(members->children.begin(), members->children.end(), sooty::id_less_than_pred());
	
	// all variable definitions
	//std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> member_variables = 
	//	std::equal_range(members->children.begin(), members->children.end(), prism::ID::variable_definition, sooty::id_less_than_pred());
	//std::for_each( member_variables.first, member_variables.second, boost::bind(statement, boost::ref(si), _1) );
	
	// all functions
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> member_functions = 
		std::equal_range(members->children.begin(), members->children.end(), prism::ID::function, sooty::id_less_than_pred());
	
	// 1) all functions must have an implicit "this" parameter inserted at the front
	//std::for_each(member_functions.first, member_functions.second, boost::bind(add_implicit_this_reference, boost::cref(N), _1));
	
	std::for_each(member_functions.first, member_functions.second, boost::bind(analyse::function, boost::ref(si), _1));
}


