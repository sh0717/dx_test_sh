#pragma once
#include "d3dUtil.h"




class gCamera;
struct DirectionalLight;

using namespace std;


class Terrain
{

public:
	struct InitInfo
	{
		std::wstring HeightMapFilename;
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;
		std::wstring BlendMapFilename;
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};
	Terrain();
	~Terrain();


	XMMATRIX GetWorld() const;
	void SetWorld(CXMMATRIX M);

	float GetWidth() const;
	float GetDepth() const;

	float GetHeight(float x, float z) const;


	void Draw(ID3D11DeviceContext* context, const gCamera& camera, DirectionalLight lights[3]);

	void Init(ID3D11Device* device, ID3D11DeviceContext* context, const InitInfo& initInfo);
private:
	void LoadHeightmap();
	void Smooth();
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	float Average(int i, int j);
	bool InBounds(int i, int j);

	void BuildVB(ID3D11Device* device);
	void BuildIB(ID3D11Device* device);
	void BuildHeightMapSRV(ID3D11Device* device);


private:


	static const int CellsPerPatch = 64;

	ComPtr<ID3D11Buffer> mQuadPatchVB;
	ComPtr<ID3D11Buffer> mQuadPatchIB;
	
	
	//ID3D11Buffer* mQuadPatchVB;
	//ID3D11Buffer* mQuadPatchIB;

	ComPtr<ID3D11ShaderResourceView> mLayerMapArraySRV;
	ComPtr<ID3D11ShaderResourceView> mBlendMapSRV;
	ComPtr<ID3D11ShaderResourceView> mHeightMapSRV;
	

	//ID3D11ShaderResourceView* mLayerMapArraySRV;
	//ID3D11ShaderResourceView* mBlendMapSRV;
	//ID3D11ShaderResourceView* mHeightMapSRV;

	InitInfo mInfo;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;

	Material mMat;

	std::vector<XMFLOAT2> mPatchBoundsY;



	std::vector<float> mHeightmap;



};
