#include "Screen.h"

Screen::Screen() {}

void Screen::Initialize(ID3D11Device* device) {

	GeometryGenerator::MeshData mesh;
	GeometryGenerator gen;
	gen.CreateFullscreenQuad(mesh);

	vector<Vertex::Basic32> vertices(mesh.Vertices.size());

	for (UINT i = 0; i < mesh.Vertices.size(); ++i) {
		vertices[i].Pos = mesh.Vertices[i].Position;
		vertices[i].Tex = mesh.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mesh.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA viniData;
	viniData.pSysMem = &vertices[0];

	HR(device->CreateBuffer(&vbd, &viniData, mVB.GetAddressOf()));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mesh.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mesh.Indices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));



}


Screen::~Screen() {}


void Screen::DrawScreen(ID3D11DeviceContext* context, ID3D11ShaderResourceView* SRV, CXMMATRIX worldviewproj)
{
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	context->IASetInputLayout(InputLayouts::Basic32.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	ID3DX11EffectTechnique* tech = Effects::DebugFX->ViewRRRRTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Effects::DebugFX->SetWorldViewProj(worldviewproj);
		Effects::DebugFX->SetTexture(SRV);

		tech->GetPassByIndex(p)->Apply(0, context);
		context->DrawIndexed(6, 0, 0);

	}

}