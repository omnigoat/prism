#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/decorate.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/backend/analysis/inline.hpp>
#include <prism/backend/generation.hpp>
#include <prism/prism.hpp>
//=====================================================================
using namespace prism;


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

namespace {
	void add_destructors(sooty::const_parseme_ptr_ref block)
	{
		sooty::parseme_container vds;
		for (sooty::parseme_container::iterator i = block->children.begin(); i != block->children.end(); ++i) {
			if ( (*i)->id == ID::variable_definition )
				vds.push_back(*i);
		}
		
		if (!vds.empty())
			llvm::output_stream(false) << "; destruction" << std::endl;
		
		for (sooty::parseme_container::reverse_iterator i = vds.rbegin(); i != vds.rend(); ++i)
		{
			sooty::const_parseme_ptr_ref vd_iden = marshall::variable_definition::name(*i);
			sooty::const_parseme_ptr_ref vd_type = resolve::type_of(marshall::variable_definition::type(*i), resolve::remove_refs());
			sooty::parseme_ptr fc = synthesize::member_function_call(block, "__destructor__", sooty::make_parseme(sooty::parseme_ptr(), ID::identifier, vd_iden->value.string));
			if ( resolve::function_of_function_call(fc) ) {
				// we don't have to worry about inlining, since inbuilt types don't have destructors. YET. LOL.
				llvm::member_function_call(fc);
			}
		}
	}
}

namespace {
	void implement_variable_initialisation(sooty::parseme_ptr_ref block)
	{
		for (sooty::parseme_container::iterator i = block->children.begin(); i != block->children.end(); )
		{
			sooty::parseme_ptr statement = *i;
			if (statement->id != ID::variable_definition) {
				++i;
				continue;
			}
			
			
			sooty::parseme_ptr type = resolve::type_of( marshall::variable_definition::type(statement) );
			sooty::const_parseme_ptr_ref name = marshall::variable_definition::name(statement);
			sooty::parseme_ptr arguments = marshall::variable_definition::arguments(statement);
			
			sooty::parseme_ptr cloned_arguments = sooty::clone_tree(arguments);
			
			if ( type->id != ID::reference_type && type->id != ID::pointer_type && type->id != ID::array_type )
			{
				sooty::parseme_ptr fc = sooty::make_parseme(block, ID::member_function_call);
				(sooty::immediate(fc)) ((
					sooty::insert(ID::identifier, "__constructor__"),
					sooty::insert(ID::argument_list) [
						sooty::insert(ID::identifier, name->value.string),
						sooty::insert(cloned_arguments->children.begin(), cloned_arguments->children.end())
					]
				));
				
				if ( prism::inlining::should_intrinsically_inline(fc) )
				{
					prism::inlining::results context = prism::inlining::build_inline_context(fc);
					prism::inlining::inline_void_function_call_at(block, i + 1, context);
					sooty::parseme_container::iterator ni = sooty::position_of(statement);
					ATMA_ASSERT(ni != block->children.end());
					i = ni + 1;
				}
				else {
					i = block->children.insert(i + 1, fc);
				}
			}
			else if ( type->id == ID::pointer_type )
			{
				if (arguments->children.size() > 0) {
					ATMA_ASSERT(arguments->children.size() == 1);
					sooty::parseme_ptr a = sooty::make_parseme(block, ID::assignment);
					(sooty::immediate(a)) ((
						sooty::insert(ID::identifier, name->value.string),
						sooty::insert(arguments->children.front())
					));
					
					i = block->children.insert(i + 1, a);
				}
				else if (arguments->children.empty()) {
					++i;
				}
			}
			else if ( type->id == ID::reference_type )
			{
				ATMA_ASSERT(arguments->children.size() == 1);
				sooty::parseme_ptr a = sooty::make_parseme(block, ID::intrinsic_set_reference);
				(sooty::immediate(a)) ((
					sooty::insert(ID::identifier, name->value.string),
					sooty::insert(arguments->children.front())
				));
				
				i = block->children.insert(i + 1, a);
			}
			else {
				++i;
			}
		}
	}
}


void llvm::block(sooty::const_parseme_ptr_ref block, const std::string& name)
{
	llvm::detail::tabs() = 0;
	output_stream() << block->value.string << ":\n"; //"block_" << block->position.guid() << ":\n";
	
	add_tab();
	
	inline_intrinsic_functions(block);
	
	if (!block->children.empty()) {
		sooty::parseme_ptr block_again = block;
		
		implement_variable_initialisation(block_again);
		
		sooty::parseme_container statements = block->children;
		sooty::parseme_container::reverse_iterator rs_position = std::find_if(statements.rbegin(), statements.rend(), sooty::id_matches(ID::return_statement));
		std::for_each(statements.begin(), --rs_position.base(), llvm::statement);
		add_destructors(block);
		std::for_each(--rs_position.base(), statements.end(), llvm::statement);
	}
	
	sub_tab();
}