#include"CellData.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
CellData<CellType>::CellData(): value{0} {}


//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get the computation load of the cell
template<typename CellType>
double CellData<CellType>::getLoad(const std::shared_ptr<CellType> &cell) const {
  return cell->isLeaf() ? 1. : 0.;
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Init cell data as a vector of double
template<typename CellType>
void CellData<CellType>::fromVectorOfData(const std::vector<double> &buffer) {
  value = buffer[0];
}

// Return cell data as a vector of double
template<typename CellType>
std::vector<double> CellData<CellType >::toVectorOfData() const {
  return {value};
}

// Return cell data size
template<typename CellType>
unsigned CellData<CellType>::getDataSize() const {
  return 1;
}
