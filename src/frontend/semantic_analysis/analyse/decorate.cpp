#include <prism/frontend/semantic_analysis/decorate.hpp>
//=====================================================================
#include <sstream>
//=====================================================================
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <atma/string_builder.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
//=====================================================================
using namespace atma;
//=====================================================================

namespace {
	void list_of_tds(sooty::const_parseme_ptr_ref parent, sooty::parseme_container& parents) {
		if (parent->id == prism::ID::member_definitions) {
			list_of_tds(parent->parent.lock(), parents);
		}
		else if (parent && parent->id == prism::ID::type_definition) {
			parents.push_back(parent);
			list_of_tds(parent->parent.lock(), parents);
		}
	}
}

std::string prism::decorate_function(sooty::const_parseme_ptr_ref node)
{
	// special logic for main()
	if ( prism::marshall::function::name(node)->value.string == "main" )
		return "main";
	
	// fucntions are preceded by "F"
	std::stringstream decorated_name;
	decorated_name << "F";


	// if a function is a child of a type-definition, preface by the type-definition
	sooty::parseme_container parents;
	list_of_tds(node->parent.lock(), parents);
	for (sooty::parseme_container::const_reverse_iterator i = parents.rbegin(); i != parents.rend(); ++i)
	{
		decorated_name << decorate_type(*i);
	}
	
	
	// name
	sooty::const_parseme_ptr_ref name = prism::marshall::function::name(node);
	decorated_name
		<< name->value.string.size()
		<< name->value.string;
	
	// return value
	decorated_name << decorate_type(prism::marshall::function::return_type(node));
	
	// parameters
	sooty::const_parseme_ptr_ref pl = marshall::function::parameter_list(node);
	for (sooty::parseme_container::iterator i = pl->children.begin(); i != pl->children.end(); ++i)
	{
		decorated_name << decorate_type(prism::marshall::variable_definition::type(*i));
	}
	
	return decorated_name.str();
}


std::string prism::decorate_type(sooty::const_parseme_ptr_ref n)
{
	using namespace prism;
	
	if (n->id == ID::pointer_type) {
		return "P" + prism::decorate_type(n->children.front());
	}
	else if (n->id == ID::int_type) {
		return "Ti";
	}
	else if (n->id == ID::real_type) {
		return "Tr";
	}
	else if (n->id == ID::bool_type) {
		return "Tb";
	}
	else if (n->id == ID::void_type) {
		return "Tv";
	}
	else if (n->id == ID::type_definition) {
		std::string& name = marshall::type_definition::name(n)->value.string;
		//return "T" + 
		return atma::string_builder("T")(name.size())(name);
	}
	else // if (n->id == ID::identifier)
	{
		return atma::string_builder("T")(n->value.string.size())(n->value.string);
	}
}
