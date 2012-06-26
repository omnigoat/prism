#include <prism/frontend/syntactical_analysis/parser.hpp>
//=====================================================================
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <prism/common/symbol_ids.hpp>
//=====================================================================
#include <prism/frontend/lexical_analysis/lex.hpp>


using namespace prism;
using namespace sooty;

main_parser::main_parser( const sooty::lexeme_list& lexemes )
 : abstract_parser(lexemes, channels::main)
{
}

void main_parser::start_rule_impl()
{
	//match_function().act();
//	rule g = guard();
//	rule g2 = guard();
///	rule func = match_insert(ids::function_keyword, ids::function);
	
	//{
	//	rule type = match_insert(ids::int_keyword, ids::int_type);
	//	rule ident = match(ids::identifier, true);
	//	g[ g && func && type && ident ];
	//}
	//
	//g.act();
	
}

// how it looks:
//  - above-scope
//    - function (value: function-name)
//      - return-type
//      - parameter-list
//        - parameter (value: parameter-name)
//          - parameter-type
//          - default-value (NYI)
//      - body
// get the function's symbol ahead of time
/*
abstract_parser::rule main_parser::match_function()
{
	rule k = match_insert(ids::function_keyword, ids::function);
	
	return 
		k[
			match_type() &&
			match(ids::identifier, true) &&
			k
		];
}

abstract_parser::rule main_parser::match_type()
{
	return 
		guard()[
			   match(ids::identifier)
			|| match_insert(ids::int_keyword, ids::int_type)
			|| match_insert(ids::void_keyword, ids::void_type)
		];
}
*/

parseme_ptr main_parser::match_identifier()
{
	return match_insert_impl(ids::identifier, ids::identifier, current_lexeme_iter()->value, false);
}

parseme_ptr main_parser::match_parameter_list()
{
	parseme_ptr list_parseme = insert_impl(ids::parameter_list, true);
	
	// only match empty parameter list for now
	if (!match_impl(ids::lparen, false, false)) {
		undo_last_insert(); // parameter-list
		return parseme_ptr();
	}
	
	// we can have no parameters! fail gracefully!
	if (!match_parameter()) {
		// but there MUST be closing parenthesis
		if (!match_impl(ids::rparen, false, false)) {
			undo_last_insert(); // parameter-list
			return parseme_ptr();
		}
		surface();
		return list_parseme;
	}
	
	size_t removable = 1;
	while ( match_impl(ids::comma, false) && match_parameter() )
		removable += 2;
	
	if (!match_impl(ids::rparen, false, false)) {
		undo_last_insert(removable);
		return parseme_ptr();
	}
	
	surface();
	
	return list_parseme;
}

parseme_ptr main_parser::match_parameter()
{
	if (LA(1) != ids::identifier) return parseme_ptr();
	
	current_lexeme_iter().move(1);
	sooty::value_t name = current_lexeme_iter()->value;
	current_lexeme_iter().move(-1);
	
	parseme_ptr parameter_parseme = insert_impl(ids::parameter, name, true);
	
//	if (!match_type()) {
//		undo_last_insert();
//		return parseme_ptr();
//	}
	
	// skip over identifier
	current_lexeme_iter().move(1);
	
	surface();
	return parameter_parseme;
}

parseme_ptr main_parser::match_function_body()
{
	parseme_ptr body_parseme = match_insert_impl(ids::lcurly, ids::function_body, true);
	
	for (;;)
	{
		if (!match_statement())
			break;
			
		if (!match_impl(ids::semi, false)) {
			undo_last_insert();
			break;
		}
	}
	
	match_impl(ids::rcurly, false, false);
	surface();
	return body_parseme;
}

parseme_ptr main_parser::match_integral_literal()
{
	// haha, only integer integrals
	parseme_ptr integral_parseme = 
		match_insert_impl(ids::int_literal, ids::int_literal, current_lexeme_iter()->value, true);
	if (!integral_parseme) return parseme_ptr();
	insert_impl(ids::int_type);
	surface();
	return integral_parseme;
}

parseme_ptr main_parser::match_rvalue()
{
	parseme_ptr p = match_integral_literal();
	if (p) return p;
	p = match_function_call();
	if (p) return p;
	return match_identifier();
}

parseme_ptr main_parser::match_statement()
{
	// atm, only function calls supported
	parseme_ptr p = match_function_call();
	if (p) return p;
	p = match_return_statement();
	if (p) return p;
	p = match_variable_definition();
	if (p) return p;
	return match_expression();
}

parseme_ptr main_parser::match_frule()
{
	parseme_ptr frule_node = insert_impl(ids::expression, value_t(ids::expr::definition), true);
	if (!match_rvalue())
	{
		// '(' expression ')' is viable
		if (!(match_impl(ids::lparen, false)
			&& match_expression()
			&& match_impl(ids::rparen, false)))
		{
			undo_last_insert(); // frule
			return parseme_ptr();
		}
	}

	for (;;)
	{
		if (match_impl(ids::star_sign, false))
			frule_node->value.integer = ids::expr::multiplication;
		else if (match_impl(ids::fwd_slash, false))
			frule_node->value.integer = ids::expr::division;
		else
			break;

		if (!match_frule()) {
			undo_last_insert(2); // frule, rvalue
			return parseme_ptr();
		}
	}
/*
	if (frule_node->value.integer == ids::expr::definition)
	{
		parseme_ptr to_keep = frule_node->children.back();
		undo_last_insert(2);
		insert(to_keep->id, to_keep->value);
		return frule_node;
	}
*/
	
	surface();
	return frule_node;
}

parseme_ptr main_parser::match_expression()
{
	parseme_ptr expr_parseme = insert_impl(ids::expression, value_t(ids::expr::definition), true);
	
	if (!match_frule()) {
		undo_last_insert(); // expression
		return parseme_ptr();
	}
	
	for (;;)
	{
		if (match_impl(ids::plus_sign, false))
			expr_parseme->value.integer = ids::expr::addition;
		else if (match_impl(ids::minus_sign, false))
			expr_parseme->value.integer = ids::expr::subtraction;
		else
			break;
		
		// tail recursion
		if (!match_expression()) {
			undo_last_insert(2); // expression, frule
			return parseme_ptr();
		}
		
		//parseme_ptr = current_lexeme_iter()->parent->children.
	}
	
	// no point keeping definition expressions around
	/*
	if (expr_parseme->value.integer == ids::expr::definition)
	{
		parseme_ptr to_keep = expr_parseme->children.back();
		undo_last_insert(2);
		insert(to_keep->id, to_keep->value);
		return expr_parseme;
	}
	*/
	
	surface();
	return expr_parseme;
}

parseme_ptr main_parser::match_function_call()
{
	
	
	// bare minimum lookahead before thinking it's a function call
	if (LA(0) != ids::identifier || LA(1) != ids::lparen) return parseme_ptr();
	
	// I guess??
	parseme_ptr function_call_parseme = insert_impl(ids::function_call, current_lexeme_iter()->value, true);
	// now ignore symbol
	match_impl(ids::identifier, false);
	// don't do anything with ()
	match_impl(ids::lparen, false);
	
	// match arguments
	if (match_rvalue())
		while (match_impl(ids::comma, false) && (match_rvalue() || match_function_call()) )
			;
	
	
	match_impl(ids::rparen, false);
	//
	surface();
	
	return function_call_parseme;
}

parseme_ptr main_parser::match_return_statement()
{
	if (parseme_ptr p = match_insert_impl(ids::return_keyword, ids::return_statement, true)) {
		match_statement();
		surface();
		return p;
	}
	return parseme_ptr();
}

static parseme_ptr SEQ = parseme::create(9000);




parseme_ptr main_parser::match_variable_definition()
{
	/*
	rule g = guard() [
			insert(ids::variable) [
				match_insert(ids::type, ids::type),
				match(ids::identifier, true)
			]
		];
	*/	
	return parseme_ptr();
}
















using namespace sooty;

std::string convert_parseme_to_string(const parseme_ptr& p)
{
	const size_t& id = p->id;
	const value_t& value = p->value;
	
	switch (id)
	{
		// parser ids
		case ids::int_type: return "int";
		case ids::expression:
			if (value.integer < ids::expr::multiplication)
				return "expression: " + boost::lexical_cast<std::string>(value.integer);
			else
				return "frule: " + boost::lexical_cast<std::string>(value.integer);
				
		case ids::identifier: return "identifier: " + value.string;
		case ids::function: return "function: " + value.string;
		case ids::parameter_list: return "parameter-list";
		case ids::return_statement: return "return-statement";
		case ids::int_literal: return "int-literal: " + boost::lexical_cast<std::string>(value.integer);
		case ids::function_body: return "function-body";
		case ids::function_call: return "function-call: " + value.string;
		case ids::parameter: return "parameter: " + value.string;
		case ids::variable: return "variable-definition";
	}
	
	return std::string();
}





//=====================================================================
bool main_parser::is_rvalue(size_t id) const
{
	switch (id)
	{
		case ids::function_call:
		case ids::identifier:
		case ids::int_literal:
			return true;
	}
	
	return false;
}



















void main_parser::print_tree() const
{
	print_tree_impl(get_root(), 0);
}

void main_parser::print_tree_impl(const parseme_ptr& p, int scope) const
{
	for (parseme_container::const_iterator i = p->children.begin(); i != p->children.end(); ++i)
	{
		for (int j = 0; j < scope; ++j)
		std::cout << " ";
		std::cout << convert_parseme_to_string(*i) << std::endl;
		print_tree_impl(*i, scope + 1);
	}
}
















//=====================================================================
//
//
//    Printing Trees!
//
//
//=====================================================================
/*
std::string convert_parseme_to_string(const parseme_ptr& p)
{
	const size_t& id = p->id;
	const value_t& value = p->value;

	switch (id)
	{
		// parser ids
	case ids::int_type: return "int";
	case ids::expression:
		if (value.integer < ids::expr::multiplication)
			return "expression: " + boost::lexical_cast<std::string>(value.integer);
		else
			return "frule: " + boost::lexical_cast<std::string>(value.integer);

	case ids::identifier: return "identifier: " + value.string;
	case ids::function: return "function: " + value.string;
	case ids::parameter_list: return "parameter-list";
	case ids::return_statement: return "return-statement";
	case ids::int_literal: return "int-literal: " + boost::lexical_cast<std::string>(value.integer);
	case ids::function_body: return "function-body";
	case ids::function_call: return "function-call: " + value.string;
	case ids::parameter: return "parameter: " + value.string;
	case ids::variable: return "variable-definition";
	}

	return std::string();
}


*/

static void print_tree_impl(const parseme_ptr& p, int scope)
{
	for (parseme_container::const_iterator i = p->children.begin(); i != p->children.end(); ++i)
	{
		for (int j = 0; j < scope; ++j)
			std::cout << " ";
		std::cout << convert_parseme_to_string(*i) << std::endl;
		print_tree_impl(*i, scope + 1);
	}
}

void prism::print_tree(const parseme_ptr& P)
{
	print_tree_impl(P, 0);
}


