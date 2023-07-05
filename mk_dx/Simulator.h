#pragma once
#include "Grid.h"
#include <memory>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Sparse>
//this is simulator for gridsystem

typedef Eigen::Triplet<double> TRI;

using namespace std;

constexpr int IDXto1D(const int i, const int j, const int k) {
	assert((i >= 0 || i < Nx) || (j >= 0 || j < Ny) || (k >= 0 || k < Nz));
	return i + Nx * j + Nx * Ny * k;
}



class Simulator
{
	
public:

	Simulator(shared_ptr<GridSystem> gridsystem);
	~Simulator();

	void Update(const double time);

private:

	void UpdateVariable();



	void AddSource(const double time);

	//force 관련
	void ReSetForce();
	//in force vortex restrict
	void RestricVorticity();
	void AddForce(const double time);



	//pressure 관련
	void AddPressure(const double time);
	//
	 
	 
	//advect 관련
	void AdvectVel(const double time);
	void AdvectScalar(const double time);


	//

	void AddSource();


	shared_ptr<GridSystem> mGrid;

	vector<TRI> tripletList;
	Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower | Eigen::Upper> ICCG;

	Eigen::SparseMatrix<double, Eigen::RowMajor> A;
	Eigen::VectorXd b;
	Eigen::VectorXd x;



};


