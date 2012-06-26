//=====================================================================
//
//=====================================================================
#ifndef PRISM_LEX_HPP
#define PRISM_LEX_HPP
//=====================================================================
#include <sooty/frontend/lexical_analysis/channel.hpp>
#include <sooty/frontend/lexical_analysis/lexeme_list.hpp>
#include <sooty/frontend/lexical_analysis/input_iterator.hpp>
//=====================================================================
namespace prism {
//=====================================================================
	
	namespace lexid
	{
		const size_t whitespace = 1;
		const size_t dedent = 2;
		const size_t indent = 3;
		const size_t newline = 4;
		const size_t tab = 5;
		
		// keywords
		const size_t int_keyword = 10;
		const size_t void_keyword = 11;
		const size_t function_keyword = 12;
		const size_t return_keyword = 13;
		const size_t pass_keyword = 14;
		const size_t real_keyword = 15;
		const size_t typeof_keyword = 16;
		const size_t as_keyword = 17;
		const size_t new_keyword = 18;
		const size_t delete_keyword = 19;
		const size_t if_keyword = 20;
		const size_t bool_keyword = 21;
		const size_t else_keyword = 22;
		const size_t var_keyword = 23;
		const size_t loop_keyword = 24;
		const size_t type_keyword = 25;
		const size_t from_keyword = 26;
		const size_t to_keyword = 27;
		const size_t in_keyword = 28;
		const size_t via_keyword = 29;
		const size_t ref_keyword = 30;
		const size_t external_keyword = 31;
		const size_t cc_c_keyword = 32;
		const size_t char_keyword = 33;
		
		const size_t single_line_comment = 50;
		//const size_t single_line_comment = 50;
		
		const size_t identifier = 100;
		const size_t int_literal = 101;
		const size_t real_literal = 102;
		const size_t bool_literal = 103;
		const size_t string_literal = 104;
		
		const size_t lparen = 1001;
		const size_t rparen = 1002;
		const size_t lcurly = 1003;
		const size_t rcurly = 1004;
		const size_t comma = 1005;
		const size_t semi = 1006;
		const size_t colon = 1007;
		const size_t plus = 1008;
		const size_t dash = 1009;
		const size_t star = 1010;
		const size_t equals = 1011;
		const size_t fwdslash = 1012;
		const size_t ltri = 1013;
		const size_t rtri = 1014;
		const size_t ampersand = 1015;
		const size_t equality = 1016;
		const size_t period = 1017;
		const size_t lsquare = 1018;
		const size_t rsquare = 1019;
		const size_t hash = 1020;
		const size_t arrow = 1021;
		const size_t ellipses = 1022;
		const size_t exclamation_mark = 1023;
	}
	
	namespace channels
	{
		static const sooty::channel whitespace(1);
		static const sooty::channel main(2);
		static const sooty::channel comments(3);
		static const sooty::channel scoper(4);
		static const sooty::channel newline(5);
	}
	
	sooty::lexeme_list lex(const sooty::input_iterator&, const sooty::input_iterator&);

//=====================================================================
} // namespace prism
//=====================================================================
#endif
//=====================================================================
