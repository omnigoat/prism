#include <prism/backend/generation/llvm/llvm-main.hpp>
//=====================================================================
#include <boost/lexical_cast.hpp>
//=====================================================================
#include <atma/string_builder.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/decorate.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
#include <prism/frontend/semantic_analysis/resolve/type.hpp>
#include <prism/frontend/semantic_analysis/resolve/operator.hpp>
#include <prism/frontend/semantic_analysis/inspect.hpp>
#include <prism/backend/analysis/inline.hpp>
#include <prism/backend/generation.hpp>

//=====================================================================
//=====================================================================
using namespace prism;

struct matching_member {
	std::string st;
	matching_member(const std::string& st) : st(st) {}
	bool operator ()(sooty::const_parseme_ptr_ref A) const {
		return A->id == ID::variable_definition && marshall::variable_definition::name(A)->value.string == st;
	}
};

namespace {
	sooty::parseme_ptr member_index_concatenator(sooty::const_parseme_ptr_ref N, std::vector<std::string>& indices)
	{
		using namespace llvm;
		
		switch (N->id)
		{
			case ID::index_operator:
			{
				sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
				sooty::parseme_ptr r = member_index_concatenator(lhs, indices);

				sooty::parseme_ptr rhs = marshall::binary_expression::rhs(N);
				pre_access_expression(rhs);
				indices.push_back( rvalue_name(rhs) );
				return r;
			}
			
			case ID::member_operator:
			{
				sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
				sooty::parseme_ptr r = member_index_concatenator(lhs, indices);

				sooty::parseme_ptr rhs = marshall::binary_expression::rhs(N);
				sooty::parseme_ptr parent_type_definition = resolve::type_of(lhs);
				if (parent_type_definition->id == ID::reference_type)
					parent_type_definition = resolve::type_of(parent_type_definition->children.front());
				
				sooty::const_parseme_ptr_ref parent_type_members = marshall::type_definition::members(parent_type_definition);
				size_t distance =
					std::count_if(
						parent_type_members->children.begin(),
						std::find_if( parent_type_members->children.begin(), parent_type_members->children.end(), matching_member(marshall::identifier::name(rhs)->value.string)),
						sooty::id_matches(ID::variable_definition)
					);
				
				indices.push_back( boost::lexical_cast<std::string>(distance) );
				//if ( resolve::type_of(rhs)->id == ID::reference_type )
				//	indices.push_back("0");
				return r;
			}
			
			default:
				pre_mutate_expression(N);
				return N;
		}
	}
}

//=====================================================================
//
// this is big function!
// it's for "mutating expressions"
//=====================================================================
namespace {
	void dereference_expression(sooty::const_parseme_ptr_ref parent, sooty::const_parseme_ptr_ref N)
	{
		switch (N->id)
		{
			case ID::identifier:
			{
				sooty::const_parseme_ptr_ref VD = resolve::identifier_to_variable_definition(N);
				
				sooty::parseme_ptr VD_name = marshall::variable_definition::name(VD);
				sooty::parseme_ptr VD_type = marshall::variable_definition::type(VD);
				
				if ( !marshall::identifier::definition_is_parameter(N) && !resolve::parent_is_operator(N) )
					llvm::output_stream() << llvm::lvalue_name(parent)
						<< " = load " << llvm::storage_type_name(N) << " " << llvm::lvalue_name(N) << "\n";
			}
		}
	}
}

void llvm::pre_mutate_expression(sooty::const_parseme_ptr_ref N)
{
	switch (N->id)
	{
		case ID::int_literal:
		case ID::real_literal:
			break;
		
		case ID::identifier:
		{
			//N->value.integer = marshall::identifier::semantics::variable_definition(N).parseme->semantics["mt-count"].integer++;
			sooty::const_parseme_ptr_ref vd = prism::resolve::identifier_to_variable_definition(N); //marshall::identifier::semantics::variable_definition(N).parseme;
			++usage_count(vd);
			
			sooty::const_parseme_ptr_ref type = marshall::variable_definition::type(vd);
			break;
		}
		
		case ID::function_call:
		{
			function_call(N);
			break;
		}
		
		case ID::member_function_call:
		{
			llvm::member_function_call(N);
			break;
		}
		
		case ID::dereference:
		{
			sooty::parseme_ptr_ref child = N->children.front();
			dereference_expression(N, child);
			break;
		}
		
		case ID::un_reference:
		{
			pre_mutate_expression(N->children.front());
			if ( N->children.front()->id == ID::identifier && marshall::identifier::definition_is_parameter(N->children.front()) ) {
			}
			else {
				output_stream() << lvalue_name(N) << " = load " << storage_type_name(N->children.front()) << " " << lvalue_name(N->children.front()) << std::endl;
			}
			break;
		}
		
		case ID::member_operator:
		{
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			
			// we want to collapse many getelementptr instructions into a single instruction
			// with all the subsequent indices tacked on to the end. lhs is changed to the
			// underlying type.
			std::vector<std::string> gep_indices;
			lhs = member_index_concatenator(N, gep_indices);
			
			output_stream() << 
				lvalue_name(N) << " = getelementptr " << storage_type_name(lhs) << " " << lvalue_name(lhs) << ", i32 0, ";
			
			for (std::vector<std::string>::const_iterator i = gep_indices.begin(); i != gep_indices.end(); ++i) {
				output_stream(false) << (i != gep_indices.begin() ? ", " : "" ) << "i32 " << *i;
			}
			
			output_stream(false) << std::endl;
			break;
		}
		
		case ID::index_operator:
		{
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			std::vector<std::string> gep_indices;
			lhs = member_index_concatenator(N, gep_indices);

			sooty::parseme_ptr rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(rhs);
			
			
			// hmm...
			
			
			sooty::parseme_ptr type_of_lhs = resolve::type_of_pred(lhs)();
			if (type_of_lhs->id == ID::pointer_type)
			{
				// bitcast the pointer-type to a dynamically-sized array type
				std::string ptr_array_type = "[0 x " + logical_type_name( marshall::array_type::type(type_of_lhs) ) + "]*";
				output_stream()
					<< lvalue_name(N) << ".bc = bitcast " << storage_type_name(lhs) << " " << lvalue_name(lhs) << " to " << ptr_array_type << std::endl;
				
				//output_stream() <<
					//lvalue_name(N) << " = getelementptr " << ptr_array_type << " " << lvalue_name(N) << ".bc, i32 0, " << "i32 " << rvalue_name(rhs) << std::endl;
				output_stream() << 
					lvalue_name(N) << " = getelementptr " << ptr_array_type << " " << lvalue_name(N) << ".bc, i32 0, ";

				for (std::vector<std::string>::const_iterator i = gep_indices.begin(); i != gep_indices.end(); ++i) {
					output_stream(false) << (i != gep_indices.begin() ? ", " : "" ) << "i32 " << *i;
				}
				output_stream() << std::endl;
			}
			else // array type
			{
				output_stream() << 
					lvalue_name(N) << " = getelementptr " << storage_type_name(lhs) << " " << lvalue_name(lhs) << ", i32 0, ";

				for (std::vector<std::string>::const_iterator i = gep_indices.begin(); i != gep_indices.end(); ++i) {
					output_stream(false) << (i != gep_indices.begin() ? ", " : "" ) << "i32 " << *i;
				}
				output_stream() << std::endl;
			}
			break;
		}
		
		case ID::add:
		case ID::sub:
		case ID::mul:
		case ID::div:
		{
			ATMA_HALT("can't do that just yet");
			break;
		}
		
		
		default:
			ATMA_HALT("terrible~~~!");
	}
	
	
}



//=====================================================================
//
//
//
//=====================================================================


void llvm::pre_access_expression(sooty::const_parseme_ptr_ref N)
{
	switch (N->id)
	{
		case ID::int_literal:
		{
			break;
			output_stream()
				<< lvalue_name(N) << " = alloca " << logical_type_name(N) << std::endl;
			output_stream()
				<< "store " << logical_type_name(N) << " " << N->value.integer << ", " << storage_type_name(N) << " " << lvalue_name(N) << std::endl;
			break;
		}
		
		case ID::intrinsic_int_add:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			output_stream() << rvalue_name(N) << " = add " << logical_type_name(lhs) << " " << rvalue_name(lhs) << ", " << rvalue_name(rhs) << std::endl;
			break;
		}
		case ID::intrinsic_int_sub:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			output_stream() << rvalue_name(N) << " = sub " << logical_type_name(lhs) << " " << rvalue_name(lhs) << ", " << rvalue_name(rhs) << std::endl;
			break;
		}
		case ID::intrinsic_int_mul:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			output_stream() << rvalue_name(N) << " = mul " << logical_type_name(lhs) << " " << rvalue_name(lhs) << ", " << rvalue_name(rhs) << std::endl;
			break;
		}
		case ID::intrinsic_int_div:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			output_stream() << rvalue_name(N) << " = sdiv " << logical_type_name(lhs) << " " << rvalue_name(lhs) << ", " << rvalue_name(rhs) << std::endl;
			break;
		}
		case ID::intrinsic_int_eq:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			output_stream() << rvalue_name(N) << " = icmp eq " << logical_type_name(lhs) << " " << rvalue_name(lhs) << ", " << rvalue_name(rhs) << std::endl;
			break;
		}
		case ID::intrinsic_int_lt:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			output_stream() << rvalue_name(N) << " = icmp slt " << logical_type_name(lhs) << " " << rvalue_name(lhs) << ", " << rvalue_name(rhs) << std::endl;
			break;
		}
		
		case ID::intrinsic_set_reference:
		{
			sooty::const_parseme_ptr_ref lhs = marshall::binary_expression::lhs(N);
			sooty::const_parseme_ptr_ref rhs = marshall::binary_expression::rhs(N);
			sooty::parseme_ptr lhs_type = resolve::type_of(lhs);
			ATMA_ASSERT(lhs_type->id == ID::reference_type);
			pre_mutate_expression(lhs);
			pre_mutate_expression(rhs);
			output_stream() << "store " << storage_type_name(rhs) << " " << lvalue_name(rhs) << ", " << storage_type_name(lhs) << " " << lvalue_name(lhs) << std::endl;
			break;
		}
		
		case ID::identifier:
		{
			sooty::parseme_ptr parent = N->parent.lock();
			if (parent->id != ID::member_operator &&
				parent->id != ID::variable_definition &&
				!marshall::identifier::definition_is_parameter(N) &&
				resolve::type_of(N)->id != ID::reference_type)
			{
				sooty::const_parseme_ptr_ref VD = resolve::identifier_to_variable_definition(N);
				sooty::const_parseme_ptr_ref name = marshall::variable_definition::name(VD);
				
				output_stream() << rvalue_name(N) << " = load " << storage_type_name(N) << " %" << name->value.string << std::endl;
			}
			break;
		}
		
		case ID::function_call:
		{
			sooty::parseme_ptr function = resolve::function_of_function_call(N);
			ATMA_ASSERT(function);
			if (!function)
				return;
			
			// for function calls, accessing, not mutating, simply requires an addition to the mutating code
			pre_mutate_expression(N);
			
			// if the return-type was too large to fit into a register, we need to load it from the address
			// in memory
			sooty::parseme_ptr_ref return_type = marshall::function::return_type(function);
			if (!inspect::can_fit_in_register(return_type) && return_type->value.string != "void") {
				output_stream() << rvalue_name(N) << " = load " << storage_type_name(N) << " " << lvalue_name(N) << std::endl;
			}
			
			break;
		}
		
		case ID::member_function_call:
		{
			sooty::parseme_ptr function = resolve::function_of_member_function_call(N);
			ATMA_ASSERT(function);
			if (!function)
				return;
			
			// for function calls, accessing, not mutating, simply requires an addition to the mutating code
			pre_mutate_expression(N);
			
			// if the return-type was too large to fit into a register, we need to load it from the address
			// in memory
			sooty::parseme_ptr_ref return_type = marshall::function::return_type(function);
			if (!inspect::can_fit_in_register(return_type) && return_type->value.string != "void") {
				output_stream() << rvalue_name(N) << " = load " << storage_type_name(N) << " " << lvalue_name(N) << std::endl;
			}
			
			break;
		}
		
		case ID::member_operator:
		{
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			pre_access_expression(lhs);
			
			sooty::parseme_ptr rhs = marshall::binary_expression::rhs(N);
			
			// this is a composite type, unless it's a reference type
			sooty::parseme_ptr parent_type_definition = resolve::type_of_pred(lhs)();
			unreference(parent_type_definition);
			
			if (resolve::type_of_pred(lhs)()->id == ID::pointer_type) {
				output_stream()
					<< "; MO\n";
				output_stream()
					<< lvalue_name(N) << " = getelementptr " << storage_type_name(lhs) << " " << lvalue_name(lhs) << ", i32 0, " << "i32 " << rvalue_name(rhs) << std::endl;
			}
			
			sooty::const_parseme_ptr_ref parent_type_members = marshall::type_definition::members(parent_type_definition);
			size_t distance =
				std::count_if(
					parent_type_members->children.begin(),
					std::find_if( parent_type_members->children.begin(), parent_type_members->children.end(), matching_member(marshall::identifier::name(rhs)->value.string)),
					sooty::id_matches(ID::variable_definition)
				);
			
			output_stream() << 
				lvalue_name(N) << " = getelementptr " << storage_type_name(lhs) << " " << lvalue_name(lhs) << ", i32 0, " << "i32 " << distance << std::endl;
			
			if ( !resolve::parent_is_operator(N) )
				output_stream() <<
					rvalue_name(N) << " = load " << storage_type_name(N) << " " << lvalue_name(N) << std::endl;
			
			break;
			
		}
		
		case ID::index_operator:
		{
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			sooty::parseme_ptr rhs = marshall::binary_expression::rhs(N);
			pre_access_expression(lhs);
			pre_access_expression(rhs);
			
			sooty::parseme_ptr type_of_lhs = resolve::type_of_pred(lhs)();
			if (type_of_lhs->id == ID::pointer_type)
			{
				output_stream() << "; access-index-operator\n";
				// bitcast the pointer-type to a dynamically-sized array type
				std::string ptr_array_type = "[0 x " + logical_type_name(marshall::pointer_type::pointee_type(type_of_lhs)) + "]*";
				output_stream()
					<< lvalue_name(N) << ".bc = bitcast " << storage_type_name(lhs) << " " << lvalue_name(lhs) << " to " << ptr_array_type << std::endl;

				output_stream() <<
					lvalue_name(N) << " = getelementptr " << ptr_array_type << " " << lvalue_name(N) << ".bc, i32 0, " << "i32 " << rvalue_name(rhs) << std::endl;
			}
			else
			{
				output_stream() <<
					lvalue_name(N) << " = getelementptr " << storage_type_name(lhs) << " " << lvalue_name(lhs) << ", i32 0, " << "i32 " << rvalue_name(rhs) << std::endl;
			}
			
			if ( !resolve::parent_is_operator(N) )
				output_stream() <<
					//"steve!";
					rvalue_name(N) << " = load " << storage_type_name( resolve::type_of(N) ) << " " << lvalue_name(N) << std::endl;
			break;
		}
		
		case ID::un_reference:
		{
			sooty::parseme_ptr child = N->children.front();
			pre_access_expression(child);
			output_stream() << rvalue_name(N) << " = load " << logical_type_name(child) << " " << rvalue_name(child) << std::endl;
			break;
		}
		
		case ID::dereference:
		{
			sooty::parseme_ptr child = N->children.front();
			pre_access_expression(child);
			if ( N->parent.lock()->id != ID::member_operator ) {
				output_stream() << rvalue_name(N) << " = load " << logical_type_name(child) << " " << rvalue_name(child) << std::endl;
			}
			break;
		}
		
		// the address-of of a member-operator [sic] needs a distinct calculation
		case ID::address_of:
		{
			pre_mutate_expression(N->children.front());
			break;
		}
		
		case ID::assignment:
		{
			sooty::parseme_ptr lhs = marshall::binary_expression::lhs(N);
			sooty::parseme_ptr rhs = marshall::binary_expression::rhs(N);
			//bool lhs_is_ref = resolve::type_of(lhs)->id == ID::reference_type;
			
			pre_mutate_expression(lhs);
			//if (lhs_is_ref)
			//	pre_mutate_expression(rhs);
			//else
				pre_access_expression(rhs);
			
			//if (lhs_is_ref)
			//	output_stream()
			//		<< "store " << storage_type_name(rhs) << " " << lvalue_name(rhs) << ", " << storage_type_name(lhs) << " " << lvalue_name(lhs) << "\n";
			//else
				output_stream()
					<< "store " << logical_type_name(rhs) << " " << rvalue_name(rhs) << ", " << storage_type_name(lhs) << " " << lvalue_name(lhs) << "\n";
			break;
		}
		
		case ID::new_:
		{
			// find the block
			sooty::parseme_ptr block = sooty::direct_upwards_find_first_if(N, sooty::id_matches(ID::block));
			
			sooty::parseme_ptr type = marshall::new_::type(N); //resolve::type_of_pred( N->children[0])();
			
			//int num_elements = 1;
			if (type->id == ID::array_type)
				output_stream() << rvalue_name(N) << " = malloc " << logical_type_name(marshall::array_type::type(type)) << ", i32 " << marshall::array_type::arity(type)->value.integer << std::endl;
			else
				output_stream() << rvalue_name(N) << " = malloc " << logical_type_name(type) << std::endl;
			
			sooty::parseme_container& new_arguments = marshall::new_::arguments(N)->children;
			if (!new_arguments.empty())
			{
				// first, perform the correct actions
				std::for_each(new_arguments.begin(), new_arguments.end(), llvm::pre_access_expression);
				
				// now create a function-call for our construction. we dereference our new-expression,
				// passing it as the first argument in a member-function-call. then we add all the
				// arguments specified (by the coder).
				sooty::parseme_ptr dr = sooty::make_parseme(sooty::parseme_ptr(), ID::dereference);
				dr->children.push_back(N);
				sooty::parseme_ptr fc = prism::synthesize::member_function_call(N, "__constructor__", dr);
				marshall::function_call::argument_list(fc)->children.insert(
					marshall::function_call::argument_list(fc)->children.end(),
					new_arguments.begin(),
					new_arguments.end()
				);
				
				// resolve our synthesized function-call to a function
				sooty::parseme_ptr function = prism::resolve::function_of_member_function_call(fc);
				ATMA_ASSERT(function);
				
				// since we're generating this function-call post-block processing, we'll
				// need to do the inlining manually.
				if ( prism::inlining::should_intrinsically_inline(fc) )
				{
					prism::inlining::results context = prism::inlining::build_inline_context(fc);
					std::for_each(context.statements_to_insert.begin(), context.statements_to_insert.end(), llvm::statement);
				}
				else
				{
					output_stream() << "call void @" << prism::decorate_function(function) << "(";
					output_stream(false) << logical_type_name(N) << " " << rvalue_name(N) << ", ";
					for (sooty::parseme_container::iterator i = new_arguments.begin(); i != new_arguments.end(); ++i)
					{
						if (i != new_arguments.begin())
							output_stream(false) << ", ";
						output_stream(false) << logical_type_name(*i) << " " << rvalue_name(*i);
					}
					output_stream(false) << ")" << std::endl;
				}
			}
			
			break;
		}
		
		case ID::string_literal:
		{
			output_stream() << rvalue_name(N) << " = getelementptr " << storage_type_name(N) << " " << lvalue_name(N) << ", i32 0, i32 0" << std::endl;
			break;
		}
		
		
	}
}
