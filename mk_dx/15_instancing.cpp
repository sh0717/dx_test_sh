

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
#include "BasicModel.h"

#include "xnacollision.h"
using namespace std;
enum RenderOptions {
	Light=1,
	Texture=2,
	TextureAndFog=3
};

bool mFtoggle = false;

struct InstancedData
{
	XMFLOAT4X4 World;
	XMFLOAT4 Color;
};
class TexturedHillsAndWavesApp : public D3DApp
{
public:
	TexturedHillsAndWavesApp(HINSTANCE hInstance);
	~TexturedHillsAndWavesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:


	void BuildSkullGeometryBuffers();
	void BuildInstancedBuffer();

	
	
private:
	BasicModel skull;

	std::vector<InstancedData> mInstancedData;
	UINT mVisibleObjectCount;

	CubeModel* msky;

	gCamera mCamera;


	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;
	ID3D11Buffer* mInstancedBuffer;


	XMFLOAT4X4 mSkullWorld;
	UINT mSkullIndexCount;
	XMFLOAT3 mSkullTranslation;

	XNA::AxisAlignedBox mSkullBox;
	XNA::Frustum mCamFrustum;

	




	DirectionalLight mDirLights[3];
	
	Material mSkullMat;

	
	RenderOptions mRenderOption;

	bool mAlphaToCoverage;
	bool mFrustumEnable;
	
	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	TexturedHillsAndWavesApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

TexturedHillsAndWavesApp::TexturedHillsAndWavesApp(HINSTANCE hInstance)
: D3DApp(hInstance), mAlphaToCoverage(false),mCamera(),msky(nullptr)
{
	
	
	mCamera.SetPosition(0.0f, 20.0f, -25.0f);
	mMainWndCaption = L"TexturedHillsAndWaves Demo";

	mRenderOption = RenderOptions::Texture;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;



	mDirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);



	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mSkullTranslation = XMFLOAT3(50.0, 30.0, 2.0);
}

TexturedHillsAndWavesApp::~TexturedHillsAndWavesApp()
{
	md3dImmediateContext->ClearState();




	Effects::Shutdown();
	InputLayouts::Shutdown();
	RenderStates::Shutdown();

}

bool TexturedHillsAndWavesApp::Init()
{
	if(!D3DApp::Init())
		return false;

	

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::Initialize(md3dDevice);
	InputLayouts::Initialize(md3dDevice);
	RenderStates::Initialize(md3dDevice);

	msky = new CubeModel(md3dDevice, L"../Textures/snowcube1024.dds", 5000.0f);




	XMMATRIX skullRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
	XMStoreFloat4x4(&mSkullWorld, skullScale  * skullOffset);

	skull.Initialize(md3dDevice, L"../Models/skull.txt",mSkullMat ,skullScale,nullptr);


	

	BuildSkullGeometryBuffers();
	BuildInstancedBuffer();
	return true;
}

void TexturedHillsAndWavesApp::OnResize()
{
	D3DApp::OnResize();
	mCamera.InitLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	ComputeFrustumFromProjection(&mCamFrustum, &mCamera.proj());
}

void TexturedHillsAndWavesApp::UpdateScene(float dt)
{
	static float time = 0;

	time += dt;
	if (time > 1.1f) {
		time = 0.0f;
		mFtoggle = false;
	}

	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOption = RenderOptions::Light;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOption = RenderOptions::Texture;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOption = RenderOptions::TextureAndFog;

	if (GetAsyncKeyState('O') & 0x8000)
		mAlphaToCoverage = true;

	if (GetAsyncKeyState('P') & 0x8000)
		mAlphaToCoverage = false;



	if (GetAsyncKeyState('W') & 0x8000)
		mCamera.Walk(50.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera.Walk(-50.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera.Strafe(-50.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera.Strafe(50.0f * dt);


	if (GetAsyncKeyState('F') & 0x8000) {
		if (mFtoggle == false) {
			mFrustumEnable = !mFrustumEnable;
			mFtoggle = true;
		}
	}
		


	mCamera.Update();


	mVisibleObjectCount = 0;
	if (mFrustumEnable) {
		XMVECTOR detView = XMMatrixDeterminant(mCamera.view());
		XMMATRIX invView = XMMatrixInverse(&detView, mCamera.view());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	
		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);
		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			XMMATRIX W = XMLoadFloat4x4(&mInstancedData[i].World);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

			// View space to the object's local space.
			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

			// Decompose the matrix into its individual parts.
			XMVECTOR scale;
			XMVECTOR rotQuat;
			XMVECTOR translation;
			XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

			// Transform the camera frustum from view space to the object's local space.
			XNA::Frustum localspaceFrustum;
			XNA::TransformFrustum(&localspaceFrustum, &mCamFrustum, XMVectorGetX(scale), rotQuat, translation);

			// Perform the box/frustum intersection test in local space.
			if (XNA::IntersectAxisAlignedBoxFrustum(&skull.GetBox(), &localspaceFrustum) != 0)
			{
				// Write the instance data to dynamic VB of the visible objects.
				dataView[mVisibleObjectCount++] = mInstancedData[i];
			}
		}

		md3dImmediateContext->Unmap(mInstancedBuffer, 0);

	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);
		int a = mInstancedData.size();
		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			dataView[mVisibleObjectCount++] = mInstancedData[i];
		}

		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
	}
	





}

void TexturedHillsAndWavesApp::DrawScene()
{	
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3DX11EffectTechnique* activeTech = Effects::InstanceBasicFX->Light3Tech;
	if(mVisibleObjectCount==125)
	msky->Draw(md3dImmediateContext, mCamera);
	{

		md3dImmediateContext->IASetInputLayout(InputLayouts::InstanceBasic32);
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		UINT stride[2] = { sizeof(Vertex::Basic32), sizeof(InstancedData) };
		UINT offset[2] = { 0,0 };
		

		ID3D11Buffer* vbs[2] = { skull.GetVB(), mInstancedBuffer};

		md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		md3dImmediateContext->IASetIndexBuffer(skull.GetIB(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX view = mCamera.view();
		XMMATRIX proj = mCamera.proj();
		XMMATRIX viewProj = mCamera.viewProj();
		XMFLOAT3 eyePos = mCamera.GetPosition();
		Effects::InstanceBasicFX->SetDirLights(mDirLights);
		Effects::InstanceBasicFX->SetEyePosW(eyePos);
		Effects::InstanceBasicFX->SetFogColor(Colors::Red);
		Effects::InstanceBasicFX->SetFogStart(15.0f);
		Effects::InstanceBasicFX->SetFogRange(1750.0f);


		XMMATRIX world = skull.GetWorld();
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

		Effects::InstanceBasicFX->SetWorld(world);
		Effects::InstanceBasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::InstanceBasicFX->SetViewProj(viewProj);
		Effects::InstanceBasicFX->SetMaterial(mSkullMat);

		D3DX11_TECHNIQUE_DESC techDesc;
		activeTech->GetDesc(&techDesc);
	
		
		activeTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexedInstanced(skull.GetIndexCount(), mVisibleObjectCount, 0, 0, 0);
	}

	//DrawTreeSprites(mCamera.viewProj());

	HR(mSwapChain->Present(0, 0));
}

void TexturedHillsAndWavesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void TexturedHillsAndWavesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TexturedHillsAndWavesApp::OnMouseMove(WPARAM btnState, int x, int y)
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



void TexturedHillsAndWavesApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("../Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex::Basic32> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}


void TexturedHillsAndWavesApp::BuildInstancedBuffer()
{
	const int n = 5;
	mInstancedData.resize(n * n * n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				// Position instanced along a 3D grid.
				mInstancedData[k * n * n + i * n + j].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				// Random color.
				mInstancedData[k * n * n + i * n + j].Color.x = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k * n * n + i * n + j].Color.y = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k * n * n + i * n + j].Color.z = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k * n * n + i * n + j].Color.w = 1.0f;
			}
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}