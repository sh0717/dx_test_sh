#pragma once
#include "GeometryGenerator.h"
#include "InputLayouts.h"


enum Geometry {
	BOX = 0,
	SPHERE = 1,
	CYLINDER = 2,
	GRID = 3
};
class gCamera;
class NormalMapModel
{
public:
	NormalMapModel();
	~NormalMapModel();
	void Initialize(ID3D11Device* device,Geometry geo, Material mat, CXMMATRIX W,const WCHAR* diffuse, const WCHAR* normal);
	void Shutdown();

	void Render(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, gCamera& camera);
	void RenderBasic(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, gCamera& camera);
	void RenderDisplacement(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, gCamera& camera);
	
	void RenderShadow(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech,
		CXMMATRIX world, CXMMATRIX lightViewProj, CXMMATRIX textransform
	);
	
	inline UINT GetIndexCount() {
		return mIndexCount;
	}
	inline UINT GetStride() {
		return mstride;
	}
	inline ID3D11Buffer* GetVB() {
		return mVB.Get();
	}
	inline ID3D11Buffer* GetIB() {
		return mIB.Get();
	}

	inline ID3D11ShaderResourceView* GetDiffuse() {
		return mDiffuseSRV.Get();
	}

	inline ID3D11ShaderResourceView* GetNormal() {
		return mNormalSRV.Get();
	}

	inline XMMATRIX GetWorld() {
		return XMLoadFloat4x4(&mWorld);
	}

	void SetWorld(CXMMATRIX W) {
		XMStoreFloat4x4(&mWorld, W);
	}


	inline Material GetMaterial() {
		return mMaterial;
	}

private:
	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;

	XMFLOAT4X4 mWorld;
	Material mMaterial;


	ComPtr<ID3D11ShaderResourceView> mDiffuseSRV;
	ComPtr<ID3D11ShaderResourceView> mNormalSRV;
	

	UINT mIndexCount;
	UINT mstride;

};


