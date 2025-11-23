/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Wrapper around base MPI functions.
 */

#pragma once

#ifdef USE_MPI
  #include <mpi.h>
#endif // USE_MPI

// MPI initialization
int mpi_init(int *, char ***);

// MPI rank
unsigned mpi_rank();

// MPI size
unsigned mpi_size();

// MPI barrier
int mpi_barrier();

// MPI finalization
int mpi_finalize();
