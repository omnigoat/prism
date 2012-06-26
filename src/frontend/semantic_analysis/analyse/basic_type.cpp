#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/semantic_analysis/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
#include <prism/frontend/semantic_analysis/reduce.hpp>
//=====================================================================

struct type_definition_levels
{
	bool operator ()(sooty::const_parseme_ptr_ref N) const {
		return N->id == prism::ID::root ||
		       N->id == prism::ID::block;
	}
};

static void rewrite_postfix_expressions(prism::semantic_analysis::semantic_info& si, sooty::parseme_ptr parent, sooty::parseme_ptr back)
{
	switch (back->id) {
		case prism::ID::pointer_type:
		case prism::ID::array_type:
		case prism::ID::dynamic_array_type:
		{
			// insert all other elements to a new post-fixer
			sooty::parseme_ptr new_postfixer = make_parseme(back, prism::ID::postfix, sooty::value_t());
			back->children.insert(back->children.begin(), new_postfixer);
			new_postfixer->children.assign(parent->children.begin(), parent->children.end() - 1);
			for (sooty::parseme_container::iterator i = new_postfixer->children.begin(); i != new_postfixer->children.end(); ++i) {
				(*i)->parent = new_postfixer;
			}
			
			sooty::parseme_ptr parents_parent ( parent->parent );
			sooty::parseme_container::iterator piter = std::find(parents_parent->children.begin(), parents_parent->children.end(), parent);
			
			back->parent = parents_parent;
			*piter = back;
			break;
		}
		
		default: {
			sooty::parseme_ptr parents_parent ( parent->parent );
			sooty::parseme_container::iterator piter = std::find(parents_parent->children.begin(), parents_parent->children.end(), parent);
			*piter = back;
			(*piter)->parent = parents_parent;
		}
	}
}

void prism::semantic_analysis::analyse::type(semantic_info& si, sooty::parseme_ptr_ref N)
{
	// lookup the type-definition
	switch (N->id)
	{
		case ID::reference_type:
		case ID::pointer_type:
			type(si, N->children[0]);
			break;
		
		case ID::postfix:
			rewrite_postfix_expressions(si, N, N->children.back());
			type(si, N);
			break;
		
		case ID::array_type:
			type(si, N->children[0]);
			type(si, N->children[1]);
			break;
		
		case ID::dynamic_array_type:
			type(si, N->children[0]);
			break;
		
		// int and real SHOULD fall-through
		case ID::int_type:
		case ID::real_type:
			if (N->value.integer == 0)
				N->value.integer = 32;
		case ID::bool_type:
		case ID::void_type:
		
		case ID::type_identifier:
		{
			analyse::type_identifier(si, N);
			break;
		}
		
		case ID::identifier:
		{
			ATMA_HALT("Bad!");
		}
		
		default:
			break;
	}
}

namespace {
	using namespace prism;
	
	struct type_definition_does_not_match
	{
		sooty::parseme_ptr type_identifier;
		type_definition_does_not_match(sooty::const_parseme_ptr_ref type_identifier)
			: type_identifier(type_identifier)
		{
		}
		
		bool operator ()(sooty::const_parseme_ptr_ref type_definition)
		{
			if ( marshall::type_definition::name(type_definition)->value.string !=
			  marshall::type_identifier::identifier(type_identifier)->value.string )
				return true;
			
			return false;
		}
	};
}

namespace {
	bool is_nontemplated_type(sooty::const_parseme_ptr_ref td)
	{
		return marshall::type_definition::parameters(td)->children.empty() && marshall::type_definition::arguments(td)->children.empty();
	}
	
	bool is_instantiated_type(sooty::const_parseme_ptr_ref td)
	{
		return marshall::type_definition::parameters(td)->children.empty();
	}
}

namespace {
	sooty::parseme_ptr create_instantiated_type(sooty::const_parseme_ptr_ref td, const sooty::parseme_container& arguments)
	{
		sooty::parseme_ptr cloned_td = sooty::clone_tree(td);
		cloned_td->parent = td->parent;
		
		sooty::parseme_ptr parameters = marshall::type_definition::parameters(cloned_td);
		sooty::parseme_ptr members = marshall::type_definition::members(cloned_td);
		
		prism::reduce::reduce_with_template_arguments(members->children, parameters->children, arguments);
		
		// remove all parameters
		marshall::type_definition::parameters(cloned_td)->children.clear();
		
		// add all arguments
		for (sooty::parseme_container::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
			marshall::type_definition::arguments(cloned_td)->children.push_back(*i);
			marshall::type_definition::arguments(cloned_td)->children.back()->parent = marshall::type_definition::arguments(cloned_td);
		}
		
		

		return cloned_td;
	}
}





namespace {
	sooty::parseme_ptr matching_instantiated_definition(const sooty::parseme_container& ti_arguments,
	  sooty::parseme_container::const_iterator definitions_begin, sooty::parseme_container::const_iterator definitions_end)
	{
		for (sooty::parseme_container::const_iterator current = definitions_begin; current != definitions_end; ++current)
		{
			sooty::const_parseme_ptr_ref type_definition = *current;
			const sooty::parseme_container& arguments = marshall::type_definition::arguments(type_definition)->children;
			if (arguments.size() != ti_arguments.size())
				continue;
			
			sooty::parseme_container::const_iterator tdai = arguments.begin();
			sooty::parseme_container::const_iterator tiai = ti_arguments.begin();
			for (; tdai != arguments.end(); ++tdai, ++tiai)
			{
				sooty::const_parseme_ptr_ref td_argument = *tdai;
				sooty::const_parseme_ptr_ref ti_argument = *tiai;
				
				if (!resolve::types_match(td_argument, ti_argument))
					break;
				if (td_argument->value != ti_argument->value)
					break;
			}
			
			if (tdai == arguments.end()) {
				return type_definition;
			}
		}
		
		return sooty::parseme_ptr();
	}
}








void prism::semantic_analysis::analyse::type_identifier(semantic_info& si, sooty::parseme_ptr_ref type_identifier)
{
	// first, analysis upon the arguments
	sooty::parseme_container& arguments = marshall::type_identifier::arguments(type_identifier)->children;
	std::for_each(arguments.begin(), arguments.end(), boost::bind(analyse::expression, boost::ref(si), _1));
	
	// first, find all type-definitions
	sooty::parseme_container definitions;
	sooty::upwards_bredth_first_copy_if(type_identifier, std::back_inserter(definitions), sooty::id_matches(ID::type_definition));
	
	// remove type-definitions that aren't the right name
	definitions.erase(
	    std::remove_if(definitions.begin(), definitions.end(), type_definition_does_not_match(type_identifier)),
	    definitions.end()
	);
	
	if (definitions.empty()) {
		std::cerr << error(type_identifier, atma::string_builder("\"")(type_identifier->value.string)("\" - couldn't find corresponding type-definition"));
		++si.errors;
		return;
	}

	
	// sort based upon depth
	std::sort(definitions.begin(), definitions.end(), prism::depth_pred());
	
	// get all definitions that are the same depth as the deepest
	std::pair<sooty::parseme_container::iterator, sooty::parseme_container::iterator> possible_definitions
	    = std::equal_range(definitions.begin(), definitions.end(), definitions.back(), depth_pred());
	definitions.erase(definitions.begin(), possible_definitions.first);
	

	// split up instantiated vs non-instantiated
	sooty::parseme_container::iterator instantiated_types_end =
	    std::partition(definitions.begin(), definitions.end(), is_instantiated_type);
	
	//bool should_be_false = is_instantiated_type(*instantiated_types_end);
	
	// go through all the non-instantiated types
	for (sooty::parseme_container::iterator current = instantiated_types_end; current != definitions.end(); ++current)
	{
		sooty::const_parseme_ptr_ref type_definition = *current;
		
		const sooty::parseme_container& parameters = marshall::type_definition::parameters(type_definition)->children;
		if (arguments.size() > parameters.size())
			continue;
		
		// manually compare parameters
		sooty::parseme_container::const_iterator ai = arguments.begin();
		sooty::parseme_container::const_iterator pi = parameters.begin();
		
		// if our type doesn't have any arguments, then we default to using the
		// remaining parameters
		bool use_remaining_default_parameters = arguments.empty();
		
		for ( ; pi != parameters.end(); ++pi, ++ai)
		{
			sooty::const_parseme_ptr_ref parameter = *pi;
			
			// no more arguments!
			if (ai == arguments.end()) {
				// if our parameters don't have default-values, we fail!
				if ( !marshall::parameter::has_defvalue(parameter) ) {
					break;
				}
				else {
					use_remaining_default_parameters = true;
					break;
				}
			}
			
			sooty::const_parseme_ptr_ref argument = *ai;
			if ( !resolve::type_can_convert_to(argument, parameter) )
				break;
		}
		
		// create instantiated type
		if (ai == arguments.end() && (pi == parameters.end() || use_remaining_default_parameters))
		{
			sooty::parseme_container all_arguments = arguments;
			for ( sooty::parseme_container::const_iterator ppi = pi; ppi != parameters.end(); ++ppi) {
				ATMA_ASSERT( marshall::parameter::has_defvalue(*ppi) );
				all_arguments.push_back( marshall::parameter::defvalue(*ppi) );
			}
			
			// we now have the arguments to check the instantiated types
			sooty::parseme_ptr existing_instantiated_type = matching_instantiated_definition(all_arguments,
				definitions.begin(), instantiated_types_end);
			if (existing_instantiated_type) {
				marshall::type_identifier::semantics::type_definition(type_identifier).parseme = existing_instantiated_type;
				return;
			}
			
			sooty::parseme_ptr ntd = create_instantiated_type(type_definition, all_arguments);
			sooty::parseme_ptr parent = type_definition->parent.lock();
			sooty::parseme_container::iterator otdi = sooty::position_of(type_definition);
			parent->children.insert(otdi, ntd);
			
			prism::semantic_analysis::analyse::type_definition_signature(si, ntd);
			prism::semantic_analysis::analyse::type_definition_interface(si, ntd);
			prism::semantic_analysis::analyse::type_definition(si, ntd);
			
			marshall::type_identifier::semantics::type_definition(type_identifier).parseme = ntd;
			return;
		}
	}
	
	// no non-instantiated types were present!
	// that means we only have instantiated types (includes regular types).
	// we can't have templated types without non-instantiated types, so there must only
	// be one type here, a regular type
	ATMA_ASSERT(definitions.size() == 1);
	marshall::type_identifier::semantics::type_definition(type_identifier).parseme = definitions.front();
}
