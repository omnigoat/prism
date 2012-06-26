
#if 0


//=====================================================================
//
//  basic win32 assembly generation. it's entirely stack based.
//
//=====================================================================
#include <prism/backend/generation.hpp>
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/decorate.hpp>
#include <prism/frontend/semantic_analysis/semantic_analysis.hpp>
//=====================================================================
// usings
//=====================================================================
using namespace prism;

//=====================================================================
// function declarations
//=====================================================================
static void start(sooty::const_parseme_ptr_ref);
static void function(sooty::const_parseme_ptr_ref);
static void return_statement(sooty::const_parseme_ptr_ref, size_t stack_size);
static void expression(sooty::const_parseme_ptr_ref);
static void function_call(sooty::const_parseme_ptr_ref node);
static void rvalue(sooty::const_parseme_ptr_ref);
static void statement(sooty::const_parseme_ptr_ref);
static void variable_definition_statement(sooty::const_parseme_ptr_ref);
static void assignment_statement(sooty::const_parseme_ptr_ref);

//=====================================================================
// local variables
//=====================================================================
std::ostream* output_stream_;
std::ostream& output_stream() { return *output_stream_; }




void prism::generate(std::ostream* the_output_stream, sooty::const_parseme_ptr_ref root)
{
	output_stream_ = the_output_stream;
	
	// includes
	output_stream()
		<< "%include 'inc/nasmx.inc'\n"
		<< "%include 'inc/win32/windows.inc'\n"
		<< "%include 'inc/win32/kernel32.inc'\n"
		<< "%include 'inc/win32/user32.inc'\n";

	// pre-known data
	output_stream() << "section .data\n";

	// entry
	output_stream()
		<< "section .text\n"
		<< "[global main]\n";
	
	start(root);
}


void start(sooty::const_parseme_ptr_ref node)
{
	for (sooty::parseme_container::iterator i = node->children.begin(); i != node->children.end(); ++i) {
		if ((*i)->id == ID::function) {
			function(*i);
		}
	}
}

struct anonymous_real_definition
{
	int rn;
	anonymous_real_definition() : rn() {}
	void operator ()(const sooty::parseme_ptr& P)
	{
		if (P->id == ID::real_literal) {
			P->value.integer = rn;
			output_stream() << ".real" << rn++ << " dd " << P->value.real << "\n";
		}
	}
};

void function(sooty::const_parseme_ptr_ref node)
{
	// convenience!
	sooty::parseme_ptr_ref sym = marshall::function::name(node);
	sooty::parseme_ptr_ref return_type = marshall::function::return_type(node);
	sooty::parseme_ptr_ref parameter_list = marshall::function::parameter_list(node);
	sooty::parseme_ptr_ref body = marshall::function::body(node);
	
	output_stream()
		<< decorate_function(node) + ":\n";

	// making space for parameters
	size_t argp = 8;
	for (sooty::parseme_container::iterator i = parameter_list->children.begin();
		i != parameter_list->children.end(); ++i)
	{
		output_stream()
			<< "." << marshall::variable_definition::name(*i)->value.string << ": equ " << argp << "\n";
		
		argp += 4;
	}
	
	// reserve space for anonymous reals
	sooty::depth_first_for_each(body, anonymous_real_definition());
	
	
	// making things for variables
	int stackv = 0;
	for (sooty::parseme_container::iterator i = body->children.begin(); i != body->children.end(); ++i)
	{
		sooty::const_parseme_ptr_ref P = *i;
		if (P->id == ID::variable_definition) {
			stackv -= 4;
			
			output_stream()
				<< "." << marshall::variable_definition::name(P)->value.string << ": equ " << stackv << "\n";
		}
	}
	
	output_stream()
		<< "push ebp\n"
		<< "mov ebp, esp\n"
		<< "sub esp, " << -stackv << "\n"
		;
	
	std::for_each(body->children.begin(), body->children.end(), variable_definition_statement);
	
	for (sooty::parseme_container::iterator i = body->children.begin(); i != body->children.end(); ++i)
	{
		if ( (*i)->id == ID::return_statement )
			return_statement(*i, argp);
		else
			statement(*i);
	}

}

void statement(sooty::const_parseme_ptr_ref N)
{
	if (N->id == ID::assignment_statement)
		assignment_statement(N);
}

void variable_definition_statement(sooty::const_parseme_ptr_ref N)
{
	if (N->id != ID::variable_definition) return;
	if (N->children.size() < 3) return;
	
	expression(N->children[2]);
	output_stream()
		<< "pop DWORD eax\n"
		<< "mov [ebp+." << marshall::identifier::name(N->children[1])->value.string << "], eax\n";		
}

void assignment_statement(sooty::const_parseme_ptr_ref N)
{
	expression(N->children[1]);
	output_stream()
		<< "pop DWORD eax\n"
		<< "mov [ebp+." << marshall::identifier::name(N->children[0])->value.string << "], eax\n";
}

void return_statement(sooty::const_parseme_ptr_ref rs, size_t stack_size)
{
	sooty::const_parseme_ptr_ref rvalue = rs->children.front();
	
	ATMA_ASSERT(rvalue->id == ID::expression);
	
	expression(rvalue);
	
	// we need a return value in eax, and expressions push their thing onto the stack
	output_stream()
		<< "pop eax\n"
		<< "mov esp, ebp\n"
		<< "pop ebp\n"
		<< "ret " << stack_size << std::endl;
}

void function_call(sooty::const_parseme_ptr_ref node)
{
	//sooty::const_parseme_ptr_ref name = marshall::function::name(node);
	sooty::const_parseme_ptr_ref argument_list = marshall::function_call::argument_list(node);
	
	// semantic analysis pass has made sure this is totally fine:
	sooty::parseme_ptr func_to_call = sooty::upwards_find_first_if(node, function_matches(node));
	ATMA_ASSERT(func_to_call);
	
	// parameters
	std::for_each(argument_list->children.rbegin(), argument_list->children.rend(), rvalue);
	
	output_stream()
		<< "call " << decorate_function(func_to_call) << "\n"
		<< "push DWORD eax\n"
		;
}

void rvalue(sooty::const_parseme_ptr_ref P)
{
	if (P->id == ID::int_literal)
		output_stream() << "push DWORD " << P->value.integer << "\n";
	if (P->id == ID::real_literal)
		output_stream() << "fld DWORD [.real" << P->value.integer << "]\n";
	else if (P->id == ID::identifier)
		output_stream() << "push DWORD " << "[ebp+." << P->value.string << "]\n";
	else if (P->id == ID::function_call) {
		function_call(P);
	}
}

/*
sooty::parseme_ptr underlying_type(sooty::const_parseme_ptr_ref P)
{
	switch (P->id) {
		case ID::real_literal:
		case ID::int_literal:
			return P->children.front()->id;
		
		case ID::expression:
			if (P->value.integer == ID::expression_id::definition)
				return underlying_type(P->children.front());
			else
			{
				// SERIOUSLY, a hack. reals are just a higher number than ints... lulz.
				size_t lhs_type = underlying_type(P->children[0]);
				size_t rhs_type = underlying_type(P->children[1]);
//				ATMA_ASSERT(lhs_type == rhs_type);
				return lhs_type;
			}
		
		case ID::identifier:
		{
			sooty::parseme_ptr N = sooty::upwards_find_first_if(P, identifier_matches(P));
			return type_of(N);
		}
	}
	
	return ID::undefined;
}
*/

void expression(sooty::const_parseme_ptr_ref P)
{
	const sooty::parseme& E = *P;
	
	if (E.value.integer == ID::expression_id::definition)
	{
		sooty::const_parseme_ptr_ref lhs = marshall::detail::get<0>(P);
		rvalue(lhs);
		return;
	}
	else
	{
		sooty::const_parseme_ptr_ref lhs = marshall::detail::get<0>(P);
		sooty::const_parseme_ptr_ref rhs = marshall::detail::get<1>(P);
		
		expression(lhs);
		expression(rhs);
		
		//ATMA_ASSERT(underlying_type(lhs)->id == underlying_type(rhs)->id);
		
		output_stream()
			<< "pop DWORD ebx\n"
			<< "pop DWORD eax\n";
		
		switch (E.value.integer) {
			case ID::expression_id::addition: output_stream() << "add eax, ebx\n"; break;
			case ID::expression_id::subtraction: output_stream() << "sub eax, ebx\n"; break;
			case ID::expression_id::multiplication: output_stream() << "imul ebx\n"; break;
			case ID::expression_id::division: output_stream() << "xor edx, edx\n" << "div ebx\n"; break;
		}
		
		output_stream()
			<< "push DWORD eax" << std::endl;
	}
}


#endif
