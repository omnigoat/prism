#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/decorate.hpp>
#include <prism/backend/analysis/inline.hpp>
//=====================================================================
using namespace prism;


void llvm::function_declaration(sooty::const_parseme_ptr_ref N)
{
	sooty::const_parseme_ptr_ref linkage = marshall::function::linkage(N);
	sooty::const_parseme_ptr_ref calling = marshall::function::calling(N);
	sooty::const_parseme_ptr_ref name = marshall::function::name(N);
	sooty::const_parseme_ptr_ref parameter_list = marshall::function::parameter_list(N);
	sooty::const_parseme_ptr_ref return_type = marshall::function::return_type(N);
	
	// for the moment, we'll only consider the C calling convention
	ATMA_ASSERT(calling->id == ID::cc_c);
	
	output_stream() << "declare " << logical_type_name(return_type) << " @" << name->value.string;
	
	// parameter-list
	output_stream() << "(";
	if (!parameter_list->children.empty())
	{
		sooty::parseme_container::const_iterator i = parameter_list->children.begin();
		for ( ; i != parameter_list->children.end(); ++i)
		{
			if (i != parameter_list->children.begin())
				output_stream() << ", ";
			
			if ( marshall::parameter::type(*i)->value.string == "varags" ) {
				output_stream() << "...";
			}
			else {
				parameter(*i);
			}
		}
	}
	output_stream() << ")";
	output_stream() << std::endl;
}



//=====================================================================
// call_should_be_inlined
//=====================================================================
namespace {
	struct call_should_be_inlined
	{
		bool operator ()(sooty::const_parseme_ptr_ref N) const
		{
			if (N->id != ID::function_call && N->id != ID::member_function_call)
				return false;
			
			//sooty::parseme_ptr function = marshall::function_call::semantics::function(N).parseme;
			
			sooty::parseme_ptr function 
				= (N->id == ID::function_call) ? resolve::function_of_function_call(N)
				: resolve::function_of_member_function_call(N)
				;
			
			// one == inline plz
			return function && function->value.integer == 1;
		}
	};
}


//=====================================================================
// inline_intrinsic_functions
//=====================================================================
namespace {
	void inline_intrinsic_functions(sooty::const_parseme_ptr_ref block)
	{
		// find all function-calls that should be inlined
		sooty::parseme_container calls_to_inline;
		sooty::depth_first_copy_if(std::back_inserter(calls_to_inline), block, call_should_be_inlined());

		// inline them back-to-front, so that we modify the correct memory addresses!
		std::for_each(calls_to_inline.rbegin(), calls_to_inline.rend(), prism::inlining::inline_function_call);
	}
}










void llvm::function(sooty::const_parseme_ptr_ref N)
{
	ATMA_ASSERT(N->id == ID::function);

	// skip over intrinsic functions
	if (N->value.integer == 1)
		return;

	sooty::const_parseme_ptr_ref linkage = marshall::function_declaration::linkage(N);
	sooty::const_parseme_ptr_ref calling = marshall::function_declaration::calling(N);
	sooty::const_parseme_ptr_ref name = marshall::function::name(N);
	sooty::const_parseme_ptr_ref parameter_list = marshall::function::parameter_list(N);
	sooty::const_parseme_ptr_ref return_type = marshall::function::return_type(N);
	sooty::const_parseme_ptr_ref init_list = marshall::function::init_list(N);
	
	if (linkage->id == ID::external) {
		function_declaration(N);
		return;
	}
	sooty::const_parseme_ptr_ref body = marshall::function::body(N);
	
	// return-type
	bool return_type_fits_in_register = inspect::can_fit_in_register(return_type);
	bool return_type_is_void = return_type->value.string == "void";
	bool place_return_variable_in_parameters = !return_type_fits_in_register && !return_type_is_void;
	
	if (place_return_variable_in_parameters) {
		output_stream() << "define void "; 
	}
	else {
		output_stream() << "define " << logical_type_name(return_type) << " ";
	}
	
	// name
	output_stream() << "@" << decorate_function(N);

	// parameter-list
	output_stream() << "(";
	if (place_return_variable_in_parameters)
	{
		output_stream() << storage_type_name(return_type) << " sret %retv";
		if (!parameter_list->children.empty())
			output_stream() << ", ";
	}

	if (!parameter_list->children.empty())
	{
		sooty::parseme_container::const_iterator i = parameter_list->children.begin();
		parameter(*i++);
		for ( ; i != parameter_list->children.end(); ++i)
		{
			output_stream() << ", ";
			parameter(*i);
		}
	}
	output_stream() << ")";

	// attributes!
	if (N->value.integer == 1) {
		output_stream() << " alwaysinline";
	}


	output_stream() << std::endl;

	output_stream() << "{\n";
	
	// transform initialiser list to a series of constructors
	if (!init_list->children.empty()) {
		output_stream() << "initialiser_list:" << std::endl;
		llvm::add_tab();
		std::for_each( init_list->children.begin(), init_list->children.end(), llvm::initialiser_element );
		output_stream() << "br label %" << body->value.string << std::endl;
		llvm::sub_tab();
	}
	
	
	// body
	block(body, "entry");
	
	
	output_stream() << "}\n" << std::endl;
}


void prism::llvm::initialiser_element(sooty::const_parseme_ptr_ref elem)
{
	sooty::parseme_ptr lhs = marshall::binary_expression::lhs(elem);
	sooty::parseme_ptr rhs = marshall::binary_expression::rhs(elem);
	bool lhs_is_ref = resolve::type_of(lhs)->id == ID::reference_type;
	
	if (lhs_is_ref) {
		sooty::parseme_ptr sr = sooty::make_parseme(elem, ID::intrinsic_set_reference);
		sr->children.push_back( lhs );
		sr->children.push_back( rhs );
		llvm::pre_access_expression( sr );
		return;
	}
	else {
		sooty::parseme_ptr fc = synthesize::member_function_call(elem, "__constructor__", lhs, rhs);
		lhs->parent = elem;
		rhs->parent = elem;
		sooty::parseme_ptr function = resolve::function_of_member_function_call(fc);
		ATMA_ASSERT(function);
		if (function->value.integer == 1) {
			prism::inlining::results R = prism::inlining::build_inline_context(fc);
			std::for_each(R.statements_to_insert.begin(), R.statements_to_insert.end(), sooty::set_parent_to(elem->parent.lock()));
			std::for_each(R.statements_to_insert.begin(), R.statements_to_insert.end(), llvm::statement);
		}
		else {
			llvm::statement(fc);
		}
	}
}
