/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Bcast operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI

#include <mpi.h>

template<class T>
MPI_Datatype mpi_type();

template<> inline MPI_Datatype mpi_type<bool>()        { return MPI_C_BOOL; }
template<> inline MPI_Datatype mpi_type<char>()        { return MPI_CHAR; }
template<> inline MPI_Datatype mpi_type<double>()      { return MPI_DOUBLE; }
template<> inline MPI_Datatype mpi_type<float>()       { return MPI_FLOAT; }
template<> inline MPI_Datatype mpi_type<int>()         { return MPI_INT; }
template<> inline MPI_Datatype mpi_type<long double>() { return MPI_LONG_DOUBLE; }
template<> inline MPI_Datatype mpi_type<unsigned>()    { return MPI_UNSIGNED; }

#endif // USE_MPI
