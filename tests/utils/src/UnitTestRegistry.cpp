#include "../includes/UnitTestRegistry.h"

//***********************************************************//
//  DATA                                                     //
//***********************************************************//

// Tests that run in serial
std::vector<UnitTest> UnitTestRegistry::serialTests;

// Tests that run in parallel
std::vector<UnitTest> UnitTestRegistry::parallelTests;

// Label for prefix outputs
std::string UnitTestRegistry::label = "";


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Register a serial test
void UnitTestRegistry::registerSerialTest(const std::string &name, FuncType func, const std::string &group) {
  serialTests.emplace_back(name, group, func); // LEAD TO ERROR
}

// Register a parallel test
void UnitTestRegistry::registerParallelTest(const std::string &name, FuncType func, const std::string &group) {
  parallelTests.emplace_back(name, group, func); // LEAD TO ERROR
}

// Run all registered serial tests
void UnitTestRegistry::runSerial() {
  std::cout << label << "==== Running Serial Unit Tests ====" << std::endl;

  runTests(serialTests);
}

// Run all registered parallel tests
void UnitTestRegistry::runParallel(const bool display) {
  if (display)
    std::cout << label << "==== Running Parallel Unit Tests ====" << std::endl;

  runTests(parallelTests, display);
}

// Run an array of tests
void UnitTestRegistry::runTests(std::vector<UnitTest> &tests, const bool display) {
std::map<std::string, std::vector<const UnitTest*>> grouped_tests;
  int passed = 0, failed = 0;

  for (const auto &test : tests)
    grouped_tests[test.getGroup()].push_back(&test);

  for (const auto &[group, tests] : grouped_tests) {
    if (display)
      std::cout << label << ">> Group: " << group << " (" << tests.size() << " tests)\n";
    int group_passed = 0, group_failed = 0;

    for (const UnitTest* test : tests) {
      bool result = false;
      try {
        result = test->run();
      } catch (const std::exception &e) {
        std::cerr << label << "❌ EXCEPTION in test " << test->getName() << ": " << e.what() << '\n';
      } catch (...) {
        std::cerr << label << "❌ UNKNOWN ERROR in test " << test->getName() << '\n';
      }

      if (result) {
        if (display)
          std::cout << label << "   ✅ " << test->getName() << "\n";
        ++passed; ++group_passed;
      } else {
        if (display)
          std::cout << label << "   ❌ " << test->getName() << "\n";
        ++failed; ++group_failed;
      }
    }

    if (display)
      std::cout << label << "   -> " << group_passed << " passed, " << group_failed << " failed" << std::endl;
  }

  if (display)
    std::cout << label << "==== Overall Summary: " << passed << " passed, " << failed << " failed ====" << std::endl;
}
