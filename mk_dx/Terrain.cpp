#include "Terrain.h"
#include "InputLayouts.h"
#include "gCamera.h"
#include "Effects.h"

Terrain::Terrain()
	
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());


	mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	mMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}



Terrain::~Terrain()
{
	
}
/*
minfo.Heightmapwidth 는 cols
minfo.hegihtmapHeight 는 rows


*/



float Terrain::GetWidth() const {

	return (mInfo.HeightmapWidth - 1) * mInfo.CellSpacing;
}


float Terrain::GetDepth()const {
	return (mInfo.HeightmapHeight - 1) * mInfo.CellSpacing;
}


XMMATRIX Terrain::GetWorld() const {
	return XMLoadFloat4x4(&mWorld);
}

void Terrain::SetWorld(CXMMATRIX M) {
	XMStoreFloat4x4(&mWorld, M);
}

float Terrain::GetHeight(float x, float z) const{

	float c = (x + 0.5f * GetWidth()) / mInfo.CellSpacing;
	float d = (z - 0.5f * GetWidth()) / (-mInfo.CellSpacing);

	int row = (int)floorf(d);
	int col = (int)floorf(c);
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D

	float A = mHeightmap[row * mInfo.HeightmapWidth + col];
	float B = mHeightmap[row * mInfo.HeightmapWidth + col + 1];
	float C = mHeightmap[(row + 1) * mInfo.HeightmapWidth + col];
	float D = mHeightmap[(row + 1) * mInfo.HeightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	if (s + t <= 1.0f) {
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	
	}
	else {
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s) * uy + (1.0f - t) * vy;
	}




}

/// mNUmPathchRows 는 patch 로 잘랐을 때의 row cols 수  고 mNumPathchQuadFaces 가 patch 수지




void Terrain::Init(ID3D11Device* device, ID3D11DeviceContext* context, const InitInfo& initInfo)
{
	mInfo = initInfo;
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	mNumPatchVertRows = ((mInfo.HeightmapHeight - 1) / CellsPerPatch) + 1;
	mNumPatchVertCols = ((mInfo.HeightmapWidth - 1) / CellsPerPatch) + 1;

	mNumPatchVertices = mNumPatchVertRows * mNumPatchVertCols;
	mNumPatchQuadFaces = (mNumPatchVertRows - 1) * (mNumPatchVertCols - 1);

	LoadHeightmap();
	Smooth();
	CalcAllPatchBoundsY();

	BuildVB(device);
	BuildIB(device);
	BuildHeightMapSRV(device);





	/////layerfile -> SRVARRAY

	std::vector<std::wstring> layerFilenames;

	layerFilenames.push_back(mInfo.LayerMapFilename0);
	layerFilenames.push_back(mInfo.LayerMapFilename1);
	layerFilenames.push_back(mInfo.LayerMapFilename2);
	layerFilenames.push_back(mInfo.LayerMapFilename3);
	layerFilenames.push_back(mInfo.LayerMapFilename4);

	/*ComPtr  = operator  포인터 하면 바꾸는듯 만약에 다를 경우에 */
	mLayerMapArraySRV = d3dHelper::CreateTexture2DArraySRV(device, context, layerFilenames
	);


	HR(D3DX11CreateShaderResourceViewFromFile(device,
		mInfo.BlendMapFilename.c_str(), 0, 0, mBlendMapSRV.GetAddressOf(), 0));


}

/* draw strat*/


void Terrain::Draw(ID3D11DeviceContext* context, const gCamera& camera, DirectionalLight lights[3]){
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	context->IASetInputLayout(InputLayouts::Terrain.Get());

	UINT stride = sizeof(Vertex::Terrain);
	UINT offset = 0;


	context->IASetVertexBuffers(0, 1, mQuadPatchVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mQuadPatchIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX viewProj = camera.viewProj();
	
	XMFLOAT4 worldPlanes[6];
	ExtractFrustumPlanes(worldPlanes, viewProj);

	Effects::TerrainFX->SetViewProj(viewProj);
	Effects::TerrainFX->SetEyePosW(camera.GetPosition());
	Effects::TerrainFX->SetDirLights(lights);
	Effects::TerrainFX->SetFogColor(Colors::Silver);

	Effects::TerrainFX->SetFogStart(15.0f);
	Effects::TerrainFX->SetFogRange(175.0f);
	Effects::TerrainFX->SetMinDist(20.0f);
	Effects::TerrainFX->SetMaxDist(500.0f);
	Effects::TerrainFX->SetMinTess(0.0f);
	Effects::TerrainFX->SetMaxTess(6.0f);

	Effects::TerrainFX->SetTexelCellSpaceU(1.0f / mInfo.HeightmapWidth);
	Effects::TerrainFX->SetTexelCellSpaceV(1.0f / mInfo.HeightmapHeight);
	Effects::TerrainFX->SetWorldCellSpace(mInfo.CellSpacing);
	Effects::TerrainFX->SetWorldFrustumPlanes(worldPlanes);

	Effects::TerrainFX->SetLayerMapArray(mLayerMapArraySRV.Get());
	Effects::TerrainFX->SetBlendMap(mBlendMapSRV.Get());
	Effects::TerrainFX->SetHeightMap(mHeightMapSRV.Get());
	Effects::TerrainFX->SetMaterial(mMat);

	ID3DX11EffectTechnique* tech = Effects::TerrainFX->Light1Tech;
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);


	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		ID3DX11EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0, context);

		context->DrawIndexed(mNumPatchQuadFaces * 4, 0, 0);
	}

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	context->HSSetShader(0, 0, 0);
	context->DSSetShader(0, 0, 0);

}



/*draw end */

/// LoadHeightmap 함수는 
///모든 점들의 height 를 하나의 배열 mHeightmap 에 저장한다 .
/// 
/// 순서는 
/// 1234
/// 5678
/// 9 10 11 12
void Terrain::LoadHeightmap() {
	std::vector<unsigned char> in(mInfo.HeightmapHeight * mInfo.HeightmapWidth);

	std::ifstream fin;

	fin.open(mInfo.HeightMapFilename.c_str(), std::ios_base::binary);
	if (fin) {
		fin.read((char*)&in[0], (std::streamsize)in.size());
		fin.close();
	}

	mHeightmap.resize(mInfo.HeightmapHeight * mInfo.HeightmapWidth, 0);
	for (UINT i = 0; i < mInfo.HeightmapHeight * mInfo.HeightmapWidth; ++i) {
		mHeightmap[i] = (in[i] / 255.0f) * mInfo.HeightScale;
	}
}



bool Terrain::InBounds(int i, int j) {

	if (i < 0 || j < 0) {
		return false;
	}

	if (i >= mInfo.HeightmapHeight || j >= mInfo.HeightmapWidth) {
		return false;
	}
	return true;
}


float Terrain::Average(int i, int j) {
	float ret = 0.0f;
	float num = 0.0f;


	for (int m = i - 1; m <= i + 1; m++) {
		for (int n = j - 1; n <= j + 1; n++) {
			if (InBounds(m, n)) {
				ret += mHeightmap[m * mInfo.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}
	if (num != 0.0f) {
		return ret / num;
	}
	else {
		return 0.0f;
	}

}

/// <summary>
/// smooth 는 모든 점들에 대하여 주변의 값들로 좀 평균을 내준다. 
/// </summary>
/// ////// vector 하나 새로 만들어서 거기에 저장한 후에 스왑을 해준다. 
void Terrain::Smooth() {
	std::vector<float> dest(mHeightmap.size());
	for (UINT i = 0; i < mInfo.HeightmapHeight; ++i)
	{
		for (UINT j = 0; j < mInfo.HeightmapWidth; ++j)
		{
			dest[i * mInfo.HeightmapWidth + j] = Average(i, j);
		}
	}
	mHeightmap = dest;

}
/// <summary>
/// // i  행 j 열 
/// patch의 bound 를 결정해준다. 
/// </summary>
void Terrain::CalcAllPatchBoundsY() {
	mPatchBoundsY.resize(mNumPatchQuadFaces);

	for (UINT i = 0; i < mNumPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < mNumPatchVertCols - 1; ++j) {
			CalcPatchBoundsY(i, j);

		}
	}


}



void Terrain::CalcPatchBoundsY(UINT i, UINT j) {
	UINT x0 = j * CellsPerPatch;
	UINT x1 = x0 + CellsPerPatch;

	UINT y0 = i * CellsPerPatch;
	UINT y1 = y0 + CellsPerPatch;


	float minY = +MathHelper::Infinity;
	float maxY = -MathHelper::Infinity;

	for (UINT y = y0; y <= y1; ++y) {
		for (UINT x = x0; x <= x1; ++x) {
			UINT k = y * mInfo.HeightmapWidth + x;
			minY = MathHelper::Min(minY, mHeightmap[k]);
			maxY = MathHelper::Max(maxY, mHeightmap[k]);
		}
	}

	UINT patchID = i * (mNumPatchVertCols - 1) + j;
	mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);


}


/*build vb and ib */
/// patch 의 4각형을 vb 로 넣어준다. 

void Terrain::BuildVB(ID3D11Device* device) {

	vector<Vertex::Terrain> patchVertices(mNumPatchVertRows * mNumPatchVertCols);
	////
	/// i*cols +j  로 vector 


	/*
	0 1 2 3 4
	5 6 .... 이런식으로

	*/

	float halfWidth = 0.5f * GetWidth();
	float halfDepth = 0.5f * GetDepth();
	float patchWidth = GetWidth() / (mNumPatchVertCols - 1);
	float patchDepth = GetDepth() / (mNumPatchVertRows - 1);
	float du = 1.0f/(mNumPatchVertCols - 1);
	float dv = 1.0f / (mNumPatchVertRows - 1);

	for (UINT i = 0; i < mNumPatchVertRows; ++i) {
		float z = halfDepth - i * patchDepth;
		for (UINT j = 0; j < mNumPatchVertCols; ++j) {
			float x = -halfWidth + j * patchWidth;
			patchVertices[i * mNumPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);

			patchVertices[i * mNumPatchVertCols + j].Tex.x = j * du;
			patchVertices[i * mNumPatchVertCols + j].Tex.y = i * dv;
		}
	}

	for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
		{
			UINT patchID = i * (mNumPatchVertCols - 1) + j;
			patchVertices[i * mNumPatchVertCols + j].BoundsY = mPatchBoundsY[patchID];
		}
	}
	/// left top 에 bounds 저장 한다. 


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Terrain) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, mQuadPatchVB.GetAddressOf()));

}


void Terrain::BuildIB(ID3D11Device* device) {

	vector<UINT> indices(mNumPatchQuadFaces * 4);

	int k = 0;
	for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
		{
			// Top row of 2x2 quad patch
			indices[k] = i * mNumPatchVertCols + j;
			indices[k + 1] = i * mNumPatchVertCols + j + 1;

			// Bottom row of 2x2 quad patch
			indices[k + 2] = (i + 1) * mNumPatchVertCols + j;
			indices[k + 3] = (i + 1) * mNumPatchVertCols + j + 1;

			k += 4; // next quad
		}
	}
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, mQuadPatchIB.GetAddressOf()));


}



/*build vb, ib end*/

void Terrain::BuildHeightMapSRV(ID3D11Device* device) {
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mInfo.HeightmapWidth;
	texDesc.Height = mInfo.HeightmapHeight;

	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1; 
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	std::vector<HALF> hmap(mHeightmap.size());
	std::transform(mHeightmap.begin(), mHeightmap.end(), hmap.begin(), XMConvertFloatToHalf);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
	data.SysMemPitch = mInfo.HeightmapWidth * sizeof(HALF);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	HR(device->CreateShaderResourceView(hmapTex, &srvDesc, mHeightMapSRV.GetAddressOf()));

	// SRV saves reference.
	ReleaseCOM(hmapTex);



}