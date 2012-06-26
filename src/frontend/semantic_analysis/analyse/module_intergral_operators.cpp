#include <prism/frontend/semantic_analysis/analyse.hpp>
//=====================================================================
#include <boost/bind.hpp>
//=====================================================================
#include <sooty/frontend/syntactic_analysis/parser.hpp>
#include <sooty/frontend/syntactic_analysis/algorithm.hpp>
//=====================================================================
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/syntactical_analysis/marshall.hpp>
#include <prism/frontend/syntactical_analysis/synthesize.hpp>
#include <prism/frontend/semantic_analysis/resolve/identifier.hpp>
#include <prism/frontend/semantic_analysis/resolve/type_definition.hpp>
#include <prism/frontend/semantic_analysis/resolve/primitive.hpp>
#include <prism/frontend/semantic_analysis/resolve/function.hpp>
//=====================================================================

namespace {
	void integer_operators(sooty::parseme_ptr_ref module)
	{
		using namespace prism;
		
		//=====================================================================
		// insert type definitions for primitive types
		//=====================================================================
		(sooty::immediate(module)) (
			
			// type int
			sooty::insert(ID::type_definition)
			[
				// name
				sooty::insert(ID::identifier, "int"),
				
				// parameters
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "bitwidth"),
						sooty::insert(ID::bitwidth),
						sooty::insert(ID::bitwidth, 32)
					]
				],
				
				// arguments
				sooty::insert(ID::argument_list),
				
				// members
				sooty::insert(ID::member_definitions, 1)
				[
					sooty::insert(ID::int_type) [
						sooty::insert(ID::identifier, "bitwidth")
					],
					
					// default constructor for ints
					sooty::insert(ID::function, 1)
					[
						sooty::insert(ID::internal),
						sooty::insert(ID::cc_fast),
						sooty::insert(ID::identifier, "__constructor__"),
						sooty::insert(ID::parameter_list),
						sooty::insert(synthesize::type_identifier("void")),
						sooty::insert(ID::init_list),
						sooty::insert(ID::block) [
							sooty::insert(ID::assignment) [
								sooty::insert(ID::identifier, "this"),
								sooty::insert(ID::int_literal, 0)
							],
							sooty::insert(ID::return_statement)
						]
					]
					>>
					// copy constructor for ints
					sooty::insert(ID::function, 1)
					[
						sooty::insert(ID::internal),
						sooty::insert(ID::cc_fast),
						sooty::insert(ID::identifier, "__constructor__"),
						sooty::insert(ID::parameter_list) [
							sooty::insert(ID::parameter) [
								sooty::insert(ID::identifier, "rhs"),
								sooty::insert(ID::type_identifier, "int") [
									sooty::insert(ID::argument_list) [
										sooty::insert(ID::identifier, "bitwidth")
									]
								]
							]
						],
						sooty::insert(synthesize::type_identifier("void")),
						sooty::insert(ID::init_list),
						sooty::insert(ID::block) [
							sooty::insert(ID::assignment) [
								sooty::insert(ID::identifier, "this"),
								sooty::insert(ID::identifier, "rhs")
							],
							sooty::insert(ID::return_statement)
						]
					]
					
				]
			]
			>>
			
			sooty::insert(ID::type_definition) [
				sooty::insert(ID::identifier, "void"),
				sooty::insert(ID::parameter_list),
				sooty::insert(ID::argument_list),
				sooty::insert(ID::void_type, sooty::value_t("void"))
			] >>
			
			sooty::insert(ID::type_definition) [
				sooty::insert(ID::identifier, "bitwidth_type"),
				sooty::insert(ID::parameter_list),
				sooty::insert(ID::argument_list),
				sooty::insert(ID::void_type, sooty::value_t("void"))
			]
			
			>>
			// operator for adding two ints!
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::internal),
				sooty::insert(ID::cc_fast),
				sooty::insert(ID::identifier, "__addition__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(synthesize::type_identifier("int"))
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(synthesize::type_identifier("int"))
					]
				],
				// return-type
				sooty::insert(synthesize::type_identifier("int")),
				// init-list
				sooty::insert(ID::init_list),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::return_statement) [
						sooty::insert(ID::intrinsic_int_add) [
							sooty::insert(ID::identifier, "lhs"),
							sooty::insert(ID::identifier, "rhs")
						]
					]
				]
			] >>
			
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::internal),
				sooty::insert(ID::cc_fast),
				sooty::insert(ID::identifier, "__subtraction__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(synthesize::type_identifier("int"))
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(synthesize::type_identifier("int"))
					]
				],
				// return-type
				sooty::insert(synthesize::type_identifier("int")),
				// init-list
				sooty::insert(ID::init_list),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::return_statement) [
						sooty::insert(ID::intrinsic_int_sub) [
							sooty::insert(ID::identifier, "lhs"),
							sooty::insert(ID::identifier, "rhs")
						]
					]
				]
			] >>
			
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::internal),
				sooty::insert(ID::cc_fast),
				sooty::insert(ID::identifier, "__multiplication__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(synthesize::type_identifier("int"))
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(synthesize::type_identifier("int"))
					]
				],
				// return-type
				sooty::insert(synthesize::type_identifier("int")),
				// init-list
				sooty::insert(ID::init_list),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::return_statement) [
						sooty::insert(ID::intrinsic_int_mul) [
							sooty::insert(ID::identifier, "lhs"),
							sooty::insert(ID::identifier, "rhs")
						]
					]
				]
			] >>
			
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::internal),
				sooty::insert(ID::cc_fast),
				sooty::insert(ID::identifier, "__division__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(synthesize::type_identifier("int"))
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(synthesize::type_identifier("int"))
					]
				],
				// return-type
				sooty::insert(synthesize::type_identifier("int")),
				// init-list
				sooty::insert(ID::init_list),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::intrinsic_int_div) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(ID::identifier, "rhs")
					]
				]
			] >>
			
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::internal),
				sooty::insert(ID::cc_fast),
				sooty::insert(ID::identifier, "__equality__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(synthesize::type_identifier("int"))
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(synthesize::type_identifier("int"))
					]
				],
				// return-type
				sooty::insert(synthesize::type_identifier("bool")),
				// init-list
				sooty::insert(ID::init_list),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::return_statement) [
						sooty::insert(ID::intrinsic_int_eq) [
							sooty::insert(ID::identifier, "lhs"),
							sooty::insert(ID::identifier, "rhs")
						]
					]
				]
			] >>
			
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::internal),
				sooty::insert(ID::cc_fast),
				sooty::insert(ID::identifier, "__less_than__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(synthesize::type_identifier("int"))
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(synthesize::type_identifier("int"))
					]
				],
				// return-type
				sooty::insert(synthesize::type_identifier("bool")),
				// init-list
				sooty::insert(ID::init_list),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::intrinsic_int_lt) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(ID::identifier, "rhs")
					]
				]
			]
		);
	}
}

namespace {
	void char_type(sooty::parseme_ptr_ref module)
	{
		using namespace prism;

		(sooty::immediate(module)) (
			sooty::insert(ID::type_definition) [
				sooty::insert(ID::identifier, "char"),
				sooty::insert(ID::parameter_list),
				sooty::insert(ID::argument_list),
				sooty::insert(ID::member_definitions)
			]
			>>
			sooty::insert(ID::type_definition) [
				sooty::insert(ID::identifier, "varags"),
				sooty::insert(ID::parameter_list),
				sooty::insert(ID::argument_list),
				sooty::insert(ID::member_definitions)
			]
		);
	}
}

namespace {
	void boolean_operators(sooty::parseme_ptr_ref module)
	{
		using namespace prism;
		
		(sooty::immediate(module)) (
			sooty::insert(ID::type_definition) [
			
				// name!
				sooty::insert(ID::identifier, "bool"),
				
				sooty::insert(ID::parameter_list),
				
				// arguments
				sooty::insert(ID::argument_list),
				
				// members
				sooty::insert(ID::member_definitions)
				/*
				[
				
					// default constructor
					sooty::insert(ID::function, 1)
					[
						sooty::insert(ID::identifier, "__constructor__"),
						sooty::insert(ID::parameter_list) [
							sooty::insert(ID::parameter) [
								sooty::insert(ID::identifier, "lhs"),
								sooty::insert(ID::pointer_type) [
									sooty::insert(ID::type_identifier, "bool")
								]
							]
						],
						sooty::insert(ID::type_identifier, "void"),
						sooty::insert(ID::block) [
							sooty::insert(ID::assignment) [
								sooty::insert(ID::dereference) [
									sooty::insert(ID::identifier, "lhs")
								],
								sooty::insert(ID::bool_literal, "false")
							],
							sooty::insert(ID::return_statement)
						]
					],
					
					// copy constructor
					sooty::insert(ID::function, 1)
					[
						sooty::insert(ID::identifier, "__constructor__"),
						sooty::insert(ID::parameter_list) [
							sooty::insert(ID::parameter) [
								sooty::insert(ID::identifier, "lhs"),
								sooty::insert(ID::pointer_type) [
									sooty::insert(ID::type_identifier, "bool")
								]
							],
							sooty::insert(ID::parameter) [
								sooty::insert(ID::identifier, "rhs"),
								sooty::insert(ID::type_identifier, "bool")
							]
						],
						sooty::insert(ID::type_identifier, "void"),
						sooty::insert(ID::block) [
							sooty::insert(ID::assignment) [
								sooty::insert(ID::dereference) [
									sooty::insert(ID::identifier, "lhs")
								],
								sooty::insert(ID::identifier, "rhs")
							],
							sooty::insert(ID::return_statement)
						]
					]	
				]
				*/
			]
		);
	}
}
#if 0
namespace {
	void real_operators(sooty::parseme_ptr_ref N)
	{
		using namespace prism;
		sooty::parseme_ptr oiu = sooty::make_parseme(N, ID::identifier, "int");
		sooty::parseme_ptr integer_type_definition = prism::resolve::type_definition_of(oiu);
		
		//=====================================================================
		// insert type definitions for primitive types
		//=====================================================================
		(sooty::immediate(N))(
			sooty::insert(ID::type_definition) [
				sooty::insert(ID::identifier, "real"),
				sooty::insert(ID::real_type, sooty::value_t("real", 32))
			] >>
			
			
			// operator for adding two ints!
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::identifier, "__addition__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(ID::identifier, "real")
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(ID::identifier, "real")
					]
				],
				// return-type
				sooty::insert(ID::identifier, "real"),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::add_real_intrinsic) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(ID::identifier, "rhs")
					]
				]
			] >>
			
			// operator for subbing two ints!
			sooty::insert(ID::function, 1)
			[
				// name.
				sooty::insert(ID::identifier, "__subtraction__"),
				// paramter-list
				sooty::insert(ID::parameter_list) [
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(ID::identifier, "real")
					],
					sooty::insert(ID::parameter) [
						sooty::insert(ID::identifier, "rhs"),
						sooty::insert(ID::identifier, "real")
					]
				],
				// return-type
				sooty::insert(ID::identifier, "int"),
				// body
				sooty::insert(ID::block) [
					sooty::insert(ID::sub_real_intrinsic) [
						sooty::insert(ID::identifier, "lhs"),
						sooty::insert(ID::identifier, "rhs")
					]
				]
			]

			
		);
	}
}
#endif


void prism::semantic_analysis::analyse::module_integral_operators(sooty::parseme_ptr_ref N)
{
	boolean_operators(N);
	integer_operators(N);
	char_type(N);
	//real_operators(N);
	
}



