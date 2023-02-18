#pragma once


#include "d3dUtil.h"

class gCamera;
class ParticleEffect;

class Particle
{
public:
	Particle();
	~Particle();

	void Initialize(ID3D11Device* device, ParticleEffect* fx, ID3D11ShaderResourceView* texArraySRV, ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles);
	void SetEyePos(const XMFLOAT3& eyePosW);
	void SetEmitPos(const XMFLOAT3& emitPosW);
	void SetEmitDir(const XMFLOAT3& emitDirW);
	float GetAge()const;
	void Reset();
	void Update(float dt, float gameTime);
	void Draw(ID3D11DeviceContext* context, const gCamera& camera);

private:
	void BuildVB(ID3D11Device* device);
private:
	ID3D11Device* mDevice;
	ParticleEffect* mFX;
	ID3D11ShaderResourceView* mTexArraySRV;
	ID3D11ShaderResourceView* mRandomTexSRV;

	UINT mMaxParticles;
	bool mFirstRun;

	float mGameTime;
	float mTimeStep;
	float mAge;

	ID3D11Buffer* mInitVB;
	ID3D11Buffer* mDrawVB;
	ID3D11Buffer* mStreamOutVB;



	XMFLOAT3 mEyePosW;
	XMFLOAT3 mEmitPosW;
	XMFLOAT3 mEmitDirW;
};
