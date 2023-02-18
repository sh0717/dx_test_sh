#pragma once
#include "GeometryGenerator.h"
#include "InputLayouts.h"
#include "NormalMapModel.h"


#include "Effects.h"
#include "gCamera.h"

#include "DirectXMath.h"
#include "DirectXPackedVector.h"
#include "DirectXCollision.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

class BasicModel
{



public:
	BasicModel();
	~BasicModel();
	void Initialize(ID3D11Device* device, Geometry geo, Material mat, CXMMATRIX W, const WCHAR* diffuse);
	void Initialize(ID3D11Device* device, const WCHAR* modelFilename, Material mat, CXMMATRIX W, const WCHAR* diffuse);
	void InitializeObj(ID3D11Device* device, const WCHAR* modelFilename, Material mat, CXMMATRIX W, const WCHAR* diffuse);
	void Shutdown();

	void Render(ID3D11DeviceContext* context,ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, const gCamera& camera);
	void RenderShadow(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech,
		CXMMATRIX world,CXMMATRIX lightViewProj, CXMMATRIX textransform
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



	inline XMMATRIX GetWorld() {
		return XMLoadFloat4x4(&mWorld);
	}

	void SetWorld(CXMMATRIX W) {
		XMStoreFloat4x4(&mWorld, W);
	}
	

	inline Material GetMaterial()  {
		return mMaterial;
	}

	std::vector<Vertex::Basic32>& GetVertices() {
		return mVertices;
	}

	std::vector<UINT>& GetIndices() {
		return mIndices;
	}

private:
	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;
	//ID3D11Buffer* mVB;
	//ID3D11Buffer* mIB;

	std::vector<Vertex::Basic32> mVertices;
	std::vector<UINT> mIndices;

	XMFLOAT4X4 mWorld;
	Material mMaterial;

	

	ComPtr<ID3D11ShaderResourceView> mDiffuseSRV;
	//ID3D11ShaderResourceView* mDiffuseSRV;
	

	UINT mIndexCount;
	UINT mstride;
};

