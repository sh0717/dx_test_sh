
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
#include "ShadowMap.h"
#include "Terrain.h"


using namespace std;
enum RenderOptions {
	Light=1,
	Texture=2,
	TextureAndFog=3
};
struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
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
	void RenderShadowMap();
	void BuildShadowTransform();
private:
	static const int SMapSize = 2048;
	ShadowMap* mSMap;
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	BoundingSphere mSceneBounds;


	NormalMapModel mBox;
	NormalMapModel mCylinder;
	NormalMapModel mGrid;


	BasicModel mSphere;
	BasicModel mSkull;

	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;

	CubeModel* msky;

	gCamera mCamera;
	bool WalkCamMode = false;

	Terrain mTerrain;
	
	RenderOptions mRenderOption;

	
	DirectionalLight mDirLights[3];
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
: D3DApp(hInstance),mCamera(),msky(nullptr),mSMap(nullptr)
{
	
	
	mCamera.SetPosition(0.0f, 2.0, 100.0f);
	mMainWndCaption = L"TexturedHillsAndWaves Demo";

	mRenderOption = RenderOptions::Texture;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;


	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);


	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 20.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}
	


	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	// Shadow acne gets worse as we increase the slope of the polygon (from the
	// perspective of the light).
	//mDirLights[0].Direction = XMFLOAT3(5.0f/sqrtf(50.0f), -5.0f/sqrtf(50.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(125.0f), -5.0f/sqrtf(125.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(116.0f), -4.0f/sqrtf(116.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(109.0f), -3.0f/sqrtf(109.0f), 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);



	mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mSkullMat.Reflect = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

TexturedHillsAndWavesApp::~TexturedHillsAndWavesApp()
{
	SafeDelete(msky);
	SafeDelete(mSMap);
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

	msky = new CubeModel(md3dDevice, L"../Textures/sunsetcube1024.dds", 5000.0f);
	mSMap = new ShadowMap(md3dDevice, SMapSize, SMapSize);

	
	Material SkullMat;
	SkullMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	SkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	SkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	SkullMat.Reflect = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkull.Initialize(md3dDevice, L"../Models/skull.txt", SkullMat, XMMatrixIdentity(), nullptr);
	mSphere.Initialize(md3dDevice, Geometry::SPHERE, mSphereMat, XMMatrixIdentity(), nullptr);


	mBox.Initialize(md3dDevice, Geometry::BOX, mBoxMat, XMMatrixIdentity(), L"../Textures/bricks.dds", L"../Textures/bricks_nmap.dds");
	mCylinder.Initialize(md3dDevice, Geometry::CYLINDER, mCylinderMat, XMMatrixIdentity(), L"../Textures/bricks.dds", L"../Textures/bricks_nmap.dds");
	mGrid.Initialize(md3dDevice, Geometry::GRID, mGridMat, XMMatrixIdentity(), L"../Textures/floor.dds", L"../Textures/floor_nmap.dds");




	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"../Textures/terrain.raw";
	tii.LayerMapFilename0 = L"../Textures/grass.dds";
	tii.LayerMapFilename1 = L"../Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"../Textures/stone.dds";
	tii.LayerMapFilename3 = L"../Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"../Textures/snow.dds";
	tii.BlendMapFilename = L"../Textures/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;
	mTerrain.Init(md3dDevice, md3dImmediateContext, tii);

	return true;
}

void TexturedHillsAndWavesApp::OnResize()
{
	D3DApp::OnResize();
	mCamera.InitLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	
}

void TexturedHillsAndWavesApp::UpdateScene(float dt)
{
	


	//
	// Every quarter second, generate a random wave.
	//


	//
	// Update the wave vertex buffer with the new solution.
	//
	static bool jump = false;
	static float jump_dist = 0.0f;

	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOption = RenderOptions::Light;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOption = RenderOptions::Texture;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOption = RenderOptions::TextureAndFog;

	

	if (GetAsyncKeyState('W') & 0x8000)
		mCamera.Walk(50.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera.Walk(-50.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera.Strafe(-50.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera.Strafe(50.0f * dt);

	if (GetAsyncKeyState('P') & 0x8000)
		WalkCamMode = true;
	if (GetAsyncKeyState('Q') & 0x8000)
		WalkCamMode = false;
	if (GetAsyncKeyState('G') & 0x8000) {
		jump = true; 
		jump_dist = 0.8f;
	}
	

	static float angle = 0.0001f*dt;
	


	XMMATRIX R = XMMatrixRotationY(angle);

	for (int i = 0; i < 3; i++) {
		XMVECTOR lightDir = XMLoadFloat3(&mDirLights[i].Direction);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mDirLights[i].Direction, lightDir);
	}

	if (WalkCamMode) {
	
		XMFLOAT3 pos = mCamera.GetPosition();
		float y = mTerrain.GetHeight(pos.x, pos.z);
		if (jump == false) {
			mCamera.SetPosition(pos.x, y + 1.8f, pos.z);
		}
		else {
			mCamera.SetPosition(pos.x, y +1.8f+ jump_dist, pos.z);
		}
	}

	BuildShadowTransform();
	mCamera.Update();
	if (jump) { jump_dist -= 0.001f; }
	
	if (jump_dist < 0&&jump) {
		jump = false;
		jump_dist = 0.6f;
	}
}

void TexturedHillsAndWavesApp::DrawScene()
{
	RenderShadowMap();


	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	


	msky->Draw(md3dImmediateContext, mCamera);

	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCamera.GetPosition());
	Effects::BasicFX->SetCubeMap(msky->GetSRV());
	Effects::BasicFX->SetShadowMap(mSMap->DepthMapSRV());
	Effects::BasicFX->SetShadowTransform(XMLoadFloat4x4(&mShadowTransform));

	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCamera.GetPosition());
	Effects::NormalMapFX->SetCubeMap(msky->GetSRV());
	Effects::NormalMapFX->SetShadowMap(mSMap->DepthMapSRV());
	Effects::NormalMapFX->SetShadowTransform(XMLoadFloat4x4(&mShadowTransform));
	
	mSkull.Render(md3dImmediateContext, Effects::BasicFX->Light3Tech, XMLoadFloat4x4(&mSkullWorld), XMMatrixIdentity(), mCamera);

	for (int i = 0; i < 10; ++i) {
		mCylinder.Render(md3dImmediateContext, Effects::NormalMapFX->Light3TexTech, XMLoadFloat4x4(&mCylWorld[i]), XMMatrixIdentity(),mCamera);
		mSphere.Render(md3dImmediateContext, Effects::BasicFX->Light3ReflectTech, XMLoadFloat4x4(&mSphereWorld[i]), XMMatrixIdentity(), mCamera);
	
	}
	mBox.Render(md3dImmediateContext, Effects::NormalMapFX->Light3TexTech, XMLoadFloat4x4(&mBoxWorld), XMMatrixIdentity(), mCamera);
	mGrid.Render(md3dImmediateContext, Effects::NormalMapFX->Light3TexTech, XMLoadFloat4x4(&mGridWorld), XMMatrixScaling(5.0f,5.0f,0.0f), mCamera);
	
	RenderStates::WireOn(md3dImmediateContext);
	//mTerrain.Draw(md3dImmediateContext, mCamera, mDirLights);
	RenderStates::WireOff(md3dImmediateContext);
	
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
void  TexturedHillsAndWavesApp::RenderShadowMap() {
	mSMap->BindDsv_SetNullRenderTarget(md3dImmediateContext);
	
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	
	ID3DX11EffectTechnique* smapTech = Effects::ShadowFX->BuildShadowMapTech;

	mSkull.RenderShadow(md3dImmediateContext, smapTech, XMLoadFloat4x4(&mSkullWorld), viewProj, XMMatrixIdentity());
	mBox.RenderShadow(md3dImmediateContext, smapTech, XMLoadFloat4x4(&mBoxWorld), viewProj, XMMatrixIdentity());
	mGrid.RenderShadow(md3dImmediateContext, smapTech, XMLoadFloat4x4(&mGridWorld), viewProj, XMMatrixIdentity());

	for (int i = 0; i < 10; ++i) {
		mCylinder.RenderShadow(md3dImmediateContext, smapTech, XMLoadFloat4x4(&mCylWorld[i]), viewProj, XMMatrixIdentity());
		mSphere.RenderShadow(md3dImmediateContext, smapTech, XMLoadFloat4x4(&mSphereWorld[i]), viewProj, XMMatrixIdentity());

	}


	md3dImmediateContext->RSSetState(0);

}

void  TexturedHillsAndWavesApp::BuildShadowTransform() {
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f * mSceneBounds.Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	
	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX S = V * P * T;
	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}