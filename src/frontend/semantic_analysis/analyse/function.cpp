#include <prism/frontend/semantic_analysis/analyse.hpp>
#include <prism/frontend/semantic_analysis/function.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>


std::string undecorated_function(sooty::const_parseme_ptr_ref N)
{
	using namespace prism;
	
	sooty::const_parseme_ptr_ref name = marshall::function::name(N);
	sooty::const_parseme_ptr_ref parameter_list = marshall::function::parameter_list(N);
	sooty::const_parseme_ptr_ref return_type = marshall::function::return_type(N);
	sooty::const_parseme_ptr_ref body = marshall::function::body(N);
	
	std::string result = "function " + name->value.string + "(";
	for (sooty::parseme_container::const_iterator i = parameter_list->children.begin(); i != parameter_list->children.end(); ++i)
	{
		sooty::const_parseme_ptr_ref parameter = *i;
		result += marshall::parameter::type(parameter)->value.string;
	}
	
	result += ")";
	result += "->" + return_type->value.string;
	return result;
}

void prism::semantic_analysis::analyse::function_signature(semantic_info& si, sooty::parseme_ptr_ref N)
{
	sooty::parseme_ptr_ref name = marshall::function::name(N);
	sooty::parseme_ptr_ref parameter_list = marshall::function::parameter_list(N);
	sooty::parseme_ptr_ref return_type = marshall::function::return_type(N);
	sooty::parseme_ptr_ref init_list = marshall::function::init_list(N);
	sooty::parseme_ptr_ref body = marshall::function::body(N);

	// return-type
	type(si, return_type);
	// parameters
	std::for_each( parameter_list->children.begin(), parameter_list->children.end(), boost::bind(&parameter, boost::ref(si), _1) );
	// initialiser-list
	//std::for_each( init_list->children.begin(), init_list->children.end(), boost::bind(&analyse::
	for (sooty::parseme_container::iterator i = init_list->children.begin(); i != init_list->children.end(); ++i)
	{
		analyse::expression(si, marshall::binary_expression::lhs(*i));
		analyse::expression(si, marshall::binary_expression::rhs(*i));
	}
	
	//=====================================================================
	// check special stuff for main
	//=====================================================================
	if (name->value.string == "main") {
		if ( resolve::type_of_pred(return_type)()->value.string == "int") {
			std::cerr << "error (" << N->position << "): main's return type should be int" << std::endl;
			++si.errors;
		}
	}
	
	//=====================================================================
	// check for ambiguities
	//=====================================================================
	sooty::parseme_container matching_functions;
	// we only need to use a linear-upwards algorithm, since if the duplicate function definition is later,
	// it will be picked up by the semantic analysis for that node
	sooty::linear_upwards_copy_if(N, std::back_inserter(matching_functions), prism::resolve::function_matches_function_pred(N));
	// remove ourselves from the list
	matching_functions.erase( std::remove(matching_functions.begin(), matching_functions.end(), N), matching_functions.end() );
	// sort based upon function ordering. it's complex.
	std::sort(matching_functions.begin(), matching_functions.end(), depth_pred());
	//matching_functions.sort(function_pred());

	// we need to limit our multiple definition errors to functions declared in the same scope. it is very possible 
	// to have a free function and a member function with the same signature, and that is TOTALLY FINE
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> same_depth_functions =
		std::equal_range(matching_functions.begin(), matching_functions.end(), N, depth_pred());

	if (same_depth_functions.first != same_depth_functions.second) {
		std::cerr << "error (" << N->position << "): " << " multiple definitions found! listing previous definitions:\n";
		for (sooty::parseme_container::const_reverse_iterator f(same_depth_functions.second), f_end(same_depth_functions.first); f != f_end; ++f) {
			std::cerr << "\t" << undecorated_function(N) << " at " << (*f)->position << std::endl;
		}
		++si.errors;
	}
}


void prism::semantic_analysis::analyse::function(semantic_info& si, sooty::parseme_ptr_ref N)
{
	ATMA_ASSERT(N->id == ID::function);
	sooty::parseme_ptr_ref parameter_list = marshall::function::parameter_list(N);
	sooty::parseme_ptr_ref return_type = marshall::function::return_type(N);
	sooty::parseme_ptr_ref init_list = marshall::function::init_list(N);
	sooty::parseme_ptr_ref body = marshall::function::body(N);
	
	// return-type
	type(si, return_type);
	// parameters
	std::for_each( parameter_list->children.begin(), parameter_list->children.end(), boost::bind(&parameter, boost::ref(si), _1) );
	// initialiser-list
	std::for_each( init_list->children.begin(), init_list->children.end(), boost::bind(analyse::init_element, boost::ref(si), _1) );
	
	// body!
	block(si, body);
	
	if ( (body->children.empty() || body->children.back()->id != ID::return_statement) && return_type->value.string == "void") {
		body->children.push_back( sooty::make_parseme(body, ID::return_statement, sooty::value_t()) );
	}
	
	
	
}

void prism::semantic_analysis::analyse::init_element(semantic_info& si, sooty::parseme_ptr_ref element)
{
	sooty::parseme_ptr lhs = marshall::binary_expression::lhs(element);
	sooty::parseme_ptr rhs = marshall::binary_expression::rhs(element);
	bool lhs_is_ref = resolve::type_of(lhs)->id == ID::reference_type;
	
	if (lhs_is_ref) {
		//sooty::parseme_ptr sr = sooty::make_parseme(element, ID::intrinsic_set_reference);
		//sr->children.push_back( lhs );
		//sr->children.push_back( rhs );
		//llvm::pre_access_expression( sr );
	}
	else {
		sooty::parseme_ptr fc = synthesize::member_function_call(element, "__constructor__", lhs, rhs);
		analyse::member_function_call(si, fc);
		lhs->parent = element;
		rhs->parent = element;
		//prism::inlining::results R = prism::inlining::inline_function_call(fc);
		//std::for_each(R.statements_to_insert.begin(), R.statements_to_insert.end(), sooty::set_parent_to(elem->parent.lock()));
		//std::for_each(R.statements_to_insert.begin(), R.statements_to_insert.end(), llvm::statement);
	}
}

bool prism::inspect::is_intrinsic_function(sooty::parseme_ptr_ref N)
{
	return N->value.integer == 1;
}