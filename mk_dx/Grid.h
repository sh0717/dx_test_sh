#pragma once
#include <cassert>
#include <array>
#include<algorithm>
#include <cmath>
#include "Grid.h"
#include "d3dUtil.h"
#include "Constants.h"



using namespace std;

enum INTERPOLATIONMETHOD {
	ELINEAR ,
	EMONOTONICCUBIC

};



//grid 하나마다 여러개의 그리드 데이터를 가질수 있다. 
//덴서티용 프레셔용 , ux 용 등등
template<int X, int Y, int Z>
class GridData
{
public:
	GridData();
	~GridData();
	

	double& operator()(int i, int j, int k);
	auto begin();
	auto end();

	double interp(FXMVECTOR  position);
	std::array<double, X* Y* Z> mData;
private:
	static INTERPOLATIONMETHOD interpol;

	double LINEARinterp(FXMVECTOR position);
	double monotonicCubicInterpolation(FXMVECTOR position);
	double ONEDmonotonicCubicInterpolation(const double f[], const double t);

	int sign(const double num) const;
	int constrainIndex(const int idx, const int N) const;

	const int maxNx;
	const int maxNy;
	const int maxNz;

	
	
	//where data are stored
};

template <int X, int Y, int Z>
GridData<X, Y, Z>::GridData() : mData(), maxNx(X - 1), maxNy(Y - 1), maxNz(Z - 1) {}

template <int X, int Y, int Z>
GridData<X, Y, Z>::~GridData() {}




template <int X, int Y, int Z>
INTERPOLATIONMETHOD GridData<X, Y, Z>::interpol = INTERPOLATIONMETHOD::EMONOTONICCUBIC;




template <int X, int Y, int Z>
double& GridData<X, Y, Z>::operator()(int i, int j, int k)
{
	assert((i >= 0 || i < X) || (j >= 0 || j < Y) || (k >= 0 || k < Z));
	return mData[i + X * j + X * Y * k];
};


template <int X, int Y, int Z>
auto GridData<X, Y, Z>::begin()
{
	return mData.begin();
};

template <int X, int Y, int Z>
auto GridData<X, Y, Z>::end()
{
	return mData.end();
};


template <int X, int Y, int Z>
double GridData<X, Y, Z>::interp(FXMVECTOR position) {

	switch (interpol)
	{
	case ELINEAR:
		return LINEARinterp(position);
	case EMONOTONICCUBIC:

		return monotonicCubicInterpolation(position);
	}

};



template <int X, int Y, int Z>
double GridData<X, Y, Z>::LINEARinterp(FXMVECTOR position) {
	//firstly clamp position

	XMVECTOR minvec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR maxvec = XMVectorSet((float)maxNx * VOXELSIZE - 1e-6, (float)maxNx * VOXELSIZE - 1e-6, (float)maxNx * VOXELSIZE - 1e-6,0.0);

	XMVECTOR pos = XMVectorClamp(position, minvec, maxvec);

	float posX = XMVectorGetX(pos);
	float posY = XMVectorGetY(pos);
	float posZ = XMVectorGetZ(pos);


	int i = (int)(posX / VOXELSIZE);
	int j = (int)(posY / VOXELSIZE);
	int k = (int)(posZ / VOXELSIZE);
	//let's get 8 value and interpolate linearlly

	double scale = 1.0 / VOXELSIZE;
	double fractx = scale * (posX - i * VOXELSIZE);   ///  i     ~~~~~~~~~~~   i+1  어느정도?
	double fracty = scale * (posY - j * VOXELSIZE);
	double fractz = scale * (posZ - k * VOXELSIZE);


	assert(fractx < 1.0 && fractx >= 0);
	assert(fracty < 1.0 && fracty >= 0);
	assert(fractz < 1.0 && fractz >= 0);

	/*
	*
	* low z
	y
	2        4



	1        3   x



	high z

	7        8


	5        6

	*/
	int i_1 = constrainIndex(i + 1, maxNx);
	int j_1 = constrainIndex(j + 1, maxNy);
	int k_1 = constrainIndex(k + 1, maxNz);

	//low z 
	double tmp1 = (*this)(i, j, k);
	double tmp2 = (*this)(i, j_1, k);
	double tmp3 = (*this)(i_1, j, k);
	double tmp4 = (*this)(i_1, j_1, k);
	//high z
	double tmp5 = (*this)(i, j, k_1);
	double tmp6 = (*this)(i, j_1, k_1);
	double tmp7 = (*this)(i_1, j, k_1);
	double tmp8 = (*this)(i_1, j_1, k_1);


	double tmp12 = (1 - fracty) * tmp1 + fracty * tmp2;
	double tmp34 = (1 - fracty) * tmp3 + fracty * tmp4;
	double tmp1234 = (1 - fractx) * tmp12 + fractx * tmp34;

	double tmp56 = (1 - fracty) * tmp5 + fracty * tmp6;
	double tmp78 = (1 - fracty) * tmp7 + fracty * tmp8;
	double tmp5678 = (1 - fractx) * tmp56 + fractx * tmp78;



	return   (1 - fractz) * tmp1234 + fractz * tmp5678;

	return 0.0;
};




template <int X, int Y, int Z>
double GridData<X, Y, Z>::monotonicCubicInterpolation(FXMVECTOR position) {
	//firstly clamp position

	XMVECTOR minvec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR maxvec = XMVectorSet((float)maxNx * VOXELSIZE - 1e-6, (float)maxNx * VOXELSIZE - 1e-6, (float)maxNx * VOXELSIZE - 1e-6, 0.0);

	XMVECTOR pos = XMVectorClamp(position, minvec, maxvec);

	float posX = XMVectorGetX(pos);
	float posY = XMVectorGetY(pos);
	float posZ = XMVectorGetZ(pos);


	int i = (int)(posX / VOXELSIZE);
	int j = (int)(posY / VOXELSIZE);
	int k = (int)(posZ / VOXELSIZE);
	//let's get 8 value and interpolate linearlly

	double scale = 1.0 / VOXELSIZE;
	double fractx = scale * (posX - i * VOXELSIZE);   ///  i     ~~~~~~~~~~~   i+1  어느정도?
	double fracty = scale * (posY - j * VOXELSIZE);
	double fractz = scale * (posZ - k * VOXELSIZE);


	assert(fractx < 1.0 && fractx >= 0);
	assert(fracty < 1.0 && fracty >= 0);
	assert(fractz < 1.0 && fractz >= 0);

	
	double arr_z[4];

	for (int z = 0; z < 4; ++z) {
		
		double arr_x[4];
		for (int x = 0; x < 4; ++x) {
			//i-1. i, i+1 ,i+2 해야됨
			int i_cur = constrainIndex(i + x - 1,maxNx);
			int j_1 = constrainIndex(j - 1, maxNy);
			int j_3 = constrainIndex(j + 1, maxNy);
			int j_4 = constrainIndex(j + 2 , maxNy);
			int k_cur = constrainIndex(k + z - 1, maxNz);

			double arr_y[4] = { (*this)(i_cur, j_1, k_cur), (*this)(i_cur, j, k_cur), (*this)(i_cur, j_3, k_cur), (*this)(i_cur, j_4, k_cur) };
			arr_x[x] = ONEDmonotonicCubicInterpolation(arr_y, fracty);
		}
		arr_z[z] = ONEDmonotonicCubicInterpolation(arr_x,fractx);
	}
	return ONEDmonotonicCubicInterpolation(arr_z, fractz);

};
template <int X, int Y, int Z>
double GridData<X, Y, Z>::ONEDmonotonicCubicInterpolation(const double f[], const double t) {


	double delta = f[2] - f[1];

	double d0 = 0.5 * (f[2] - f[0]);
	double d1 = 0.5 * (f[3] - f[1]);

	d0 = (double)sign(delta) * std::abs(d0);
	d1 = (double)sign(delta) * std::abs(d1);

	double a0 = f[1];
	double a1 = d0;
	double a2 = 3 * delta - 2* d0 - d1;
	double a3 = d0 + d1 - 2 * delta;

	return a3 * t * t * t + a2 * t * t + a1 * t + a0;

};



template <int X, int Y, int Z>
int GridData<X, Y, Z>::sign(const double num) const {
	if (num > 0) {
		return 1;
	}
	else if (num < 0) {
		return -1;
	}
	else {
		return 0;
	}
};

template <int X, int Y, int Z>
int GridData<X, Y, Z>::constrainIndex(const int idx, const int N) const
{
	if (idx == -1)
	{
		return 0;
	}
	if (idx > N)
	{
		return N;
	}
	return idx;
};


using GridScalarData = GridData<Nx, Ny, Nz>;
using GridVectorDataX = GridData<Nx + 1, Ny, Nz>;
using GridVectorDataY = GridData<Nx, Ny + 1, Nz>;
using GridVectorDataZ = GridData<Nx, Ny, Nz + 1>;


#define FOR_EACH_CELL                \
    for (int k = 0; k < Nz; ++k)     \
        for (int j = 0; j < Ny; ++j) \
            for (int i = 0; i < Nx; ++i)

#define FOR_EACH_FACE_X              \
    for (int k = 0; k < Nz; ++k)     \
        for (int j = 0; j < Ny; ++j) \
            for (int i = 0; i < Nx + 1; ++i)

#define FOR_EACH_FACE_Y                  \
    for (int k = 0; k < Nz; ++k)         \
        for (int j = 0; j < Ny + 1; ++j) \
            for (int i = 0; i < Nx; ++i)

#define FOR_EACH_FACE_Z              \
    for (int k = 0; k < Nz + 1; ++k) \
        for (int j = 0; j < Ny; ++j) \
            for (int i = 0; i < Nx; ++i)



/// <summary>
/// Allgrid 는 속도장 농도장등 다른 모든 그리드 데이터를 포함한 데이터 
/// 이거를 시뮬레이터한테 주고 업데이트를 시키면 된다 .
/// 그리고 fluid 도 애를 포인터로 가지고 있으면서 보고 버텍스 수정하고 이런식으로 하면 될듯 
/// </summary>
class GridSystem {
public:
	GridSystem();
	~GridSystem();
	XMVECTOR  GetCenter(int i, int j, int k);
	XMVECTOR GetVelocity(FXMVECTOR position);
	 
	double GetVelocityX(FXMVECTOR position);
	double GetVelocityY(FXMVECTOR position);
	double GetVelocityZ(FXMVECTOR position);
	double GetDensity(FXMVECTOR position);
	double GetTemperature(FXMVECTOR position);
	double GetPressure(FXMVECTOR position);


	GridScalarData density;
	GridScalarData density0;
	GridScalarData temperature;
	GridScalarData temperature0;
	GridScalarData pressure;
	
	GridVectorDataX u;
	GridVectorDataX u0;
	GridVectorDataY v;
	GridVectorDataY v0;
	GridVectorDataZ w;
	GridVectorDataZ w0;


	//3D -> 1D data 
	double avg_u[SIZEofGRID], avg_v[SIZEofGRID], avg_w[SIZEofGRID]; // average velocity
	double omg_x[SIZEofGRID], omg_y[SIZEofGRID], omg_z[SIZEofGRID]; //vorticity
	double vort[SIZEofGRID];
	double fx[SIZEofGRID], fy[SIZEofGRID], fz[SIZEofGRID]; // force 


private:


};

