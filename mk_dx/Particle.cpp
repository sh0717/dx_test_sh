#include "Particle.h"
#include "Effects.h"
#include "InputLayouts.h"
#include "gCamera.h"

Particle::Particle() 
	:mInitVB(nullptr),mDrawVB(nullptr),mStreamOutVB(nullptr),mTexArraySRV(nullptr),mRandomTexSRV(nullptr)
{
	mFirstRun = true;

	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge = 0.0f;

	mEyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);

}

Particle::~Particle() {
	ReleaseCOM(mInitVB);
	ReleaseCOM(mDrawVB);
	ReleaseCOM(mStreamOutVB);

}



void Particle::SetEyePos(const XMFLOAT3& eyePosW) {
	mEyePosW = eyePosW;

}

void Particle::SetEmitPos(const XMFLOAT3& emitposwW) {
	mEmitPosW = emitposwW;

}
void Particle::SetEmitDir(const XMFLOAT3& emitDirW) {
	mEmitDirW = emitDirW;
}

float Particle::GetAge()const {
	return mAge;
}


void Particle::Initialize(ID3D11Device* device, ParticleEffect* fx, ID3D11ShaderResourceView* texArraySRV,
	ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles)
{
	mMaxParticles = maxParticles;
	mFX = fx;
	mTexArraySRV = texArraySRV;
	mRandomTexSRV = randomTexSRV;

	BuildVB(device);
}


void Particle::Reset() {
	mFirstRun = true;
	mAge = 0;


}
void Particle::Update(float dt, float gameTime)
{
	mGameTime = gameTime;
	mTimeStep = dt;

	mAge += dt;
}


void Particle::BuildVB(ID3D11Device* device) {
	
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex::Particle) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	Vertex::Particle p;
	ZeroMemory(&p, sizeof(Vertex::Particle));
	p.Age = 0.0f;
	p.Type = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	HR(device->CreateBuffer(&vbd, &vinitData, &mInitVB));

	vbd.ByteWidth = sizeof(Vertex::Particle) * mMaxParticles;
	vbd.BindFlags= D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	HR(device->CreateBuffer(&vbd, 0, &mDrawVB));
	HR(device->CreateBuffer(&vbd, 0, &mStreamOutVB));

}

void Particle::Draw(ID3D11DeviceContext* context, const gCamera& camera) {
	XMMATRIX viewproj = camera.viewProj();

	mFX->SetViewProj(viewproj);
	mFX->SetGameTime(mGameTime);
	mFX->SetTimeStep(mTimeStep);
	mFX->SetEyePosW(mEyePosW);
	mFX->SetEmitPosW(mEmitPosW);
	mFX->SetEmitDirW(mEmitDirW);
	mFX->SetTexArray(mTexArraySRV);
	mFX->SetRandomTex(mRandomTexSRV);


	context->IASetInputLayout(InputLayouts::Particle);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Vertex::Particle);
	UINT offset = 0;

	if (mFirstRun)
		context->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);
	else
		context->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);


	context->SOSetTargets(1, &mStreamOutVB, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	mFX->StreamOutTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		mFX->StreamOutTech->GetPassByIndex(p)->Apply(0, context);
		
		if (mFirstRun)
		{
			context->Draw(1, 0);
			mFirstRun = false;
		}
		else
		{
			context->DrawAuto();
		}
	}
	ID3D11Buffer* bufferArray[1] = { nullptr };
	context->SOSetTargets(1, bufferArray, &offset);
	std::swap(mDrawVB, mStreamOutVB);

	context->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	mFX->DrawTech->GetPassByIndex(0)->Apply(0, context);

	context->DrawAuto();

}