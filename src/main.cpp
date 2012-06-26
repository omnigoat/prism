#if 1
#define BOOST_WINDOWS_API
#include <fstream>
//=====================================================================
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
//=====================================================================
//#include <atma/unittest/unittest.hpp>
//=====================================================================
#include <atma/assert.hpp>
#include <atma/unittest/run.hpp>
#include <atma/time.hpp>
//=====================================================================
#include <sooty/frontend/lexical_analysis/lexer.hpp>
#include <sooty/frontend/syntactic_analysis/parser.hpp>
//=====================================================================
#include <prism/frontend/lexical_analysis/lex.hpp>
#include <prism/frontend/syntactical_analysis/parse.hpp>
#include <prism/frontend/semantic_analysis/analyse.hpp>
#include <prism/backend/generation.hpp>
//=====================================================================
namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
//=====================================================================
void command_line(const std::string& command) {
	system(command.c_str());
}
//=====================================================================


struct commandline_representation
{
	friend commandline_representation parse_commandline(int, char **);
private:
	bpo::variables_map vm_;
	std::vector<std::string> sourcefiles_;
	bool bad_;
	
public:
	bool bad() const;
	bool help() const;
	bool run_unittests() const;
	bool run_unittestsonly() const;
	bool no_tree() const;
	bool no_genprint() const;
	const std::vector<std::string>& sourcefiles() const;
	void print_usage() const;
};


commandline_representation parse_commandline(int, char **);





//=====================================================================

void print_tree(const sooty::parseme_ptr&, int);
extern atma::time_t full_time;
int main(int arg_count, char ** args)
{
	//=====================================================================
	// 1) parse command-line stuff
	//=====================================================================
	commandline_representation cmlr = parse_commandline(arg_count, args);
	
	
	
	// print help if required
	if (cmlr.bad() || cmlr.help())
	{
		cmlr.print_usage();
		return EXIT_SUCCESS;
	}
	// run unit-tests if required
	else if (cmlr.run_unittestsonly())
	{
		atma::assert::set_handler(&atma::assert::exit_failure_handler);
		return atma::unittest::run( atma::unittest::all_tests() ).checks_failed;
	}
	else if (cmlr.run_unittests())
	{
		atma::assert::set_handler(&atma::assert::exit_failure_handler);
		atma::unittest::run( atma::unittest::all_tests() );
	}
	// exit if required :(
	else if (cmlr.sourcefiles().empty())
	{
		return EXIT_SUCCESS;
	}
	
	atma::assert::set_handler( atma::assert::hard_break_handler );
	
	//=====================================================================
	// 2.1) where our tools are for easy typing
	//=====================================================================
	// basic directories we'll work out of
	bfs::path dependencies_path = bfs::path("..") / "dependencies";
	bfs::path bin_path = bfs::path("..") / "bin";

	// tools we'll use
	bfs::path runshow_path = bin_path / "run_and_show_return_value.bat";
	bfs::path lli_path = dependencies_path / "llvm" / "lli.exe";
	
	
	//=====================================================================
	// 2.2) what the user gave us, and what we'll output
	//=====================================================================
	std::string base_filename = cmlr.sourcefiles().front();

	bfs::path input_filename = base_filename;
	bfs::path output_filename = base_filename + ".ll";
	bfs::path object_filename = base_filename + ".bc";
	
	// error checking
	if ( !bfs::exists(input_filename) )
	{
		input_filename = bfs::current_path() / input_filename;
		if ( !bfs::exists(input_filename) )
		{
			std::cout << "File " << input_filename << " doesn't exist! :(" << std::endl;
			return EXIT_FAILURE;
		}
	}
	
	

	using namespace std;
	using namespace sooty;
	
	// open up the file, and pass it to our lexer
	fstream file(input_filename.string().c_str(), fstream::binary | fstream::in);
	ATMA_ASSERT(file.is_open());
	
	//=====================================================================
	// 0) put everything into memory (muuuch faster)
	//=====================================================================
	std::vector<char> stream_buffer( std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()) );
	
	atma::time_t start, end;
	
	//=====================================================================
	// 1) lexing
	//=====================================================================
	std::cout << "lexing... ";
	start = atma::time();
	lexeme_list lexemes = prism::lex(stream_buffer.begin(), stream_buffer.end());
	end = atma::time();
	std::cout << std::setprecision(2) << std::fixed << atma::convert_time_to_seconds(end - start) << "s" << std::endl;


	//=====================================================================
	// 2) parsing
	//=====================================================================
	std::cout << "parsing... ";
	start = atma::time();
	parseme_ptr root = parseme::create( parseme_ptr(), prism::parsid::root, lexical_position(0, 0, 0) );
	prism::parse(lexemes.begin(prism::channels::main | prism::channels::scoper), lexemes.end(), root);
	end = atma::time();
	std::cout << std::setprecision(2) << std::fixed << atma::convert_time_to_seconds(end - start) << "s" << std::endl;
	
	std::cout << "time spent in base parsers: " << std::setprecision(2) << std::fixed << atma::convert_time_to_seconds(full_time) << "s" << std::endl;
	
	//=====================================================================
	// 3) output for screen
	//=====================================================================
	if (!cmlr.no_tree())
		print_tree(root, 0);
	
	if (!cmlr.no_tree() || !cmlr.no_genprint())
	{
		//=====================================================================
		// 3.1) semantic analysis
		//=====================================================================
		detail::reset_guid();
		sooty::parseme_ptr cloned_root = sooty::clone_tree(root);
		prism::semantic_analysis::semantic_info si;
		prism::semantic_analysis::analyse::module(si, cloned_root);
		//prism::convert_to_ssa(cloned_root);
		if (!cmlr.no_tree()) {
			std::cout << "\ntree:\n------\n";
			print_tree(root, 0);
			std::cout << "\nmutated tree:\n------\n";
			print_tree(cloned_root, 0);
		}
		
		//=====================================================================
		// 3.2) code listing
		//=====================================================================
		if (!cmlr.no_genprint() && si.errors == 0) {
			//prism::llvm_prepass(cloned_root);
			std::cout << "\ncode-listing:\n----------------\n";
			prism::generate_llvm(std::cout, cloned_root);
		}
	}
	
	
	//=====================================================================
	// 4) output for file
	//=====================================================================
	{
		detail::reset_guid();
		//=====================================================================
		// 4.1) semantic analysis
		//=====================================================================
		std::cout << "semantic analysis... ";
		start = atma::time();
		sooty::parseme_ptr cloned_root = sooty::clone_tree(root);
		prism::semantic_analysis::semantic_info si;
		prism::semantic_analysis::analyse::module(si, cloned_root);
		//prism::convert_to_ssa(cloned_root);
		end = atma::time();
		std::cout << std::setprecision(2) << std::fixed << atma::convert_time_to_seconds(end - start) << "s" << std::endl;
		
		//=====================================================================
		// 4.2) code listing
		//=====================================================================
		if (si.errors == 0) 
		{
			std::cout << "code generation... ";
			start = atma::time();
			std::ofstream llvmoutput( output_filename.string().c_str(), std::ostream::out | std::ostream::trunc );
			//prism::llvm_prepass(cloned_root);
			prism::generate_llvm(llvmoutput, cloned_root);
			llvmoutput.close();
			end = atma::time();
			std::cout << std::setprecision(2) << std::fixed << atma::convert_time_to_seconds(end - start) << "s" << std::endl;
			
			//=====================================================================
			// 5) assembling
			//=====================================================================
			std::cout << "assembling... ";
			start = atma::time();
			command_line(  (dependencies_path / "llvm" / "llvm-as.exe").file_string() + " -f " + output_filename.file_string()  );
			end = atma::time();
			std::cout << std::setprecision(2) << std::fixed << atma::convert_time_to_seconds(end - start) << "s" << std::endl;
			
			//=====================================================================
			// optimising
			//=====================================================================
			//command_line( (dependencies_path / "llvm" / "opt.exe").file_string() + " --std-compile-opts  ./test.prism.bc " );
			//command_line( (dependencies_path / "llvm" / "opt.exe").file_string() + " --std-compile-opts " + object_filename.file_string() + 
			//	".bc | " + (dependencies_path / "llvm" / "llvm-dis").file_string() + " > " + object_filename.file_string() + ".opt" );
			
			command_line( (bin_path / "run_and_show_return_value.bat ").file_string() + lli_path.file_string() + " " + object_filename.file_string() );
		}
	}
	
	
	
	
	//=====================================================================
	// 6) linking - WOAH. no linking yet.
	//=====================================================================
	// psyche! no linking required yet.
	
	//=====================================================================
	// 7) run it!
	//=====================================================================
	

	
	std::cin.get();
}

void print_tree(const sooty::parseme_ptr& P, int level)
{
	using namespace sooty;
	using namespace prism;
	
	size_t& id = P->id;
	value_t& value = P->value;
	
	std::string result;
	
	
	switch (id) {
		case ID::root: result = "root-node"; break;

		case ID::int_literal: result = "int-literal: " + boost::lexical_cast<std::string>(value.integer); break;
		case ID::real_literal: result = "real-literal: " + boost::lexical_cast<std::string>(value.real); break;
		case ID::bool_literal: result = "bool-literal: " + value.string; break;
		
		case ID::int_type: result = "int-type"; break;
		case ID::real_type: result = "real-type"; break;
		case ID::bool_type: result = "bool-type"; break;
		case ID::pointer_type: result = "pointer:"; break;
		case ID::typeof: result = "typeof"; break;

		case ID::identifier: result = "identifier: " + value.string; break;
		case ID::type_identifier: result = "type-identifier: " + value.string; break;
		case ID::any_type: result = "any-type"; break;
		case ID::member_definitions: result = "members:"; break;
		
		case ID::function: 
			if (value.integer == 1)
				return;
			result = "function"; break;
		
		case ID::parameter_list: result = "parameter-list"; break;
		case ID::parameter: result = "parameter"; break;
//		case ID::function_body: result = "function-body"; break;
		case ID::function_call: result = "function-call"; break;
		case ID::argument_list: result = "argument-list"; break;
		
		
		case ID::add: result += "addition"; break;
		case ID::sub: result += "subtraction"; break;
		case ID::mul: result += "multiplication"; break;
		case ID::div: result += "division"; break;
		case ID::dereference: result = "dereference"; break;
		case ID::address_of: result = "address_of"; break;
		case ID::new_: result = "new:"; break;
		
		case ID::type_definition: result = "type-definition: (" + boost::lexical_cast<std::string>( reinterpret_cast<long long>(P.get()) ) + ")"; break;
		
		case ID::block: result = "block"; break;
		
		case ID::if_statement: result = "if-statement"; break;
//		case ID::if_block: result = "if-block"; break;
//		case ID::else_block: result = "else-block"; break;
//		case ID::if_cont_block: result = "if-cont-block"; break;
		case ID::postfix: "postfix"; break;
		
		case ID::variable_definition: result = "variable-definition: (" + boost::lexical_cast<std::string>( reinterpret_cast<long long>(P.get()) ) + ")"; break;
		
		case ID::return_statement: result = "return-statement"; break;
		case ID::equ: result = "equlity-comparision"; break;
		case ID::assignment: result = "assignment-statement:"; break;
		default: result = "UNDEFINED"; break;
	}
	
	if (!result.empty()) {
		for (int i = 0; i < level; ++i) std::cout << "  ";
		std::cout << result << std::endl;
	}
	
	for (parseme_container::const_iterator i = P->children.begin(); i != P->children.end(); ++i)
		print_tree(*i, level + 1);
}


//=====================================================================
bool commandline_representation::bad() const
{
	return bad_;
}

bool commandline_representation::help() const

{
	return vm_.count("help") > 0;
}

bool commandline_representation::run_unittests() const
{
	return vm_.count("runtests") > 0;
}

bool commandline_representation::run_unittestsonly() const
{
	return vm_.count("runtestsonly") > 0;
}

bool commandline_representation::no_tree() const
{
	return vm_.count("no-tree") > 0;
}

bool commandline_representation::no_genprint() const
{
	return vm_.count("no-genprint") > 0;
}

const std::vector<std::string>& commandline_representation::sourcefiles() const
{
	return sourcefiles_;
}

void commandline_representation::print_usage() const
{
	std::cout << "Usage: [ --runtests ] files..." << std::endl;
}





commandline_representation parse_commandline(int arg_count, char ** args)
{
	commandline_representation cmlr;
	
	// all options
	bpo::options_description desc("Allowed Options");
	desc.add_options()
		("runtests", "runs unit testing framework")
		("runtestsonly", "runs unit testing framework then quits immediately")
		("no-tree", "disables printing the parse-tree to stdout")
		("no-genprint", "disables print the code-generation to stdout")
		("sourcefile", bpo::value< std::vector<std::string> >(), "input file");

	// position options
	bpo::positional_options_description positional_desc;
	positional_desc.add("sourcefile", -1);

	bpo::variables_map vm;
	
	cmlr.bad_ = false;
	
	try {
		bpo::store(
			bpo::command_line_parser(arg_count, args)
			.options(desc)
			.positional(positional_desc)
			.run(),
			vm);

		bpo::notify(vm);
	}
	catch (std::exception&) {
		std::cerr << desc << std::endl;
		cmlr.bad_ = true;
	}
	catch (...) {
		cmlr.bad_ = true;
	}

	cmlr.vm_ = vm;
	
	if (vm.count("sourcefile"))
		cmlr.sourcefiles_ = vm["sourcefile"].as<std::vector<std::string> >();
	
	return cmlr;
}

#else
#include <string>
#include <sooty/frontend/lexical_analysis/lexer.hpp>
#include <sooty/frontend/syntactic_analysis/sandbox.hpp>
#include <prism/frontend/lexical_analysis/lex.hpp>
#include <prism/frontend/syntactical_analysis/parse.hpp>
int main()
{
	using namespace sooty;
	using namespace prism;
	
	std::string to_lex = " chicken giraffe elephant donkey ";
	lexeme_list lexemes = prism::lex(to_lex.begin(), to_lex.end());
	
	//baal::parser identifier = 
	//	baal::assign("iden") [baal::match(lexid::identifier)];
	//
	//baal::global_parse_state_t global_state;
	//baal::parseme root(NULL, ID::root, NULL);
	//baal::parse_results_t results = identifier.parse( global_state, baal::local_parse_state_t(lexemes.begin(), lexemes.end(), root) );
	//
	
	sooty::parseme_ptr root = sooty::make_parseme(sooty::parseme_ptr(), ID::root, sooty::value_t());
	(sooty::immediate(root)) ((
		sooty::insert(ID::if_statement),
		sooty::insert(ID::member_operator) [
			sooty::insert(ID::identifier, "iden"),
			sooty::insert(ID::function_call) [
				sooty::insert(ID::identifier, "go"),
				sooty::insert(ID::argument_list) [
					sooty::insert(ID::identifier, "lhs"),
					sooty::insert(ID::identifier, "rhs")
				]
			]
		]
	));
	
	
	using cuil::placeholders::_a;
	using cuil::placeholders::_b;
	using cuil::placeholders::_parent;
	
	using cuil::placeholders::_1;
	using cuil::placeholders::_2;
	using cuil::placeholders::_3;
	
	cuil::pattern() (
		cuil::eq(ID::root) [
			cuil::eq(ID::member_operator) [
				_1 = cuil::any(),
				cuil::eq(ID::function_call) [
					_2 = cuil::eq(ID::identifier),
					cuil::eq(ID::argument_list) [
						*(_3 += cuil::any())
					]
				]
			]
			.replace (
				cuil::mk(ID::member_function_call) [
					_2,
					cuil::mk(ID::argument_list) [
						_1,
						_3
					]
				]
			)
		]
	)
	
	(root); //LOL
	
	std::cin.get();
	
}



#endif