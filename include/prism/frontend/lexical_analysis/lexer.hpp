//=====================================================================
//
//  lexer
//  -------
//    Well, the lexer is what actually, y'know, _lexes_. 
//
//=====================================================================
#ifndef KALEIDOSCOPE_LEXER_HPP
#define KALEIDOSCOPE_LEXER_HPP
//=====================================================================
#include <sooty/frontend/lexical_analysis/abstract_lexer.hpp>
//=====================================================================
#include <prism/prism.hpp>
//=====================================================================
PRISM_BEGIN

//=====================================================================

	class main_lexer
	 : public sooty::abstract_lexer
	{
	public:
		main_lexer(std::istream& input)
		 : abstract_lexer(input)
		{
		}
		
		bool next_token_();
		
		// rules
		void whitespace();
		void identifier();
		void number();
		
		// this will create an identifier token unless it was a keyword
		void resolve_identifier_as_keyword(const std::string&);
	};

//=====================================================================

PRISM_CLOSE
//=====================================================================
#endif
//=====================================================================
