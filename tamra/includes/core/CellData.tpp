#include"CellData.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
CellData::CellData()
: value{0} {}


//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get the computation load of the cell
double CellData::getLoad(bool isLeaf, const std::shared_ptr<void> &cell) const {
  return isLeaf ? 1. : 0.;
}

// Get the value of the cell
double CellData::getValue() const {
  return value;
}


//***********************************************************//
//  MUTATORS                                                 //
//***********************************************************//

// Set the value of the cell
void CellData::setValue(const double value) {
  this->value = value;
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Init cell data as a vector of double
void CellData::fromVectorOfData(const std::vector<double> &buffer) {
  value = buffer[0];
}

// Return cell data as a vector of double
std::vector<double> CellData::toVectorOfData() const {
  return {value};
}

// Return cell data size
unsigned CellData::getDataSize() const {
  return 1;
}
