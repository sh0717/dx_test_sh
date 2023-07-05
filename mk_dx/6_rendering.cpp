
#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "InputLayouts.h"
#include "Waves.h"
#include "RenderStates.h"
#include "gCamera.h"
#include "CubeModel.h"
#include "NormalMapModel.h"
using namespace std;
enum RenderOptions {
	Light=1,
	Texture=2,
	TextureAndFog=3
};
float GetHeight(int x, int z);

class GeoApp : public D3DApp
{
public:
	GeoApp(HINSTANCE hInstance);
	~GeoApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BiuldBuffer();
	void BuildLandandWaves();
	void BuildVertexLayout();
private:
	
	ComPtr<ID3D11RasterizerState> mWireframeRS;

	CubeModel* msky;

	gCamera mCamera;

	ComPtr<ID3D11Buffer> mBoxVB;
	ComPtr<ID3D11Buffer> mBoxIB;

	ComPtr<ID3D11Buffer> mGridVB;
	ComPtr<ID3D11Buffer> mGridIB;
	/*ComPtr 로 하면 ReleaseCom 할 필요 없다*/

	ComPtr<ID3D11InputLayout> mInputLayout;
	size_t mIndexCount_grid;

	XMFLOAT4X4 mWorld;

	ComPtr<ID3D11Buffer> mLandVB;
	ComPtr<ID3D11Buffer> mLandIB;
	ComPtr<ID3D11Buffer> mWavesVB;
	ComPtr<ID3D11Buffer> mWavesIB;

	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;
	
	Waves mWaves;
	UINT mLandIndexCount;

	POINT mLastMousePos;


	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCeterSphereWorld;

	int mSphereVertexOffset;
	int mCylinderVertexOffset;
	int mBoxVertexOffset;
	int mGridVertexOffset;
	

	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;
	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	GeoApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

GeoApp::GeoApp(HINSTANCE hInstance)
: D3DApp(hInstance),mCamera(),msky(nullptr)
{
	
	
	mCamera.SetPosition(0.0f, 10.0, -10.0f);
	mMainWndCaption = L"ddd Demo";

	

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;



	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);



	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCeterSphereWorld, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i) {
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}


}

GeoApp::~GeoApp()
{
	md3dImmediateContext->ClearState();
	
	Effects::Shutdown();
	InputLayouts::Shutdown();
	RenderStates::Shutdown();

}

bool GeoApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::Initialize(md3dDevice);
	InputLayouts::Initialize(md3dDevice);
	RenderStates::Initialize(md3dDevice);

	BuildVertexLayout();

	msky = new CubeModel(md3dDevice, L"../Textures/grasscube1024.dds", 5000.0f);
	mWaves.Init(200, 200, 0.8, 0.03, 3.25, 0.4);

	BiuldBuffer();
	BuildLandandWaves();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));


	return true;
}

void GeoApp::OnResize()
{
	D3DApp::OnResize();
	mCamera.InitLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	
}

void GeoApp::UpdateScene(float dt)
{
	


	//
	// Every quarter second, generate a random wave.
	//

	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	Vertex::PosColor* v = reinterpret_cast<Vertex::PosColor*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	md3dImmediateContext->Unmap(mWavesVB.Get(), 0);





	//if (GetAsyncKeyState('1') & 0x8000)
	//	mRenderOption = RenderOptions::Light;

	//if (GetAsyncKeyState('2') & 0x8000)
	//	mRenderOption = RenderOptions::Texture;

	//if (GetAsyncKeyState('3') & 0x8000)
	//	mRenderOption = RenderOptions::TextureAndFog;

	if (GetAsyncKeyState('W') & 0x8000)
		mCamera.Walk(50.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera.Walk(-50.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera.Strafe(-50.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera.Strafe(50.0f * dt);



	
	mCamera.Update();
}

void GeoApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	
	//msky->Draw(md3dImmediateContext, mCamera);

	
	//md3dImmediateContext->RSSetState(nullptr);

	//UINT stride = sizeof(Vertex::PosColor);
	//UINT offset = 0;

	//md3dImmediateContext->IASetVertexBuffers(0, 1, mBoxVB.GetAddressOf(), &stride, &offset);
	//md3dImmediateContext->IASetIndexBuffer(mBoxIB.Get(), DXGI_FORMAT_R32_UINT, 0);


	//ID3DX11EffectTechnique* activeTech = Effects::ColorFX->ColorTech;
	//XMMATRIX world = XMMatrixIdentity();
	//XMMATRIX view = mCamera.view();
	//XMMATRIX proj = mCamera.proj();

	//XMMATRIX WorldViewProj = world * view * proj;

	//Effects::ColorFX->SetWorldViewProj(WorldViewProj);

	//D3DX11_TECHNIQUE_DESC techDesc;
	//activeTech->GetDesc(&techDesc);
	//for (UINT p = 0; p < techDesc.Passes; ++p) {
	//	activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
	//	md3dImmediateContext->DrawIndexed(36, 0, 0);
	//}

	//md3dImmediateContext->IASetVertexBuffers(0, 1, mGridVB.GetAddressOf(), &stride, &offset);
	//md3dImmediateContext->IASetIndexBuffer(mGridIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	//for (UINT p = 0; p < techDesc.Passes; ++p) {
	//	activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
	//	md3dImmediateContext->DrawIndexed(mIndexCount_grid, 0, 0);
	//}
	//
	UINT stride = sizeof(Vertex::PosColor);
	UINT offset = 0;

	md3dImmediateContext->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	ID3DX11EffectTechnique* activeTech = Effects::ColorFX->ColorTech;
	XMMATRIX view = mCamera.view();
	XMMATRIX proj = mCamera.proj();
	XMMATRIX viewProj = view * proj;


	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		Effects::ColorFX->SetWorldViewProj(world*viewProj);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		//md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		Effects::ColorFX->SetWorldViewProj(world * viewProj);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		//md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw center sphere.
		world = XMLoadFloat4x4(&mCeterSphereWorld);
		Effects::ColorFX->SetWorldViewProj(world * viewProj);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		//md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

		// Draw the cylinders.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			Effects::ColorFX->SetWorldViewProj(world * viewProj);
			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			//md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			Effects::ColorFX->SetWorldViewProj(world * viewProj);
			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			//md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}


	md3dImmediateContext->RSSetState(0);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		md3dImmediateContext->IASetVertexBuffers(0, 1, mLandVB.GetAddressOf(), &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mLandIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
		XMMATRIX worldViewProj = world * view * proj;
		Effects::ColorFX->SetWorldViewProj(worldViewProj);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

		//
		// Draw the waves.
		//

		//md3dImmediateContext->RSSetState(mWireframeRS.Get());

		md3dImmediateContext->IASetVertexBuffers(0, 1, mWavesVB.GetAddressOf(), &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mWavesIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&mWavesWorld);
		worldViewProj = world * view * proj;
		Effects::ColorFX->SetWorldViewProj(worldViewProj);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);
	}



	HR(mSwapChain->Present(0, 0));
}

void GeoApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void GeoApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void GeoApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.15f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.15f*static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
		// Update angles based on input to orbit camera around box.
		

		// Restrict the angle mPhi.
		
	}


	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void GeoApp::BiuldBuffer() {
	//Vertex::PosColor vertices[] =
	//{
	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (XMFLOAT4)Colors::White   },
	//	{ XMFLOAT3(-1.0f, +1.0f, -1.0f),  (XMFLOAT4)Colors::Black   },
	//	{ XMFLOAT3(+1.0f, +1.0f, -1.0f),  (XMFLOAT4)Colors::Red     },
	//	{ XMFLOAT3(+1.0f, -1.0f, -1.0f),  (XMFLOAT4)Colors::Green   },
	//	{ XMFLOAT3(-1.0f, -1.0f, +1.0f),  (XMFLOAT4)Colors::Blue    },
	//	{ XMFLOAT3(-1.0f, +1.0f, +1.0f),  (XMFLOAT4)Colors::Yellow  },
	//	{ XMFLOAT3(+1.0f, +1.0f, +1.0f),  (XMFLOAT4)Colors::Cyan    },
	//	{ XMFLOAT3(+1.0f, -1.0f, +1.0f),  (XMFLOAT4)Colors::Magenta }
	//};

	//D3D11_BUFFER_DESC vbd; 
	///*vertex buffer desc*/
	//vbd.Usage = D3D11_USAGE_IMMUTABLE;
	//vbd.ByteWidth = sizeof(Vertex::PosColor) * 8;
	//vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vbd.CPUAccessFlags = 0;
	//vbd.MiscFlags = 0;
	//vbd.StructureByteStride = 0;
	//D3D11_SUBRESOURCE_DATA viniData;
	//viniData.pSysMem = vertices;

	//HR(md3dDevice->CreateBuffer(&vbd, &viniData, mBoxVB.GetAddressOf()));

	//UINT indices[] = {
	//	// front face
	//	0, 1, 2,
	//	0, 2, 3,

	//	// back face
	//	4, 6, 5,
	//	4, 7, 6,

	//	// left face
	//	4, 5, 1,
	//	4, 1, 0,

	//	// right face
	//	3, 2, 6,
	//	3, 6, 7,

	//	// top face
	//	1, 5, 6,
	//	1, 6, 2,

	//	// bottom face
	//	4, 0, 3,
	//	4, 3, 7
	//};

	//D3D11_BUFFER_DESC ibd;
	//ibd.Usage = D3D11_USAGE_IMMUTABLE;
	//ibd.ByteWidth = sizeof(UINT) * 36;
	//ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//ibd.CPUAccessFlags = 0;
	//ibd.MiscFlags = 0;
	//ibd.StructureByteStride = 0;
	//D3D11_SUBRESOURCE_DATA iinitData;
	//iinitData.pSysMem = indices;
	//HR(md3dDevice->CreateBuffer(&ibd, &iinitData, mBoxIB.GetAddressOf()));



	//GeometryGenerator::MeshData grid;
	//GeometryGenerator geoGen;
	//geoGen.CreateGrid(160.0, 160.0, 50, 50, grid);

	//mIndexCount_grid = grid.Indices.size();
	//std::vector<Vertex::PosColor> vertices_grid(grid.Vertices.size());
	//for (size_t i = 0; i < grid.Vertices.size(); ++i) {
	//	XMFLOAT3 p = grid.Vertices[i].Position;
	//	p.y = GetHeight(p.x, p.z);
	//	vertices_grid[i].Pos = p;
	//	if (p.y < -10.0f)
	//	{
	//		// Sandy beach color.
	//		vertices_grid[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
	//	}
	//	else if (p.y < 5.0f)
	//	{
	//		// Light yellow-green.
	//		vertices_grid[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	//	}
	//	else if (p.y < 12.0f)
	//	{
	//		// Dark yellow-green.
	//		vertices_grid[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
	//	}
	//	else if (p.y < 20.0f)
	//	{
	//		// Dark brown.
	//		vertices_grid[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
	//	}
	//	else
	//	{
	//		// White snow.
	//		vertices_grid[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//	}
	//}
	//D3D11_BUFFER_DESC vbd_grid;
	//vbd_grid.Usage = D3D11_USAGE_IMMUTABLE;
	//vbd_grid.ByteWidth = sizeof(Vertex::PosColor) * grid.Vertices.size();
	//vbd_grid.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vbd_grid.CPUAccessFlags = 0;
	//vbd_grid.MiscFlags = 0;
	//D3D11_SUBRESOURCE_DATA vinitData_grid;
	//vinitData_grid.pSysMem = &vertices_grid[0];
	//HR(md3dDevice->CreateBuffer(&vbd_grid, &vinitData_grid, mGridVB.GetAddressOf()));
	//

	//D3D11_BUFFER_DESC ibd_grid;
	//ibd_grid.Usage = D3D11_USAGE_IMMUTABLE;
	//ibd_grid.ByteWidth = sizeof(UINT) * mIndexCount_grid;
	//ibd_grid.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//ibd_grid.CPUAccessFlags = 0;
	//ibd_grid.MiscFlags = 0;
	//D3D11_SUBRESOURCE_DATA iinitData_grid;

	//iinitData_grid.pSysMem = &grid.Indices[0]; /*첫번째 요소의 포인터*/
	//HR(md3dDevice->CreateBuffer(&ibd_grid, &iinitData_grid, mGridIB.GetAddressOf()));

	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	/*vertices 
		box ->grid ->shpere -> cylinder 순서로 넣는다.
	*/

	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = box.Vertices.size() + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;
	
	
	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;


	std::vector<Vertex::PosColor> vertices(totalVertexCount);
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	/* 각각 meshdata 에 있는 버텍스들 vertices 에 차례대로 모두 넣는다 .*/

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

	std::vector<UINT> indices; 
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, mVB.GetAddressOf()));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));



}

float GetHeight(int x, int z) {

	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}


void GeoApp::BuildVertexLayout() {
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	/*float3 한개 float4 한개*/
	D3DX11_PASS_DESC passDesc;

	Effects::ColorFX->ColorTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, mInputLayout.GetAddressOf()));

}



void GeoApp::BuildLandandWaves() {
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();
	std::vector<Vertex::PosColor> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;

		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mGridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));



	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;

	HR(md3dDevice->CreateBuffer(&vbd, 0, mWavesVB.GetAddressOf()));

	std::vector<UINT> indices2(3 * mWaves.TriangleCount());

	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();

	int k = 0;
	for (UINT i = 0; i < m - 1; ++i) {
		for (DWORD j = 0; j < n - 1; j++) {
			indices2[k] = i * n + j;
			indices2[k + 1] = i * n + j + 1;
			indices2[k + 2] = (i + 1) * n + j;

			indices2[k + 3] = (i + 1) * n + j;
			indices2[k + 4] = i * n + j + 1;
			indices2[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}
	 ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices2.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	 iinitData;
	iinitData.pSysMem = &indices2[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));



}