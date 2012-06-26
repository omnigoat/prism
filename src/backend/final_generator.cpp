#include <prism/backend/final_generator.hpp>
//=====================================================================
#include <atma/assert.hpp>
//=====================================================================
#include <prism/common/symbol_ids.hpp>
#include <prism/common/decorate.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
//=====================================================================
using namespace atma;
using namespace sooty;
using namespace prism;
using namespace prism::generator;
//=====================================================================

size_t type_proxy::size() const
{
	if (id == ids::int_type)
		return 4;
	return 0;
}

size_t function_proxy::parameter_stack_size()
{
	size_t total = 0;
	
	for (parameter_container::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
	{
		total += i->size();
	}
	
	return total;
}

size_t function_proxy::return_type_stack_size()
{
	return return_type.size();
}





final_generator::final_generator(parseme_ptr_ref root)
	: root_(root)
{
}

void final_generator::go(std::ostream* the_output_stream)
{
	output_stream_ = the_output_stream;
	
	// includes
	output_stream()
		<< "%include 'inc/nasmx.inc'\n"
		<< "%include 'inc/win32/windows.inc'\n"
		<< "%include 'inc/win32/kernel32.inc'\n"
		<< "%include 'inc/win32/user32.inc'\n";

	// we really do need this
	output_stream() << "extern _ExitProcess@4\n";

	// pre-known data
	output_stream() << "section .data\n";

	//write_to_file(output_file, 
	//	"stdout_handle: db 0\n"
	//	"stdout_written: db 0\n");

	// entry
	output_stream()
		<< "section .text\n"
		<< "[global main]\n";
	
	start_rule(root_);
}

void final_generator::start_rule(parseme_ptr_ref node)
{
	for (parseme_container::iterator i = node->children.begin(); i != node->children.end(); ++i)
	{
		if ((*i)->id == ids::function)
		{
			function(*i);
		}
	}
}

void final_generator::function(parseme_ptr_ref node)
{
	// convenience!
	parseme_ptr_ref sym = marshall::function::name(node);
	parseme_ptr_ref return_type = marshall::function::return_type(node);
	parseme_ptr_ref parameter_list = marshall::function::parameter_list(node);
	parseme_ptr_ref body = marshall::function::body(node);
	
	output_stream()
		<< decorate_function(node) + ":\n";

	size_t argp = 8;
	for (parseme_container::iterator i = parameter_list->children.begin();
		i != parameter_list->children.end(); ++i)
	{
		output_stream()
			<< "." << marshall::parameter::name(*i)->value.string << ": equ " << argp << "\n";
		
		argp += 4;
	}

	output_stream()
		<< "push ebp\n"
		<< "mov ebp, esp\n";
	
	for (parseme_container::iterator i = body->children.begin(); i != body->children.end(); ++i)
	{
		if ( (*i)->id == ids::return_statement )
			return_statement(*i, argp);
		else if ( (*i)->id == ids::function_call )
			function_call(*i);
	}
	
	if (return_type->id == ids::void_type)
		output_stream()
			<< "mov esp, ebp\n"
			<< "pop ebp\n"
			<< "ret\n";
}

void final_generator::function_return_type(parseme_ptr_ref node, function_proxy& proxy)
{
	// set id
	proxy.return_type.id = node->id;
	
	// only copy value if required
	//if (node->id != ids::int_type)
	proxy.return_type.value = node->value;
}

void final_generator::function_symbol(parseme_ptr_ref node, function_proxy& proxy)
{
	proxy.name = node->value.string;
}

void final_generator::function_parameter_list(parseme_ptr_ref node, function_proxy& proxy)
{
	parseme_container::iterator i = node->children.begin();
	for (; i != node->children.end(); ++i)
	{
		proxy.parameters.push_back( type_proxy() );
		proxy.parameters.back().id = marshall::parameter::type(*i)->id;
		proxy.parameters.back().value = marshall::parameter::name(*i)->value;
		
		//proxy.parameters.back().id = (*(*i)->children.begin())->id;
		//proxy.parameters.back().value = (*((*i)->children.begin() + 1))->value;
	}
}

size_t final_generator::calculate_required_stack_space(parseme_ptr_ref node)
{
	parseme_ptr_ref return_type = *node->children.begin();
	
	// we don't need to allocate stack space for anything that can be handled
	// in a register
	if (return_type->id == ids::int_type)
		return 0;
	else
		return 0;
}


void final_generator::return_statement(parseme_ptr_ref rs, size_t stack_size)
{
	parseme& rvalue = *rs->children.front();
	
	if ( (*rs->children.begin())->id == ids::function_call )
	{
		// we don't need to do anything more, since we're returning immediately
		function_call(*rs->children.begin());
	}
	else if (rvalue.id == ids::int_literal)
	{
		output_stream()
			<< "mov eax, " << rvalue.value.integer << "\n";
	}
	else if (rvalue.id == ids::expression)
	{
		expression(rs->children.front());
	}
	else if (rvalue.id == ids::identifier)
	{
		output_stream()
			<< "mov eax, "
			<< "[ebp+"
			<< "." << rvalue.value.string
			<< "]\n";
	}
	
	output_stream()
		<< "mov esp, ebp\n"
		<< "pop ebp\n"
		<< "ret " << stack_size << std::endl;
}


void final_generator::function_call(parseme_ptr_ref node)
{
	// we must figure out which overloaded function we're calling
	//std::string decorated_function = type_environment_.lookup(node->value.string);
	
	parseme_ptr_ref name = marshall::function::name(node);
	
	// semantic analysis pass has made sure this is totally fine:
	//parseme_ptr func_to_call = scope::find_value(S, node, resolve::closeness::perfect).front();
	parseme_ptr func_to_call = scope::upwards_find_first_if(node, scope::function_matches_call(node));
	
	
	
	// parameters
	for (parseme_container::reverse_iterator i = node->children.rbegin(); i != node->children.rend(); ++i)
	{
		parseme_ptr_ref pnode = *i;
		
		if (pnode->id == ids::int_literal) {
			output_stream()
				<< "push " << pnode->value.integer << "\n";
		}
		else if (pnode->id == ids::function_call) {
			function_call(pnode);
			output_stream()
				<< "push eax\n";
		}
			
	}
	
	output_stream()
		<< "call "
		<< decorate_function(func_to_call)
		<< std::endl;
}

void final_generator::rvalue(parseme_ptr_ref P)
{
	if (P->id == ids::int_literal)
		output_stream() << "push DWORD " << P->value.integer << "\n";
	else if (P->id == ids::identifier)
		output_stream() << "push DWORD " << "[ebp+" << "." << P->value.string << "]\n";
	else if (P->id == ids::function_call) {
		function_call(P);
		output_stream() << "push DWORD eax\n";
	}
}

void final_generator::expression(parseme_ptr_ref P)
{
/*
	if (P->value.integer == 0)
		output_stream() << "; whoo definition" << std::endl;
	else if (P->value.integer < 3)
		output_stream() << "; whoo expression" << std::endl;
	else
		output_stream() << "; whoo frule" << std::endl;
*/
		
	// DEFINITION EXPRESSION
	if (P->value.integer == ids::expr::definition)
	{
		parseme_ptr_ref lhs = marshall::detail::get<0>(P);
		
		if (lhs->id == ids::identifier)
			output_stream() << "mov eax, " << "[ebp+" << "." << lhs->value.string << "]\n";
		else if (lhs->id == ids::int_literal)
			output_stream() << "mov eax, " << lhs->value.integer << "\n";
		else if (lhs->id == ids::function_call)
			function_call(lhs);
		else if (lhs->id == ids::expression)
			expression(lhs);
		else
			output_stream() << "; ZOMG ERROR" << std::endl;
		
		return;
	}
	// OTHER EXPRESSIONS ARE BINARY
	else
	{
		parseme_ptr_ref lhs = marshall::detail::get<0>(P);
		parseme_ptr_ref rhs = marshall::detail::get<1>(P);
		
		if (lhs->id == ids::expression) {
			expression(lhs);
			output_stream() << "push eax\n";
		}
		else
			rvalue(lhs);
		
		if (rhs->id == ids::expression) {
			expression(rhs);
			output_stream() << "push eax\n";
		}
		else
			rvalue(rhs);
	}
	
	output_stream()
		<< "pop DWORD ebx\n"
		<< "pop DWORD eax\n";
	
	if (P->value.integer == ids::expr::addition)
		output_stream() << "add eax, ebx\n";
	else if (P->value.integer == ids::expr::subtraction)
		output_stream() << "sub eax, ebx\n";
	else if (P->value.integer == ids::expr::multiplication)
		output_stream() << "mul ebx\n";
	else if (P->value.integer == ids::expr::division)
		output_stream() << "xor edx, edx\n" << "div ebx\n";
	else if (P->value.integer == ids::expr::definition)
		output_stream() << "; ERROR!!\n";
	
	output_stream()	<< std::flush;
}


size_t final_generator::calculate_parameter_type_id(parseme_ptr_ref n)
{
	if (n->id == ids::int_literal)
		return ids::int_type;
	else // if (n->id == ids::identifier)
		return ids::identifier;
}