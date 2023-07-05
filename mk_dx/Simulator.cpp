#include "Simulator.h"




Simulator::Simulator(shared_ptr<GridSystem> gridsystem)
	:mGrid(gridsystem), A(SIZEofGRID, SIZEofGRID), b(SIZEofGRID), x(SIZEofGRID)
{

	tripletList.reserve(7 * SIZEofGRID);
	ICCG.setTolerance(1e-8);

};

Simulator::~Simulator() {};

void Simulator::Update(const double time) {

	//2단계-force
	ReSetForce();
	RestricVorticity();
	AddForce(time);

	//3단계 -pressure

	AddPressure(time);
	//start
	UpdateVariable();
	//1단계-advect
	AdvectVel(time);
	AdvectScalar(time);

	AddSource();
};



//fbuoy 에 따라서 힘을 초기화 
void Simulator::ReSetForce() {
	OPENMP_FOR_COLLAPSE
		FOR_EACH_CELL{
				mGrid->fx[IDXto1D(i,j,k)] = 0.0;
				mGrid->fz[IDXto1D(i,j,k)] = 0.0;
				mGrid->fy[IDXto1D(i,j,k)] = -9.8 * mGrid->density(i,j,k) + 10.0 * (mGrid->temperature0(i,j,k) - TEMPAMBIENT);

	}
}


void Simulator::AddForce(const double time) {
	OPENMP_FOR_COLLAPSE
		FOR_EACH_CELL{
			if (i < Nx - 1)
			{
				mGrid->u(i + 1, j, k) += (mGrid->fx[IDXto1D(i, j, k)] + mGrid->fx[IDXto1D(i + 1, j, k)]) * time * 0.5;
			}
			if (j < Ny - 1)
			{
				mGrid->v(i, j + 1, k) += (mGrid->fy[IDXto1D(i, j, k)] + mGrid->fy[IDXto1D(i, j + 1, k)]) * time * 0.5;
			}
			if (k < Nz - 1)
			{
				mGrid->w(i, j, k + 1) += (mGrid->fz[IDXto1D(i, j, k)] + mGrid->fz[IDXto1D(i, j, k + 1)]) * time * 0.5;
			}
	}

}


void Simulator::RestricVorticity() {

//firstly calculate avg_velocity

	OPENMP_FOR_COLLAPSE
		FOR_EACH_CELL
	{
		mGrid->avg_u[IDXto1D(i, j, k)] = (mGrid->u(i, j, k) + mGrid->u(i + 1, j, k)) * 0.5;
		mGrid->avg_v[IDXto1D(i, j, k)] = (mGrid->v(i, j, k) + mGrid->v(i, j + 1, k)) * 0.5;
		mGrid->avg_w[IDXto1D(i, j, k)] = (mGrid->w(i, j, k) + mGrid->w(i, j, k + 1)) * 0.5;
	}
//secondly calculate omg_  which is vorticity
		OPENMP_FOR_COLLAPSE
		FOR_EACH_CELL
	{
		// ignore boundary cells
		if (i == 0 || j == 0 || k == 0)
		{
			continue;
		}
		if (i == Nx - 1 || j == Ny - 1 || k == Nz - 1)
		{
			continue;
		}

		mGrid->omg_x[IDXto1D(i, j, k)] = (mGrid->avg_w[IDXto1D(i, j + 1, k)] - mGrid->avg_w[IDXto1D(i, j - 1, k)] - mGrid->avg_v[IDXto1D(i, j, k + 1)] + mGrid->avg_v[IDXto1D(i, j, k - 1)]) /2 / VOXELSIZE;
		mGrid->omg_y[IDXto1D(i, j, k)] = (mGrid->avg_u[IDXto1D(i, j, k + 1)] - mGrid->avg_u[IDXto1D(i, j, k - 1)] - mGrid->avg_w[IDXto1D(i + 1, j, k)] + mGrid->avg_w[IDXto1D(i - 1, j, k)]) /2 / VOXELSIZE;
		mGrid->omg_z[IDXto1D(i, j, k)] = (mGrid->avg_v[IDXto1D(i + 1, j, k)] - mGrid->avg_v[IDXto1D(i - 1, j, k)] - mGrid->avg_u[IDXto1D(i, j + 1, k)] + mGrid->avg_u[IDXto1D(i, j - 1, k)]) /2 / VOXELSIZE;
	}
		//third find  graident of |vorticity|
	OPENMP_FOR_COLLAPSE
	FOR_EACH_CELL{
		 if (i == 0 || j == 0 || k == 0)
		{
			continue;
		}
		if (i == Nx - 1 || j == Ny - 1 || k == Nz - 1)
		{
			continue;
		}

		XMVECTOR plus, minus;
		plus = XMVector4Length(XMVectorSet(mGrid->omg_x[IDXto1D(i + 1, j, k)], mGrid->omg_y[IDXto1D(i + 1, j, k)], mGrid->omg_z[IDXto1D(i + 1, j, k)], 0.0));
		minus = XMVector4Length(XMVectorSet(mGrid->omg_x[IDXto1D(i - 1, j, k)], mGrid->omg_y[IDXto1D(i - 1, j, k)], mGrid->omg_z[IDXto1D(i - 1, j, k)], 0.0));
		float gradx = (XMVectorGetX(plus) - XMVectorGetX(minus)) / 2 / VOXELSIZE;
		plus = XMVector4Length(XMVectorSet(mGrid->omg_x[IDXto1D(i , j+1, k)], mGrid->omg_y[IDXto1D(i , j+1, k)], mGrid->omg_z[IDXto1D(i , j+1, k)], 0.0));
		minus = XMVector4Length(XMVectorSet(mGrid->omg_x[IDXto1D(i , j-1, k)], mGrid->omg_y[IDXto1D(i , j-1, k)], mGrid->omg_z[IDXto1D(i , j-1, k)], 0.0));
		float grady = (XMVectorGetX(plus) - XMVectorGetX(minus)) / 2 / VOXELSIZE;
		plus = XMVector4Length(XMVectorSet(mGrid->omg_x[IDXto1D(i , j, k+1)], mGrid->omg_y[IDXto1D(i , j, k+1)], mGrid->omg_z[IDXto1D(i , j, k+1)], 0.0));
		minus = XMVector4Length(XMVectorSet(mGrid->omg_x[IDXto1D(i , j, k-1)], mGrid->omg_y[IDXto1D(i , j, k-1)], mGrid->omg_z[IDXto1D(i , j, k-1)], 0.0));
		float gradz = (XMVectorGetX(plus) - XMVectorGetX(minus)) / 2 / VOXELSIZE;

		XMVECTOR gradVort = XMVectorSet(gradx, grady, gradz, 0.0);
		XMVECTOR Nvec = XMVectorSet(0.0, 0.0, 0.0, 0.0);
		float length = XMVectorGetX(XMVector4Length(gradVort));
		if (length > 0) {
			Nvec = XMVector4Normalize(gradVort);
		}

		XMVECTOR Vorvec = XMVectorSet(mGrid->omg_x[IDXto1D(i, j, k)], mGrid->omg_y[IDXto1D(i, j, k)], mGrid->omg_z[IDXto1D(i, j, k)],0.0);
		XMVECTOR fvec = 0.25 * 1.0 *XMVector3Cross(Nvec, Vorvec);

		mGrid->fx[IDXto1D(i, j, k)] += XMVectorGetX(fvec);
		mGrid->fy[IDXto1D(i, j, k)] += XMVectorGetY(fvec);
		mGrid->fz[IDXto1D(i, j, k)] += XMVectorGetZ(fvec);
	}

}

void Simulator::AddPressure(const double time) {

	tripletList.clear();
	A.setZero();
	b.setZero();
	x.setZero();

	double coeff = VOXELSIZE / time;

#pragma omp parallel for collapse(3) ordered
	FOR_EACH_CELL
	{
		double F[6] = {static_cast<double>(k > 0), static_cast<double>(j > 0), static_cast<double>(i > 0),
					   static_cast<double>(i < Nx - 1), static_cast<double>(j < Ny - 1), static_cast<double>(k < Nz - 1)};
		double D[6] = {-1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
		double U[6];
		U[0] = mGrid->w(i, j, k);
		U[1] = mGrid->v(i, j, k);
		U[2] = mGrid->u(i, j, k);
		U[3] = mGrid->u(i + 1, j, k);
		U[4] = mGrid->v(i, j + 1, k);
		U[5] = mGrid->w(i, j, k + 1);
		double sum_F = 0.0;

		for (int n = 0; n < 6; ++n)
		{
			sum_F += F[n];
			b(IDXto1D(i, j, k)) += D[n] * F[n] * U[n];
		}
		b(IDXto1D(i, j, k)) *= coeff;

#pragma omp ordered
		{
			if (k > 0)
			{
				tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i, j, k - 1), F[0]));
			}
			if (j > 0)
			{
				tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i, j - 1, k), F[1]));
			}
			if (i > 0)
			{
				tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i - 1, j, k), F[2]));
			}

			tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i, j, k), -sum_F));

			if (i < Nx - 1)
			{
				tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i + 1, j, k), F[3]));
			}
			if (j < Ny - 1)
			{
				tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i, j + 1, k), F[4]));
			}
			if (k < Nz - 1)
			{
				tripletList.push_back(TRI(IDXto1D(i, j, k), IDXto1D(i, j, k + 1), F[5]));
			}
		}
	}

	A.setFromTriplets(tripletList.begin(), tripletList.end());
	/// make A
	
	ICCG.compute(A);
	
	x = ICCG.solve(b);
	printf("#iterations:     %d \n", static_cast<int>(ICCG.iterations()));
	printf("estimated error: %e \n", ICCG.error());

	

	for (int i = 0; i < x.size(); i++) {
		mGrid->pressure.mData[i] = x[i];
	}

	//add

	OPENMP_FOR_COLLAPSE
	FOR_EACH_CELL{
		 if (i < Nx - 1)
		{
			mGrid->u(i + 1, j, k) -= (time * (mGrid->pressure(i + 1, j, k) - mGrid->pressure(i, j, k))  );
		}
		if (j < Ny - 1)
		{
			mGrid->v(i, j + 1, k) -= time * (mGrid->pressure(i, j + 1, k) - mGrid->pressure(i, j, k))  ;
		}
		if (k < Nz - 1)
		{
			mGrid->w(i, j, k + 1) -= time * (mGrid->pressure(i, j, k + 1) - mGrid->pressure(i, j, k)) ;
		}
	}


}




void Simulator::UpdateVariable() {
	std::copy(mGrid->u.begin(), mGrid->u.end(), mGrid->u0.begin());
	std::copy(mGrid->v.begin(), mGrid->v.end(), mGrid->v0.begin());
	std::copy(mGrid->w.begin(), mGrid->w.end(), mGrid->w0.begin());
	std::copy(mGrid->density.begin(), mGrid->density.end(), mGrid->density0.begin());
	std::copy(mGrid->temperature.begin(), mGrid->temperature.end(), mGrid->temperature0.begin());
}



void Simulator::AdvectVel(const double time) {
	OPENMP_FOR_COLLAPSE
		FOR_EACH_FACE_X
	{
		XMVECTOR pos_u = mGrid->GetCenter(i, j, k) - 0.5 * XMVectorSet(VOXELSIZE, 0, 0,0.0);

		XMVECTOR vel_u = mGrid->GetVelocity(pos_u);
		pos_u = pos_u - time * vel_u;
		mGrid->u(i, j, k) = mGrid->GetVelocityX(pos_u);
	}
		OPENMP_FOR_COLLAPSE
		FOR_EACH_FACE_X
	{
		XMVECTOR pos_v = mGrid->GetCenter(i, j, k) - 0.5 * XMVectorSet(0.0, VOXELSIZE, 0,0.0);

		XMVECTOR vel_u = mGrid->GetVelocity(pos_v);
		pos_v = pos_v - time * vel_u;
		mGrid->v(i, j, k) = mGrid->GetVelocityY(pos_v);
	}
		OPENMP_FOR_COLLAPSE
		FOR_EACH_FACE_X
	{
		XMVECTOR pos_w = mGrid->GetCenter(i, j, k) - 0.5 * XMVectorSet(0.0, 0, VOXELSIZE,0.0);

		XMVECTOR vel_w = mGrid->GetVelocity(pos_w);
		pos_w = pos_w - time * vel_w;
		mGrid->w(i, j, k) = mGrid->GetVelocityZ(pos_w);
	}

};


void Simulator::AdvectScalar(const double time) {
	OPENMP_FOR_COLLAPSE
		FOR_EACH_CELL{
			XMVECTOR pos = mGrid->GetCenter(i, j, k);
			XMVECTOR vel = mGrid->GetVelocity(pos);
			pos = pos - time * vel;

			mGrid->density(i, j, k) = mGrid->GetDensity(pos);
			mGrid->temperature(i, j, k) = mGrid->GetTemperature(pos);
	}
}


void Simulator::AddSource() {




		OPENMP_FOR_COLLAPSE
		for (int k = 7; k < 13; ++k)
		{
			for (int j = 1; j < 5; ++j)
			{
				for (int i =7; i < 13; ++i)
				{
					mGrid->density(i, j, k) = 1.0;
					mGrid->v(i, j, k) = 80.0;
					mGrid->v0(i, j, k) = mGrid->v(i, j, k);
				}
			}
		}
		
}