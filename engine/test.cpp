#include "test.h"
#include "util.h"

std::list<AddTest> & all_tests()
{
	static std::list<AddTest> tests;
	return tests;
}

AddTest::AddTest(const std::string & test_suite, const std::string & test_name, TestFunction test_function)
	: suite(test_suite), name(test_name), impl(test_function)
{
	all_tests().push_back(*this);
}

TestException::TestException(const std::string & ex_filename, int ex_linenumber, const std::string & message)
	: filename(ex_filename), line(ex_linenumber), what(message)
{
}

std::string current_suite_name()
{
	return "";
}

void run_all_tests(int argc, char ** argv)
{
	bool tests_specified = argc > 1;
	bool all_tests_are_ok = true;
	for(std::list<AddTest>::const_iterator test = all_tests().begin(); test != all_tests().end(); ++test) {
		if(tests_specified) {
			bool found = false;
			for(int i = 1; i < argc; ++i) {
				if(test->name == argv[i]) {
					found = true;
				}
			}
			if(!found) {
				continue;
			}
		}
		bool ok = true;
		try {
			test->impl();
		} catch(const TestException & e) {
			ok = false;
			std::cout << "[FAIL] " << test->suite << test->name << std::endl;
			std::cerr << e.filename << ":" << e.line << ": " << e.what << std::endl;
		}
		if(ok) {
			std::cout << "[ OK ] " << test->suite << test->name << std::endl;
		} else {
			all_tests_are_ok = false;
		}
	}
	if(all_tests_are_ok) {
		std::cout << "All tests are passed." << std::endl;
	} else {
		std::cout << "Some tests are failed!" << std::endl;
	}
}

