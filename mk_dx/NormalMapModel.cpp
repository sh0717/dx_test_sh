#include "NormalMapModel.h"
#include "gCamera.h"
#include "Effects.h"



NormalMapModel::NormalMapModel()
{

}

NormalMapModel::~NormalMapModel() {}

void NormalMapModel::Initialize(ID3D11Device* device, Geometry geo, Material mat, CXMMATRIX W, const WCHAR* diffuse, const WCHAR* normal) {

	XMStoreFloat4x4(&mWorld, W);
	mMaterial = mat;

	HR(D3DX11CreateShaderResourceViewFromFile(device,
		diffuse, 0, 0, mDiffuseSRV.GetAddressOf(), 0));
	HR(D3DX11CreateShaderResourceViewFromFile(device,
		normal, 0, 0, mNormalSRV.GetAddressOf(), 0));

	GeometryGenerator gen;
	GeometryGenerator::MeshData mesh;

	switch (geo)
	{
	case BOX:
		gen.CreateBox(1.0, 1.0, 1.0, mesh);
		break;
	case SPHERE:
		gen.CreateSphere(0.5f, 20, 20, mesh);
		break;
	case CYLINDER:
		gen.CreateCylinder(0.5, 0.5f, 3.0f, 15, 15, mesh);
		break;
	case GRID:
		gen.CreateGrid(20.0f, 30.0f, 50, 40, mesh);
		break;
	default:
		gen.CreateCylinder(1.0f, 1.0f, 2.0f, 20, 20, mesh);
		break;
	}

	mstride = sizeof(Vertex::PosNorTexTan);
	std::vector<Vertex::PosNorTexTan> vertices(mesh.Vertices.size());
	for (UINT i = 0; i < mesh.Vertices.size(); ++i) {
		vertices[i].Pos = mesh.Vertices[i].Position;
		vertices[i].Normal = mesh.Vertices[i].Normal;
		vertices[i].Tex = mesh.Vertices[i].TexC;
		vertices[i].Tangent = mesh.Vertices[i].TangentU;
	}


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNorTexTan) * mesh.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, mVB.GetAddressOf()));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mesh.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mesh.Indices[0];

	mIndexCount = mesh.Indices.size();

	HR(device->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));


}

void NormalMapModel::Shutdown() {
	


}



void NormalMapModel::Render(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, gCamera& camera) {
	//cbperframe 은 채워놓고 

		///cbperobject를 여기서 채운다 .

	UINT stride = mstride;
	UINT offset = 0;
	SetWorld(W);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(InputLayouts::PosNorTexTan.Get());
	context->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		XMMATRIX world = W;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(W);
		XMMATRIX worldViewProj = world * camera.viewProj();

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetTexTransform(TexTransform);
		Effects::NormalMapFX->SetMaterial(mMaterial);
		

		Effects::NormalMapFX->SetDiffuseMap(mDiffuseSRV.Get());
		Effects::NormalMapFX->SetNormalMap(mNormalSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, context);
		context->DrawIndexed(mIndexCount, 0, 0);

	}

}

void NormalMapModel::RenderDisplacement(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, gCamera& camera) {
	UINT stride = mstride;
	UINT offset = 0;
	SetWorld(W);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	context->IASetInputLayout(InputLayouts::PosNorTexTan.Get());
	context->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p) {
		XMMATRIX world = W;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(W);
		XMMATRIX worldViewProj = world * camera.viewProj();
		
		Effects::DisplacementMapFX->SetWorld(world);
		Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
		Effects::DisplacementMapFX->SetViewProj(camera.viewProj());
		Effects::DisplacementMapFX->SetTexTransform(TexTransform);
		Effects::DisplacementMapFX->SetMaterial(mMaterial);

		Effects::DisplacementMapFX->SetDiffuseMap(mDiffuseSRV.Get());
		Effects::DisplacementMapFX->SetNormalMap(mNormalSRV.Get());
	
		activeTech->GetPassByIndex(p)->Apply(0, context);
		context->DrawIndexed(mIndexCount, 0, 0);
	}

	context->HSSetShader(0,0,0);
	context->DSSetShader(0,0,0);

}


void NormalMapModel::RenderBasic(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, gCamera& camera) {
	//cbperframe 은 채워놓고 

		///cbperobject를 여기서 채운다 .
	UINT stride = mstride;
	UINT offset = 0;
	SetWorld(W);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(InputLayouts::Basic32.Get());
	context->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		XMMATRIX world = W;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(W);
		XMMATRIX worldViewProj = world * camera.viewProj();

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(TexTransform);
		Effects::BasicFX->SetMaterial(mMaterial);


		Effects::BasicFX->SetDiffuseMap(mDiffuseSRV.Get());
		

		activeTech->GetPassByIndex(p)->Apply(0, context);
		context->DrawIndexed(mIndexCount, 0, 0);

	}

}



void NormalMapModel::RenderShadow(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech,
	CXMMATRIX world, CXMMATRIX lightViewProj, CXMMATRIX textransform
) {
	UINT stride = mstride;
	UINT offset = 0;
	SetWorld(world);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(InputLayouts::Basic32.Get());
	context->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {

		XMMATRIX worldviewproj = world * lightViewProj;

		Effects::ShadowFX->SetTexTransform(textransform);
		Effects::ShadowFX->SetWorldViewProj(worldviewproj);
		activeTech->GetPassByIndex(p)->Apply(0, context);
		context->DrawIndexed(mIndexCount, 0, 0);
	}

}