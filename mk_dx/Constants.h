#pragma once

// this is constant store part for smoke simulation 

constexpr int GridN = 20;
constexpr int ratio[3] = { 1, 1, 1 };
constexpr int Nx = ratio[0] * GridN, Ny = ratio[1] * GridN, Nz = ratio[2] * GridN;
constexpr int SIZEofGRID = Nx * Ny * Nz;
constexpr int VOXELSIZE = 1.0;




constexpr int TEMPAMBIENT = 50;


constexpr int SOURCE_SIZE_X = (int)(Nx / 4);
constexpr int SOURCE_SIZE_Y = (int)(Ny / 20);
constexpr int SOURCE_SIZE_Z = (int)(Nz / 4);
constexpr int SOURCE_Y_MERGIN = (int)(Ny / 20);


#ifdef _OPENMP
#include <omp.h>
#define OPENMP_FOR _Pragma("omp parallel for")
#define OPENMP_FOR_COLLAPSE _Pragma("omp parallel for collapse(3)")
#else
#define OPENMP_FOR
#define OPENMP_FOR_COLLAPSE
#endif