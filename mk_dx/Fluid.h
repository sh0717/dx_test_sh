#pragma once
#include "d3dUtil.h"
#include "Grid.h"
#include <memory>
#include "Simulator.h"
#include "Grid.h"
class gCamera;
class TestEffect;

class Fluid {

public:
	Fluid();
	~Fluid();

	void Initialize(ID3D11Device* device,ID3D11DeviceContext* context ,TestEffect* fx, shared_ptr<GridSystem> grids);
	void SetEyePos(const XMFLOAT3& eyePosW);
	void SetEmitPos(const XMFLOAT3& emitPosW);
	void SetEmitDir(const XMFLOAT3& emitDirW);
	float GetAge()const;
	void Reset();
	void Update(float dt, float gameTime);
	void Draw(ID3D11DeviceContext* context, const gCamera& camera , ID3D11Buffer* vb, ID3D11Buffer* ib, FXMMATRIX viewproj);

	shared_ptr<GridSystem> mGrid;
	unique_ptr<Simulator> mSimulator;
	ID3D11ShaderResourceView* gridSRV;
	ID3D11Texture3D* m_voxels;

private:
	void BuildVB(ID3D11Device* device);
private:
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mContext;
	TestEffect* mFX;

	bool mFirstRun;

	float mGameTime;
	float mTimeStep;
	float mAge;


	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;
	



	XMFLOAT3 mEyePosW;
	XMFLOAT3 mEmitPosW;
	XMFLOAT3 mEmitDirW;

};