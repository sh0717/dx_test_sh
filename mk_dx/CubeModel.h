#pragma once

#include "d3dUtil.h"
#include "gCamera.h"
#include "GeometryGenerator.h"
#include "InputLayouts.h"
#include "Effects.h"
class CubeModel
{
public:
	CubeModel(ID3D11Device* device, const WCHAR* cubemapFilename, float skySphereRadius);
	~CubeModel();

	ID3D11ShaderResourceView* GetSRV()const { return mCubeMapSRV.Get(); }

	void Draw(ID3D11DeviceContext* context, const gCamera& camera);
private:

	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;

	ComPtr<ID3D11ShaderResourceView> mCubeMapSRV;
	 

	UINT mIndexCount;


};

