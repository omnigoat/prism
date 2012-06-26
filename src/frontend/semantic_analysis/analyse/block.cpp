#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/backend/analysis/inline.hpp>

// a test!
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>

void prism::semantic_analysis::analyse::block(semantic_info& si, sooty::parseme_ptr_ref block)
{
	// rename us!
	//block->value.string = std::string("block_") + boost::lexical_cast<std::string>(static_cast<int>(block->position.stream));
	if (block->value.string.empty())
		block->value.string = std::string("block_") + boost::lexical_cast<std::string>(block->position.guid());
	
	//=====================================================================
	// find all function-calls of member-operators and change them into the
	// appropriate member-function-call variation
	//=====================================================================
	{
		using namespace sooty::cuil;
		using sooty::cuil::placeholders::_1;
		using sooty::cuil::placeholders::_2;
		using sooty::cuil::placeholders::_3;
		
		tree_pattern(block, no_pattern(),
			eq(ID::member_operator) [
				_1 = any(),
				eq(ID::function_call) [
					_2 = eq(ID::identifier),
					eq(ID::argument_list) [
						*(_3 += any())
					]
				]
			]
			.rewrite (
				mk(ID::member_function_call) [
					_2,
					mk(ID::argument_list) [
						_1,
						_3
					]
				]
			)
		);
	}
	
	// all statements within us
	std::for_each( block->children.begin(), block->children.end(), boost::bind(statement, boost::ref(si), _1) );
	
	// find the first return statement, and remove everything afterwards
	sooty::parseme_container::iterator return_statement_iter = 
		std::find_if(block->children.begin(), block->children.end(), sooty::id_matches(ID::return_statement));
	if (return_statement_iter != block->children.end()) {
		++return_statement_iter;
		block->children.erase( return_statement_iter, block->children.end() );
	}
	
	// call the destructors on all variable-definitions
	sooty::parseme_container vds;
	for (sooty::parseme_container::iterator i = block->children.begin(); i != block->children.end(); ++i) {
		if ( (*i)->id == ID::variable_definition )
			vds.push_back(*i);
	}
	
	for (sooty::parseme_container::reverse_iterator i = vds.rbegin(); i != vds.rend(); ++i)
	{
		sooty::const_parseme_ptr_ref vd_iden = marshall::variable_definition::name(*i);
		sooty::const_parseme_ptr_ref vd_type = resolve::type_of(marshall::variable_definition::type(*i), resolve::remove_refs());
		sooty::parseme_ptr fc = synthesize::member_function_call(block, "__destructor__", sooty::make_parseme(sooty::parseme_ptr(), ID::identifier, vd_iden->value.string));
		if ( resolve::function_of_function_call(fc) ) {
			analyse::member_function_call(si, fc);
		}
	}
}

