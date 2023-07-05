#include "Grid.h"




GridSystem::GridSystem()
	:avg_u(), avg_v(), avg_w(),
	omg_x(), omg_y(), omg_z(), vort(),
	fx(), fy(), fz()
{

	FOR_EACH_CELL{
		density0(i,j,k) = 0.0;
		density0(i,j,k) =0.0;
		temperature0(i, j, k) = 300;
	}

	FOR_EACH_FACE_X{
	
	}
	
}

GridSystem::~GridSystem() {

}

XMVECTOR GridSystem::GetCenter(int i, int j, int k) {

	double xpos = (i + 0.5) * VOXELSIZE;
	double ypos = (j + 0.5) * VOXELSIZE;
	double zpos = (k + 0.5) * VOXELSIZE;
	return XMVectorSet(xpos, ypos, zpos, 0.0);
}


XMVECTOR GridSystem::GetVelocity(FXMVECTOR position) {

	XMVECTOR vel = XMVectorSet(GetVelocityX(position), GetVelocityY(position), GetVelocityZ(position), 0.0);
	return vel;

}

double GridSystem::GetVelocityX(FXMVECTOR position) {
	return u0.interp(position - XMVectorSet(0.0, -0.5 * VOXELSIZE, -0.5 * VOXELSIZE, 0.0));
}
double GridSystem::GetVelocityY(FXMVECTOR position) {
	return v0.interp(position - XMVectorSet(-0.5 * VOXELSIZE, 0.0, -0.5 * VOXELSIZE, 0.0));
}
double GridSystem::GetVelocityZ(FXMVECTOR position) {
	return w0.interp(position - XMVectorSet(-0.5 * VOXELSIZE, -0.5 * VOXELSIZE, 0.0, 0.0));
}

double GridSystem::GetDensity(FXMVECTOR position) {
	return density0.interp(position - XMVectorSet(0.5 * VOXELSIZE, 0.5 * VOXELSIZE, 0.5 * VOXELSIZE, 0.0));
}

double GridSystem::GetTemperature(FXMVECTOR position) {
	return temperature0.interp(position - XMVectorSet(0.5 * VOXELSIZE, 0.5 * VOXELSIZE, 0.5 * VOXELSIZE, 0.0));
}

double GridSystem::GetPressure(FXMVECTOR position) {
	return pressure.interp(position - XMVectorSet(0.5 * VOXELSIZE, 0.5 * VOXELSIZE, 0.5 * VOXELSIZE, 0.0));
}