#pragma once
#include "d3dUtil.h"


namespace Vertex {
	struct PosNormal {
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
	};


	struct Basic32 {
		Basic32() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}
		Basic32(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT2& uv)
			: Pos(p), Normal(n), Tex(uv) {}
		Basic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: Pos(px, py, pz), Normal(nx, ny, nz), Tex(u, v) {}

		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};

	struct Tree32 {
		Tree32() :Pos(0.0,0.0,0.0),Size(0.0,0.0) {}
		Tree32(const XMFLOAT3& p, const XMFLOAT2& s) : Pos(p),Size(s) {}
		Tree32(float px, float py, float pz, float sx, float sy):Pos(px,py,pz),Size(sx,sy) {}

		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};


	struct PosNorTexTan {


		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 Tangent;

	};

	struct Particle {
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		unsigned int Type;


	};

	/*Terrain 용 버텍스*/
	struct Terrain {
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMFLOAT2 BoundsY;
	};


}

class InputLayouts
{
public: 
	static void Initialize(ID3D11Device* device);
	static void Shutdown();

	static const D3D11_INPUT_ELEMENT_DESC PosNormal_desc[2];
	static ComPtr<ID3D11InputLayout> PosNormal;

	//static ID3D11InputLayout* PosNormal;


	static const D3D11_INPUT_ELEMENT_DESC Basic32_desc[3];
	static ComPtr<ID3D11InputLayout> Basic32;

	//static ID3D11InputLayout* Basic32;


	static const D3D11_INPUT_ELEMENT_DESC Tree32_desc[2];
	static ID3D11InputLayout* Tree32;

	static const D3D11_INPUT_ELEMENT_DESC Pos_desc[1];
	static ID3D11InputLayout* Pos;



	static const D3D11_INPUT_ELEMENT_DESC PosNorTexTan_desc[4];
	static ComPtr<ID3D11InputLayout> PosNorTexTan;


	static const D3D11_INPUT_ELEMENT_DESC InstanceBasic32_desc[8];
	static ID3D11InputLayout* InstanceBasic32;

	static const D3D11_INPUT_ELEMENT_DESC Particle_desc[5];
	static ID3D11InputLayout* Particle;

	static const D3D11_INPUT_ELEMENT_DESC Terrain_desc[3];
	static ComPtr<ID3D11InputLayout> Terrain;


};

