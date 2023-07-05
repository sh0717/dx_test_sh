#include "Fluid.h"
#include "Effects.h"
#include "InputLayouts.h"
#include "gCamera.h"
#include "RenderStates.h"

Fluid::Fluid()
	:mVB(nullptr), mIB(nullptr),gridSRV(nullptr)
{
	mFirstRun = true;

	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge = 0.0f;

	mEyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);

}

Fluid::~Fluid() {
	

}



void Fluid::SetEyePos(const XMFLOAT3& eyePosW) {
	mEyePosW = eyePosW;

}

void Fluid::SetEmitPos(const XMFLOAT3& emitposwW) {
	mEmitPosW = emitposwW;

}
void Fluid::SetEmitDir(const XMFLOAT3& emitDirW) {
	mEmitDirW = emitDirW;
}

float Fluid::GetAge()const {
	return mAge;
}

void Fluid::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, TestEffect* fx, shared_ptr<GridSystem> grids) {
	mDevice = device;
	mContext = context;

	mFX = fx;


	BuildVB(device);

	mGrid = grids;
	mSimulator = make_unique<Simulator>(mGrid);



	D3D11_TEXTURE3D_DESC m_3DTexDesc;
	m_3DTexDesc.Height = Nx;
	m_3DTexDesc.Width = Ny;
	m_3DTexDesc.Depth = Nz;
	m_3DTexDesc.MipLevels = 1;
	m_3DTexDesc.Format = DXGI_FORMAT_R32_FLOAT;
	m_3DTexDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_3DTexDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	m_3DTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_3DTexDesc.MiscFlags = 0;

	HR(device->CreateTexture3D(&m_3DTexDesc, NULL, &m_voxels));


	D3D11_MAPPED_SUBRESOURCE mappedTex;
	//mappedTex.DepthPitch = Nx * Ny;
	//mappedTex.RowPitch = Nx;
	context->Map(m_voxels, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedTex);

	float* pVoxels = (float*)mappedTex.pData;

	float* pTexels = (float*)malloc((20*mappedTex.DepthPitch + 20 * mappedTex.RowPitch + 20));
	UINT depStart;
	UINT rowStart;
	for (UINT dep = 0; dep < m_3DTexDesc.Depth; dep++)
	{
		 depStart = mappedTex.DepthPitch / 4 * dep;
		for (UINT row = 0; row < m_3DTexDesc.Width; row++)
		{
			 rowStart = mappedTex.RowPitch / 4 * row;
			for (UINT col = 0; col < m_3DTexDesc.Height; col++)
			{
				pTexels[depStart + rowStart + col] = mGrid->GetDensity(XMVectorSet(col,row,dep,0.0));
			}
		}
	}
	memcpy((void*)pVoxels, (void*)pTexels, (20 * mappedTex.DepthPitch ));

	context->Unmap(m_voxels, 0);

	free((void*)pTexels);


	HR(device->CreateShaderResourceView(m_voxels, NULL, &gridSRV));
	
}





void Fluid::Reset() {
	mFirstRun = true;
	mAge = 0;


}
void Fluid::Update(float dt, float gameTime)
{
	mGameTime = gameTime;
	mTimeStep = dt;

	mSimulator->Update(dt);

	mAge += dt;

	

	D3D11_MAPPED_SUBRESOURCE mappedTex;
	//mappedTex.DepthPitch = Nx * Ny;
	//mappedTex.RowPitch = Nx;
	mContext->Map(m_voxels, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedTex);

	float* pVoxels = (float*)mappedTex.pData;

	float* pTexels = (float*)malloc((20 * mappedTex.DepthPitch + 20 * mappedTex.RowPitch + 20));
	UINT depStart;
	UINT rowStart;
	for (UINT dep = 0; dep < Nz; dep++)
	{
		depStart = mappedTex.DepthPitch / 4 * dep;
		for (UINT row = 0; row < Nx; row++)
		{
			rowStart = mappedTex.RowPitch / 4 * row;
			for (UINT col = 0; col < Ny; col++)
			{
				pTexels[depStart + rowStart + col] = mGrid->GetDensity(XMVectorSet(col, row, dep, 0.0));
			}
		}
	}
	memcpy((void*)pVoxels, (void*)pTexels, (20 * mappedTex.DepthPitch));

	mContext->Unmap(m_voxels, 0);

	free((void*)pTexels);

	mDevice->CreateShaderResourceView(m_voxels, NULL, &gridSRV);

}


void Fluid::BuildVB(ID3D11Device* device) {


	XMFLOAT3 v[24];


	// Fill in the front face vertex data.
	v[0] = XMFLOAT3(0, 0, 0);
	v[1] = XMFLOAT3(0, 1, 0);
	v[2] = XMFLOAT3(1, 1, 0);
	v[3] = XMFLOAT3(1, 0, 0);

	// Fill in the back face vertex data.
	v[4] = XMFLOAT3(-0, -0, +1);
	v[5] = XMFLOAT3(+1, -0, +1);
	v[6] = XMFLOAT3(+1, +1, +1);
	v[7] = XMFLOAT3(-0, +1, +1);

	// Fill in the top face vertex data.
	v[8] = XMFLOAT3(-0, +1, -0);
	v[9] = XMFLOAT3(-0, +1, +1);
	v[10] = XMFLOAT3(+1, +1, +1);
	v[11] = XMFLOAT3(+1, +1, -0);

	// Fill in the bottom face vertex data.
	v[12] = XMFLOAT3(-0, -0, -0);
	v[13] = XMFLOAT3(+1, -0, -0);
	v[14] = XMFLOAT3(+1, -0, +1);
	v[15] = XMFLOAT3(-0, -0, +1);

	// Fill in the left face vertex data.
	v[16] = XMFLOAT3(-0, -0, +1);
	v[17] = XMFLOAT3(-0, +1, +1);
	v[18] = XMFLOAT3(-0, +1, -0);
	v[19] = XMFLOAT3(-0, -0, -0);

	// Fill in the right face vertex data.
	v[20] = XMFLOAT3(+1, -0, -0);
	v[21] = XMFLOAT3(+1, +1, -0);
	v[22] = XMFLOAT3(+1, +1, +1);
	v[23] = XMFLOAT3(+1, -0, +1);



	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;




	

	

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * 24;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	HR(device->CreateBuffer(&vbd, &vinitData, mVB.GetAddressOf()));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = i;

	HR(device->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));
}

void Fluid::Draw(ID3D11DeviceContext* context, const gCamera& camera, ID3D11Buffer* vb , ID3D11Buffer* ib, FXMMATRIX viewproj) {
	//XMMATRIX viewproj = camera.viewProj();
	
	XMMATRIX worldViewProj = XMMatrixIdentity() * camera.view()*camera.proj();
	worldViewProj = viewproj;
	XMFLOAT3 eyepos = camera.GetPosition();
	mFX->SetEyePosW(eyepos);
	mFX->SetGridMap(gridSRV);
	mFX->SetWorldViewProj(worldViewProj);



	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(InputLayouts::Pos.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DX11_TECHNIQUE_DESC techDesc;
	Effects::TestFX->Testtech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::TestFX->Testtech->GetPassByIndex(p);

		pass->Apply(0, context);

		context->DrawIndexed(36, 0, 0);
	}


	RenderStates::WireOn(mContext);
	Effects::Test2FX->SetWorldViewProj(worldViewProj);
	Effects::Test2FX->Testtech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::Test2FX->Testtech->GetPassByIndex(p);

		pass->Apply(0, context);

		context->DrawIndexed(36, 0, 0);
	}
	RenderStates::WireOff(mContext);
}