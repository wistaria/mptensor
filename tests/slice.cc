/*
  Apr. 24, 2015
  Copyright (C) 2015 Satoshi Morita
 */

#include <mpi.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>

#include <mptensor.hpp>
#include "mpi_tool.hpp"
#include "functions.hpp"
#include "typedef.hpp"

namespace tests {


//! Test for TensorD::slice
/*! B = A[:, 3:6, :, :] and C = A[:, 2:4, :, 1:5]

  \param comm MPI communicator
  \param L size of tensor A.shape = (L, L+1, L+2, L+3)
  \param ostrm output stream for results
*/
void test_slice(const MPI_Comm &comm, int L, std::ostream &ostrm) {
  using namespace mptensor;
  const double EPS = 1.0e-10;
  double time0, time1, time2, time3;
  int mpirank;
  int mpisize;
  bool mpiroot;
  mpi_info(comm, mpirank, mpisize, mpiroot);

  int N0 = L;
  int N1 = L+1;
  int N2 = L+2;
  int N3 = L+3;

  if (N1<6) N1 = 6;
  if (N3<5) N3 = 5;

  TensorD A(Shape(N0, N1, N2, N3));

  Shape shape_A = A.shape();
  for(size_t i=0;i<A.local_size();++i) {
    Index index = A.global_index(i);
    double val = func4_1(index, shape_A);
    A.set_value(index, val);
  }

  time0 = MPI_Wtime();

  // B = A[:, 3:6, :, :]
  TensorD B = slice(A, 1, 3, 6);

  time1 = MPI_Wtime();

  // C = A[:, 2:4, :, 1:5]
  TensorD C = slice(A, Index(0,2,0,1), Index(0,4,0,5));

  time2 = MPI_Wtime();

  double error = 0.0;
  for(size_t i=0;i<B.local_size();++i) {
    Index index = B.global_index(i);
    double val;
    B.get_value(index, val);

    index[1] += 3;
    double exact = func4_1(index, shape_A);

    if(error < fabs(val-exact) ) error = fabs(val-exact);
  }

  for(size_t i=0;i<C.local_size();++i) {
    Index index = C.global_index(i);
    double val;
    C.get_value(index, val);

    index[1] += 2;
    index[3] += 1;
    double exact = func4_1(index, shape_A);

    if(error < fabs(val-exact) ) error = fabs(val-exact);
  }

  time3 = MPI_Wtime();

  double max_error;
  MPI_Reduce(&error, &max_error, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

  if(mpiroot) {
    ostrm << "========================================\n"
              << "slice <double> (A[N0,N1,N2,N3], 1, 3, 6) = B[N0, 3, N2, N3]\n"
              << "slice <double> (A[N0,N1,N2,N3], Index(0,2,0,1), Index(0,4,0,5)) = C[N0, 2, N2, 4]\n"
              << "[N0, N1, N2, N3] = " <<  A.shape() << "\n"
              << "Error= " << max_error << "\n"
              << "Time(B)= " << time1-time0 << " [sec]\n"
              << "Time(C)= " << time2-time1 << " [sec]\n"
              << "Time(check)= " << time3-time2 << " [sec]\n"
              << "----------------------------------------\n";
    ostrm << "A: ";
    A.print_info(ostrm);
    ostrm << "B: ";
    B.print_info(ostrm);
    ostrm << "C: ";
    C.print_info(ostrm);
    ostrm << "========================================" << std::endl;
  }
  assert(error < EPS);
  MPI_Barrier(comm);
}


//! Test for TensorC::slice
/*! B = A[:, 3:6, :, :] and C = A[:, 2:4, :, 1:5]

  \param comm MPI communicator
  \param L size of tensor A.shape = (L, L+1, L+2, L+3)
  \param ostrm output stream for results
*/
void test_slice_complex(const MPI_Comm &comm, int L, std::ostream &ostrm) {
  using namespace mptensor;
  const double EPS = 1.0e-10;
  double time0, time1, time2, time3;
  int mpirank;
  int mpisize;
  bool mpiroot;
  mpi_info(comm, mpirank, mpisize, mpiroot);

  int N0 = L;
  int N1 = L+1;
  int N2 = L+2;
  int N3 = L+3;

  if (N1<6) N1 = 6;

  TensorC A(Shape(N0, N1, N2, N3));

  Shape shape_A = A.shape();
  for(size_t i=0;i<A.local_size();++i) {
    Index index = A.global_index(i);
    complex val = cfunc4_1(index, shape_A);
    A.set_value(index, val);
  }

  time0 = MPI_Wtime();

  // B = A[:, 3:6, :, :]
  TensorC B = slice(A, 1, 3, 6);

  time1 = MPI_Wtime();

  // C = A[:, 2:4, :, 1:5]
  TensorC C = slice(A, Index(0,2,0,1), Index(0,4,0,5));

  time2 = MPI_Wtime();

  double error = 0.0;
  for(size_t i=0;i<B.local_size();++i) {
    Index index = B.global_index(i);
    complex val;
    B.get_value(index, val);

    index[1] += 3;
    complex exact = cfunc4_1(index, shape_A);

    if(error < std::abs(val-exact) ) error = std::abs(val-exact);
  }

  for(size_t i=0;i<C.local_size();++i) {
    Index index = C.global_index(i);
    complex val;
    C.get_value(index, val);

    index[1] += 2;
    index[3] += 1;
    complex exact = cfunc4_1(index, shape_A);

    if(error < std::abs(val-exact) ) error = std::abs(val-exact);
  }

  time3 = MPI_Wtime();

  double max_error;
  MPI_Reduce(&error, &max_error, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
  if(mpiroot) {
    ostrm << "========================================\n"
              << "slice <complex> (A[N0,N1,N2,N3], 1, 3, 6) = B[N0, 3, N2, N3]\n"
              << "slice <complex> (A[N0,N1,N2,N3], Index(0,2,0,1), Index(0,4,0,5)) = C[N0, 2, N2, 4]\n"
              << "[N0, N1, N2, N3] = " <<  A.shape() << "\n"
              << "Error= " << max_error << "\n"
              << "Time(B)= " << time1-time0 << " [sec]\n"
              << "Time(C)= " << time2-time1 << " [sec]\n"
              << "Time(check)= " << time3-time2 << " [sec]\n"
              << "----------------------------------------\n";
    ostrm << "A: ";
    A.print_info(ostrm);
    ostrm << "B: ";
    B.print_info(ostrm);
    ostrm << "C: ";
    C.print_info(ostrm);
    ostrm << "========================================" << std::endl;
  }
  assert(error < EPS);
  MPI_Barrier(comm);
}


} // namespace tests
