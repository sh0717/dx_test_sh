#include "BasicModel.h"
#include "WaveFrontReader.h"


BasicModel::BasicModel()
	:mVB(nullptr), mIB(nullptr), mDiffuseSRV(nullptr)
{}

BasicModel::~BasicModel() {}

void BasicModel::Initialize(ID3D11Device* device, Geometry geo, Material mat, CXMMATRIX W, const WCHAR* diffuse) {

	XMStoreFloat4x4(&mWorld, W);
	mMaterial = mat;

	if (diffuse!=nullptr) {
		HR(D3DX11CreateShaderResourceViewFromFile(device,
			diffuse, 0, 0, mDiffuseSRV.GetAddressOf(), 0));
	}

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

	mstride = sizeof(Vertex::Basic32);
	std::vector<Vertex::Basic32> vertices(mesh.Vertices.size());
	for (UINT i = 0; i < mesh.Vertices.size(); ++i) {
		vertices[i].Pos = mesh.Vertices[i].Position;
		vertices[i].Normal = mesh.Vertices[i].Normal;
		vertices[i].Tex = mesh.Vertices[i].TexC;
		
	}


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mesh.Vertices.size();
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


void BasicModel::Initialize(ID3D11Device* device, const WCHAR* modelFilename, Material mat, CXMMATRIX W, const WCHAR* diffuse) {
	XMStoreFloat4x4(&mWorld, W);
	mMaterial = mat;

	if(diffuse)
	HR(D3DX11CreateShaderResourceViewFromFile(device,
		diffuse, 0, 0, mDiffuseSRV.GetAddressOf(), 0));


	std::ifstream fin(modelFilename);

	if (!fin)
	{
		MessageBox(0, modelFilename, 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;


	XMFLOAT3 vMin3(MathHelper::Infinity, MathHelper::Infinity, MathHelper::Infinity);
	XMFLOAT3 vMax3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMin3);
	XMVECTOR vMax = XMLoadFloat3(&vMax3);

	 mVertices.resize(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> mVertices[i].Pos.x >> mVertices[i].Pos.y >> mVertices[i].Pos.z;
		fin >> mVertices[i].Normal.x >> mVertices[i].Normal.y >> mVertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&mVertices[i].Pos);
		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}
	/*XMStoreFloat3(&mBox.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&mBox.Extents, 0.5f * (-vMin + vMax));*/

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mIndexCount = 3 * tcount;
	mIndices.resize(mIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> mIndices[i * 3 + 0] >> mIndices[i * 3 + 1] >> mIndices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mVertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, mVB.GetAddressOf()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mIndices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));
	mstride = sizeof(Vertex::Basic32);

}

void BasicModel::InitializeObj(ID3D11Device* device, const WCHAR* modelFilename, Material mat, CXMMATRIX W, const WCHAR* diffuse)
{
	XMStoreFloat4x4(&mWorld, W);
	mMaterial = mat;

	if (diffuse) {
		HR(D3DX11CreateShaderResourceViewFromFile(device,
			diffuse, 0, 0, mDiffuseSRV.GetAddressOf(), 0));
	}

	

	WaveFrontReader<UINT> reader;

	reader.Load(modelFilename, true);

	

	mVertices.resize(reader.vertices.size());
	mIndices.resize(reader.indices.size());
	for (int i = 0; i < reader.vertices.size(); ++i) {
		mVertices[i].Pos = reader.vertices[i].position;
		mVertices[i].Normal = reader.vertices[i].normal;
		mVertices[i].Tex = reader.vertices[i].textureCoordinate;

	
	}
	for (int i = 0; i < reader.indices.size(); ++i) {
		mIndices[i] = reader.indices[i];
	}
	mIndexCount = reader.indices.size();



	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mVertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, mVB.GetAddressOf()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mIndices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));
	mstride = sizeof(Vertex::Basic32);
	


}


void BasicModel::Shutdown() {
	

}


void BasicModel::Render(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech, CXMMATRIX W, CXMMATRIX TexTransform, const gCamera& camera) {
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
		Effects::BasicFX->SetViewProj(camera.viewProj());

		Effects::BasicFX->SetDiffuseMap(mDiffuseSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, context);
		context->DrawIndexed(mIndexCount, 0, 0);

	}

}

void BasicModel::RenderShadow(ID3D11DeviceContext* context, ID3DX11EffectTechnique* activeTech,
	CXMMATRIX world, CXMMATRIX lightViewProj, CXMMATRIX textransform
) {
	UINT stride = mstride;
	UINT offset = 0;
	SetWorld(world);
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