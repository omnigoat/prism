#include <prism/frontend/syntactical_analysis/parse.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <atma/time.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parser.hpp>
//=====================================================================
#include <prism/frontend/lexical_analysis/lex.hpp>
//=====================================================================
using namespace sooty;
using namespace prism;
//=====================================================================

//=====================================================================
// pre-declared parsers
//=====================================================================
namespace 
{
parser
	indenter,
	dedenter,
	ending_semi,
	
	start,
	
	function_declaration,
	function,
	function_parameter_list,
	parameter,
	init_list,
	block,
	
	identifier,
	type_identifier,
	typeof,
	
	type,
	reference_type,
	postfix_type,
	postfix_type_rest,
	primary_type,
	
	pointer,
	pointer_helper,
	new_,
	delete_statement,
	
	function_call,
	argument_list,
	
	arrow,
	variable_definition,

	type_definition,
	array_,
	array_helper,
	
	statement,
	expression_statement,
	return_statement,
	variable_definition_statement,
	if_statement,
	loop_statement,
	assignment_statement,
	
	expression,
	equality_expression,
	additive_expression,
	multiplicative_expression,
	unary_expression,
	postfix_expression,
	postfix_expression_rest,
	primary_expression
	;
}


namespace {
	void expand_deref_member_operator(sooty::parseme_ptr_ref node)
	{
		ATMA_ASSERT(node->id == ID::deref_member_operator);
		sooty::rewrite_branch(node) [
			sooty::insert(ID::member_operator) [
				sooty::insert(ID::dereference) [
					sooty::insert(node->children[0])
				],
				sooty::insert(node->children[1])
			]
		];
	}
} // namespace 'anonymous'

namespace {
	//=====================================================================
	// a postfix-expression has a node N, followed by, in-order, the list
	// of ascending expressions that are parents of the former
	//=====================================================================
	void rewrite_postfix_expressions(sooty::parse_results_t& results)
	{
		ATMA_ASSERT(results.node->id == ID::postfix);
		sooty::parseme_ptr_ref N = results.node;
		
		ATMA_ASSERT(N->children.size() > 1);
		sooty::parseme_container::reverse_iterator i_begin = N->children.rbegin();
		sooty::parseme_container::reverse_iterator i_end = N->children.rend() - 1;
		for (sooty::parseme_container::reverse_iterator i = i_begin; i != i_end; ++i)
		{
			sooty::parseme_container::reverse_iterator pi = i;
			++pi;
			(*i)->children.insert( (*i)->children.begin(), (*pi) );
			(*pi)->parent = *i;
			if ((*i)->id == ID::deref_member_operator) {
				expand_deref_member_operator(*i);
			}
		}
		
		sooty::parseme_ptr to_use_now = N->children.back();
		to_use_now->parent = N->parent.lock();
		
		sooty::parseme_ptr parent = to_use_now->parent.lock();
		if (parent) {
			sooty::parseme_container::iterator i = std::find(parent->children.begin(), parent->children.end(), N);
			ATMA_ASSERT(i != parent->children.end());
			*i = to_use_now;
		}
		
		N = to_use_now;
	}
} // namespace 'anonymous'

namespace {
	void appropriate_variable_definition(sooty::parse_results_t& results)
	{
		ATMA_ASSERT(results.node->id == ID::variable_definition);
		sooty::parseme_ptr_ref N = results.node;
		
		/*
			// for regular types, we do regular things! :D
			>>
			((
				(primary_type).on_success(assign(vd_type))
				
				// hey hey hey, the default value is a constructor! :)
				>> !(match(lexid::lparen, false) >> 
					insert(ID::member_function_call) [
						insert(ID::identifier, "__constructor__"),
						insert(ID::argument_list) [
							insert(vd_iden) 
							 >> !expression >> *(match(lexid::comma, false) >> expression)
						],
						insert(vd_type)
					]
					>> match(lexid::rparen, false))
			)
			|
			// for reference types, we need to some special logic
			(
				reference_type
				>>
				!(match(lexid::lparen, false)
				>> insert(ID::intrinsic_set_reference) [
						insert(vd_iden),
						guard(expression)
					]
				>> match(lexid::rparen, false))
			))
			*/
	}
}


//void timer(sooty::parse_results_t&) {
static atma::time_t previous = atma::time();

SOOTY_FUNCTOR(reset_time, 0, 0) {
	previous = atma::time();
}


sooty::parseme_ptr P;
std::stack<sooty::parseme_ptr> equality_expression_stack;
std::stack<sooty::parseme_ptr> additive_expression_stack;
std::stack<sooty::parseme_ptr> multiplicative_expression_stack;

SOOTY_FUNCTOR(stack_is_empty, 1, 0) {
	ATMA_ASSERT(arg1.empty());
}

sooty::parseme_ptr vd_type;
sooty::parseme_ptr vd_iden;
SOOTY_FUNCTOR(assign, 1, 0) {
	arg1 = R.node;
}

void initialise_parsers()
{
	indenter =
		match(lexid::indent, false); //.on_failure(bad_dent);

	dedenter =
		match(lexid::dedent, false); //.on_failure(bad_dent);
	
	identifier =
		match_insert(lexid::identifier, parsid::identifier);
	
	typeof =
		match(lexid::typeof_keyword),
		match(lexid::lparen),
		insert(parsid::typeof) [
			expression
		],
		match(lexid::rparen)
		;
	
	block = 
		insert(ID::block) [ guard(indenter >> +statement >> dedenter) ];
	
	type
		= reference_type
		| postfix_type
	    | primary_type
	    ;
	
	reference_type
		= match_insert(lexid::ref_keyword, ID::reference_type) [ postfix_type | primary_type ]
		;
	
	postfix_type =
		insert(ID::postfix) [ primary_type >> +(postfix_type_rest) ].on_success(rewrite_postfix_expressions)
		;
	
	postfix_type_rest =
		  match_insert(lexid::star, ID::pointer_type)
		| match(lexid::lsquare, false) >> match(lexid::rsquare, false) >> insert(ID::dynamic_array_type) 
		| match_insert(lexid::lsquare, ID::array_type) [ expression >> match(lexid::rsquare, false) ]
		;
	
	primary_type
		= /*match_insert(lexid::int_keyword, ID::type_identifier) [
			insert(ID::argument_list) [
				(match(lexid::ltri, false), match_insert(lexid::int_literal, ID::bitwidth), match(lexid::rtri, false))
				|
				insert(ID::bitwidth, 32)
			]
		]
		| */type_identifier
		| match_insert(lexid::ellipses, ID::type_identifier, "varags") [ insert(ID::argument_list) ]
		| typeof
		;
	
	type_identifier
		=  match_insert(lexid::identifier, ID::type_identifier) [
			insert(ID::argument_list) [
				(
					match(lexid::ltri, false) >>
					primary_expression >>
					match(lexid::rtri, false)
				)
				%
				match(lexid::comma, false)
			]
		]
		;
	
	parameter =
		insert(ID::parameter) [
			//((identifier >> match(lexid::as_keyword, false)) | insert(ID::identifier, "undefined")) >> type
			(
				(identifier >> match(lexid::as_keyword, false))
				|
				(insert(ID::identifier, "#unused"))
			)
			>> type
		]
		;
	
	function_parameter_list =
		match_insert(lexid::lparen, parsid::parameter_list)
			[
				parameter % match(lexid::comma, false)
			]
		>> match(lexid::rparen, false)
		;
	
	init_list
		= match_insert(lexid::colon, parsid::init_list) [
			insert(parsid::init_element) [
				insert(ID::member_operator) [
					insert(ID::identifier, "this"),
					identifier
				],
				match(lexid::lparen, false),
				!expression,
				match(lexid::rparen, false)
			]
			% match(lexid::comma, false)
		]
		|
		insert(parsid::init_list)
		;
	
	argument_list =
		match_insert(lexid::lparen, ID::argument_list) [
			  match(lexid::rparen, false)
			| (expression % match(lexid::comma, false)) >> match(lexid::rparen, false)
		]
		;
	
	function_call = 
		insert(ID::function_call) [
			identifier,
			(argument_list | 
				match(lexid::exclamation_mark, false),
				insert(ID::argument_list) [ expression % match(lexid::comma, false) ]
			)
		]
		;
	
	arrow = match(lexid::arrow, false);


	expression = 
		//equality_expression
		//;
	
	//=====================================================================
	// equality-expression is: ==, !=, <, >, <=, >=
	//=====================================================================
	//equality_expression =
		// first, see if we can even have an additive expression
		sooty::store(equality_expression_stack, additive_expression)
		
		>>
		
		(
			// equality
			(match(lexid::equality, false) >>
				insert(ID::function_call) [
					insert(ID::identifier, "__equality__") >>
					insert(ID::argument_list) [
						retrieve(equality_expression_stack) >> additive_expression
					]
				]
			)
			|
			// less-than
			(match(lexid::ltri, false) >>
				insert(ID::function_call) [
					insert(ID::identifier, "__less_than__") >>
					insert(ID::argument_list) [
						retrieve(equality_expression_stack) >> additive_expression
					]
				]
			)
			|
			// additive_expression
			retrieve(equality_expression_stack)
		)
		;

	//=====================================================================
	// additive_expression is +, -
	//=====================================================================
	additive_expression =
		// for significant speed, first determine if we even have a multiplicative_expression
		sooty::store(additive_expression_stack, multiplicative_expression)
		>>
		// now begin to match what comes after
		(
			// addition
			+(match(lexid::plus, false) >>
				insert(ID::function_call) [
					insert(ID::identifier, "__addition__") >>
					insert(ID::argument_list) [
						retrieve(additive_expression_stack) >> additive_expression
					]
				]
			)
			|
			// subtraction
			+(match(lexid::dash, false) >>
				insert(ID::function_call) [
					insert(ID::identifier, "__subtraction__") >>
					insert(ID::argument_list) [
						retrieve(additive_expression_stack) >> additive_expression
					]
				]
			)
			|
			// multiplicative_expression
			retrieve(additive_expression_stack)
		)
		;
	
	
	//=====================================================================
	// multiplicative_expression is *, /
	//=====================================================================
	multiplicative_expression =
		// for significant speed, first determine if we even have a unary_expression
		sooty::store(multiplicative_expression_stack, unary_expression)
		>>
		// now begin to match what comes after
		(
			// multiplication
			+(match(lexid::star, false) >>
				insert(ID::function_call) [
					insert(ID::identifier, "__multiplication__") >>
					insert(ID::argument_list) [
						retrieve(multiplicative_expression_stack) >> multiplicative_expression
					]
				]
			)
			|
			// division
			+(match(lexid::fwdslash, false) >>
				insert(ID::function_call) [
					insert(ID::identifier, "__division__") >>
					insert(ID::argument_list) [
						retrieve(multiplicative_expression_stack) >> multiplicative_expression
					]
				]
			)
			|
			// unary_expression
			retrieve(multiplicative_expression_stack)	
		)
		;
	
	unary_expression =
	      insert(ID::address_of) [ match(lexid::ampersand, false) >> unary_expression ]
		| insert(ID::dereference) [ match(lexid::star, false) >> unary_expression ]
		| postfix_expression
		;
	
	postfix_expression =
		  insert(ID::postfix) [ primary_expression >> +postfix_expression_rest ].on_success(rewrite_postfix_expressions)
		| primary_expression
		;
	
	postfix_expression_rest =
		  match_insert(lexid::period, ID::member_operator) [ primary_expression ]
		| match_insert(lexid::arrow, ID::deref_member_operator) [ primary_expression ]
		| match_insert(lexid::lsquare, ID::index_operator) [ expression >> match(lexid::rsquare, false) ]
		;
	
	primary_expression =
		  match_insert(lexid::int_literal, ID::int_literal)
		| match_insert(lexid::bool_literal, ID::bool_literal)
		| match_insert(lexid::real_literal, ID::real_literal)
		| match_insert(lexid::string_literal, ID::string_literal)
		| new_
		| function_call
		| identifier
		| ( match(lexid::lparen, false) >> expression >> match(lexid::rparen, false) )
		;
		
	
	variable_definition =
		match_insert(lexid::var_keyword, ID::variable_definition)
			[ identifier.on_success(assign(vd_iden))
			, match(lexid::as_keyword, false)
			, type
			,
				( argument_list
				|
					( insert(ID::argument_list) 
					, !(match(lexid::from_keyword, false)
					, expression))
				)
		].on_success(appropriate_variable_definition)
		
		;
	
	new_ =
		match_insert(lexid::new_keyword, ID::new_) [
			primary_type,
			insert(ID::argument_list) [
				!(match(lexid::lparen, false) >> (expression % match(lexid::comma, false)) >> match(lexid::rparen, false))
			]
		]
		;
	
	statement =
		  variable_definition_statement
		| assignment_statement
		| expression_statement
		| return_statement
		| if_statement
		| loop_statement
		| delete_statement
		;
	
	expression_statement =
		expression;
	
	return_statement =
		match_insert(lexid::return_keyword, ID::return_statement) [
			 !expression
		]
		;
	
	variable_definition_statement =
		variable_definition
		;
	
	if_statement =
		match_insert(lexid::if_keyword, ID::if_statement) [
			expression,
				block,
			
			(match(lexid::else_keyword, false),
				block)
			
			// the else expression is optional, but we want to make sure we insert a redundant block anyway
			| ( insert(ID::block, value_t()) )
			
		]
		;
	
	loop_statement =
		match_insert(lexid::loop_keyword, ID::loop_statement) [
			// the identifier is optional
			(
				identifier
				|
				insert(ID::not_present)
			)
			>>
			(
				// check for a "from... to..." control mechanism
				insert(ID::from_to_control) [
					// from: good
					(
						match_insert(lexid::from_keyword, ID::from_expression) [
							expression
						]
						>>
						(
							match_insert(lexid::to_keyword, ID::to_expression) [
								expression
							]
							|
							insert(ID::not_present)
						)
					)
					// from: bad
					|
					(
						insert(ID::not_present)
						>>
						match_insert(lexid::to_keyword, ID::to_expression) [
							expression
						]
					)
				]
				|
				insert(ID::in_control) [
					match_insert(lexid::in_keyword, ID::in_expression) [
						expression
					]
				]
				|
				insert(ID::not_present)
			)
			>>
			// the if statement is optional
			(
				match_insert(lexid::if_keyword, ID::if_statement) [
					expression
				]
				|
				insert(ID::not_present)
			)
			// the via expression is also optional lol
			>>
			(
				match_insert(lexid::via_keyword, ID::via_expression) [
					!expression
				]
				|
				insert(ID::not_present)
			)
			
			// and now finally the block itself
			>>
			block
		]
		;
	
	delete_statement =
		match_insert(lexid::delete_keyword, ID::delete_) [
			expression
		]
		//>> ending_semi
		;
	
	assignment_statement =
		insert(ID::assignment) [ unary_expression >> match(lexid::equals, false) >> expression ]
		//>> ending_semi
		;
		
	type_definition =
		match_insert(lexid::type_keyword, ID::type_definition, 1) [
			identifier >>
			
			// parameter-list
			insert(ID::parameter_list) [
				!(
					match(lexid::ltri, false),
					(parameter % match(lexid::comma, false)),
					match(lexid::rtri, false)
				)
			] >>
			
			// argument-list
			insert(ID::argument_list) >>
			
			indenter >> 
				insert(ID::member_definitions) [
					*( variable_definition_statement | function ) 
				] >>
			dedenter
		]
		;
	
	

	function = 
		insert(ID::function) [
			// external linkage or not
			(match_insert(lexid::external_keyword, ID::external) | insert(ID::internal)),
			// keyword
			match(lexid::function_keyword, false),
			// calling-convention
			(match_insert(lexid::cc_c_keyword, ID::cc_c) | insert(ID::cc_fast)),
			// name
			identifier,
			// parameters
			function_parameter_list,
			// we can have a init-list if there's no return type
			((arrow >> type >> insert(ID::init_list)) | (insert(ID::type_identifier, sooty::value_t("void")) [ insert(ID::argument_list)] >> init_list)),
			// blocks are optional - if they are not present, it is a function-declaration
			block
		];
	
	start =
		//store_(postfix_expressions, identifier) >> identifier >> retrieve_(postfix_expressions);
		
		*(type_definition | function)
		;

}

void prism::parse( lexeme_list::const_iterator begin, lexeme_list::const_iterator end, parseme_ptr& dest )
{
	initialise_parsers();
	start(begin, end, dest);
}