#include "../includes/UnitTest.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
UnitTest::UnitTest(std::string name, std::string group, FuncType func)
: name(name),
  group(group),
  func(func) {}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Call the funcition to run the test
bool UnitTest::run() const {
  return func();
}
