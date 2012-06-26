#include <prism/frontend/lexical_analysis/lex.hpp>
//=====================================================================
#include <iostream>
//=====================================================================
#include <boost/algorithm/string.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexer.hpp>
#include <sooty/frontend/lexical_analysis/lexemifier.hpp>
//=====================================================================
using namespace sooty;
using namespace prism;

//=====================================================================
// local variables
//=====================================================================
lexemifier_t lexemifier; 

void newline_action(lex_results_t& R)
{
	lexemifier.make(lexid::newline, channels::whitespace, false)(R);
	lexemifier.newliner()(R);
}

struct keyword_creator
{
	size_t id;
	keyword_creator(size_t id) : id(id) {}
	void operator()(sooty::lex_results_t& result) {
		if (result.full) {
			lexemifier.make(id, channels::main, true)(result);
		}
	}
};


lexer keywords
	= //string_("int")      [ keyword_creator(lexid::int_keyword) ] |
	  string_("function") [ keyword_creator(lexid::function_keyword) ]
	| string_("return")   [ keyword_creator(lexid::return_keyword) ]
	| string_("pass")     [ keyword_creator(lexid::pass_keyword) ]
	| string_("typeof")   [ keyword_creator(lexid::typeof_keyword) ]
	| string_("as")       [ keyword_creator(lexid::as_keyword) ]
	| string_("new")      [ keyword_creator(lexid::new_keyword) ]
	| string_("delete")   [ keyword_creator(lexid::delete_keyword) ]
	| string_("if")       [ keyword_creator(lexid::if_keyword) ]
	| string_("else")     [ keyword_creator(lexid::else_keyword) ]
	| string_("true")     [ keyword_creator(lexid::bool_literal) ]
	| string_("false")    [ keyword_creator(lexid::bool_literal) ]
	| string_("var")      [ keyword_creator(lexid::var_keyword) ]
	| string_("loop")     [ keyword_creator(lexid::loop_keyword) ]
	| string_("type")     [ keyword_creator(lexid::type_keyword) ]
	| string_("in")       [ keyword_creator(lexid::in_keyword) ]
	| string_("from")     [ keyword_creator(lexid::from_keyword) ]
	| string_("to")       [ keyword_creator(lexid::to_keyword) ]
	| string_("via")      [ keyword_creator(lexid::via_keyword) ]
	| string_("ref")      [ keyword_creator(lexid::ref_keyword) ]
	| string_("external") [ keyword_creator(lexid::external_keyword) ]
	| string_("cc_c")     [ keyword_creator(lexid::cc_c_keyword) ]
	;

void turn_into_keyword(sooty::lex_results_t& results)
{
	std::string iden = sooty::make_string(results.start, results.stop);
	
	sooty::lex_results_t R = keywords( sooty::input_iterator(iden.begin()), sooty::input_iterator(iden.end()) );
	if (!R.pass || !R.full) {
		lexemifier.make(lexid::identifier, channels::main)(results);
	}
}

namespace {
	void convert_special_characters(sooty::lex_results_t& results)
	{
		std::string old_value = sooty::make_string(results.start, results.stop);
		boost::algorithm::replace_all(old_value, "\\n", "\n");
		
		lexemifier.list.push_back( sooty::lexeme(lexid::string_literal, channels::main, lexemifier.row, lexemifier.column, lexemifier.pos, old_value) );
	}
}

lexer tab =
	char_('\t') [ lexemifier.make(lexid::tab, channels::whitespace, false) ]
	;

lexer newline =
	(string_("\r\n") | char_('\n'))[newline_action]
	;

lexer eof =
	eof_()[newline_action];
	;

lexer whitespace =
	+tab | +newline | +any_of(" \f")
	;

lexer identifier =
	   ((char_('_') | in_range('a', 'z') | in_range('A', 'Z'))
	>> *(char_('_') | in_range('a', 'z') | in_range('A', 'Z') | in_range('0', '9')))
		[ turn_into_keyword ]
	;
	
lexer int_literal =
	(!char_('-') >> +in_range('0', '9'))[ lexemifier.make<int>(lexid::int_literal, channels::main) ]
	;

lexer real_literal =
	(!char_('-') >> +in_range('0', '9') >> char_('.') >> +in_range('0', '9'))[ lexemifier.make<float>(lexid::real_literal, channels::main) ]
	;

lexer string_literal =
	(char_('"') >> (*(any() & ~char_('"'))) [ convert_special_characters ] >> char_('"')) 
	;

lexer literals
	= real_literal
	| int_literal
	| string_literal
	;

lexer single_line_comment =
	(char_('#') >> +(any() & ~newline))[ lexemifier.make(lexid::single_line_comment, channels::comments) ]
	;





lexer symbols
	= char_('(')[ lexemifier.make(lexid::lparen, channels::main, false) ]
	| char_(')')[ lexemifier.make(lexid::rparen, channels::main, false) ]
	| char_('{')[ lexemifier.make(lexid::lcurly, channels::main, false) ]
	| char_('}')[ lexemifier.make(lexid::rcurly, channels::main, false) ]
	| char_(',')[ lexemifier.make(lexid::comma, channels::main, false)  ]
	| char_(';')[ lexemifier.make(lexid::semi, channels::main, false)   ]
	| char_(':')[ lexemifier.make(lexid::colon, channels::main, false)  ]
	| char_('+')[ lexemifier.make(lexid::plus, channels::main, false)   ]
	| string_("->") [ lexemifier.make(lexid::arrow, channels::main, false) ]
	| char_('-')[ lexemifier.make(lexid::dash, channels::main, false)   ]
	| char_('*')[ lexemifier.make(lexid::star, channels::main, false)   ]
	| char_('/')[ lexemifier.make(lexid::fwdslash, channels::main, false)   ]
	| string_("==") [ lexemifier.make(lexid::equality, channels::main, false) ]
	| char_('=')[ lexemifier.make(lexid::equals, channels::main, false) ]
	| char_('<')[ lexemifier.make(lexid::ltri, channels::main, false) ]
	| char_('>')[ lexemifier.make(lexid::rtri, channels::main, false) ]
	| char_('&')[ lexemifier.make(lexid::ampersand, channels::main, false) ]
	| string_("...")[ lexemifier.make(lexid::ellipses, channels::main, false) ]
	| char_('.')[ lexemifier.make(lexid::period, channels::main, false) ]
	| char_('[')[ lexemifier.make(lexid::lsquare, channels::main, false) ]
	| char_(']')[ lexemifier.make(lexid::rsquare, channels::main, false) ]
	| char_('!')[ lexemifier.make(lexid::exclamation_mark, channels::main, false) ]
	;

lexer start =
	*(whitespace | single_line_comment | literals | identifier | symbols | eof)
	;

namespace {
	struct lexeme_id_eq_pred : std::unary_function<lexeme, bool> {
		size_t id;
		lexeme_id_eq_pred(size_t id) : id(id) {}
		bool operator ()(const lexeme& L) const {
			return L.id == id;
		}
	};
}

namespace {
	struct channel_equals {
		sooty::channel c;
		channel_equals(sooty::channel c) : c(c) {}
		bool operator ()(const lexeme& L) const {
			return L.channel == c;
		}
	};
}

// makes tabs and newlines behave nicely
void post_process(sooty::lexeme_list& list)
{
	sooty::lexeme_list::iterator iter = list.begin();
	
	int previous_indent_level = 0;
	while (iter != list.end())
	{
		// get all tabs at the start of a line
		sooty::lexeme_list::iterator tab_begin = iter;
		sooty::lexeme_list::iterator tab_end = 
			std::find_if(tab_begin, list.end(), std::not1(lexeme_id_eq_pred(lexid::tab)));
		
		int tab_count = std::distance(tab_begin, tab_end);
		iter = tab_end;
		
		sooty::lexeme_list::iterator content_begin =
			std::find_if(iter, list.end(), channel_equals(channels::main));
		sooty::lexeme_list::iterator next_newline =
			std::find_if(iter, list.end(), lexeme_id_eq_pred(lexid::newline));
		if (next_newline == list.end())
			break;
		bool empty_line = next_newline < content_begin;
		iter = next_newline + 1;
		
		// insert correct number of indents and dedents
		if ( !empty_line )
		{
			int distance = std::distance(content_begin, iter);
			if (tab_count > previous_indent_level) {
				for (int i = tab_count; i != previous_indent_level; --i)
					content_begin = list.insert(content_begin, lexeme(lexid::indent, channels::main, 0, 0, 0, sooty::value_t()));
			}
			else if (tab_count < previous_indent_level) {
				for (int i = tab_count; i != previous_indent_level; ++i)
					content_begin = list.insert(content_begin, lexeme(lexid::dedent, channels::main, 0, 0, 0, sooty::value_t()));
			}
			
			iter = content_begin;
			std::advance(iter, distance);
			
			previous_indent_level = tab_count;
		}
	}
	
	if (0 > previous_indent_level) {
		for (int i = 0; i != previous_indent_level; ++i)
			list.push_back(lexeme(lexid::indent, channels::main, 0, 0, 0, sooty::value_t()));
	}
	else if (0 < previous_indent_level) {
		for (int i = 0; i != previous_indent_level; ++i)
			list.push_back(lexeme(lexid::dedent, channels::main, 0, 0, 0, sooty::value_t()));
	}
}


sooty::lexeme_list prism::lex( const sooty::input_iterator& begin, const sooty::input_iterator& end )
{
	//std::cout << std::endl;
	lexemifier.list = lexeme_list();
	start(begin, end);
	post_process(lexemifier.list);
	return lexemifier.list;
}





