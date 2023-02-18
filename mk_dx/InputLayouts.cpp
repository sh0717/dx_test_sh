#include "InputLayouts.h"
#include "Effects.h"


#pragma region InputLayouts


ComPtr<ID3D11InputLayout> InputLayouts::Basic32;


ID3D11InputLayout* InputLayouts::Tree32 = nullptr;
ID3D11InputLayout* InputLayouts::Pos = nullptr;
ComPtr<ID3D11InputLayout> InputLayouts::PosNorTexTan(nullptr)
;
ID3D11InputLayout* InputLayouts::InstanceBasic32 = nullptr;
ID3D11InputLayout* InputLayouts::Particle = nullptr;
ComPtr<ID3D11InputLayout> InputLayouts::Terrain(nullptr);

const D3D11_INPUT_ELEMENT_DESC InputLayouts::PosNormal_desc[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayouts::Basic32_desc[3] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayouts::Tree32_desc[2] = {
	{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	,
	{"SIZE",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}


};

const D3D11_INPUT_ELEMENT_DESC InputLayouts::Pos_desc[1] = {
	{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayouts::PosNorTexTan_desc[4] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}

};

const D3D11_INPUT_ELEMENT_DESC InputLayouts::InstanceBasic32_desc[8] = {
	
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }

};
const D3D11_INPUT_ELEMENT_DESC InputLayouts::Particle_desc[5] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

const D3D11_INPUT_ELEMENT_DESC InputLayouts::Terrain_desc[3] =

{
	{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
	{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
};



/// <summary>
/// ///
/// </summary>
/// <param name="device"></param>
void InputLayouts::Initialize(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;
	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Basic32_desc, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, Basic32.GetAddressOf()));

	Effects::TreeFX->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Tree32_desc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Tree32));


	Effects::CubeFX->SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Pos_desc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Pos));



	Effects::NormalMapFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);

	HR(device->CreateInputLayout(PosNorTexTan_desc, 4, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,PosNorTexTan.GetAddressOf()));



	Effects::InstanceBasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InstanceBasic32_desc, 8, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &InstanceBasic32));

	Effects::FireFX->StreamOutTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Particle_desc, 5, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Particle));


	Effects::TerrainFX->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Terrain_desc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, Terrain.GetAddressOf()));


}

void InputLayouts::Shutdown()
{
	
	
	ReleaseCOM(Tree32);
	ReleaseCOM(Pos);
	
	ReleaseCOM(Particle);
	

}

#pragma endregion