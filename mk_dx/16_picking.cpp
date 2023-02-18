

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
	UINT Pick(int sx, int sy,  BasicModel& model);


	
private:
	BasicModel mCar;
	Material mCarMat;


	UINT mPickedTriangle = -1;

	std::vector<InstancedData> mInstancedData;
	UINT mVisibleObjectCount;

	CubeModel* msky;

	gCamera mCamera;


	



	XMFLOAT4X4 mSkullWorld;
	UINT mSkullIndexCount;
	XMFLOAT3 mSkullTranslation;

	XNA::AxisAlignedBox mSkullBox;
	XNA::Frustum mCamFrustum;

	




	DirectionalLight mDirLights[3];
	
	Material mSkullMat;
	Material mPickedTriangleMat;
	
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

	mCarMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mCarMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mCarMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	mPickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
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



	XMMATRIX scale = XMMatrixScaling(10.0, 10.0, 10.0);
	mCar.Initialize(md3dDevice, L"../Models/car.txt", mCarMat, scale, nullptr);


	



	

	

	return true;
}

void TexturedHillsAndWavesApp::OnResize()
{
	D3DApp::OnResize();
	mCamera.InitLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	
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


	


	mCamera.Update();






}

void TexturedHillsAndWavesApp::DrawScene()
{	
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	if (mPickedTriangle != -1) {
		msky->Draw(md3dImmediateContext, mCamera);
	}
	
	
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;


	XMMATRIX view = mCamera.view();
	XMMATRIX proj = mCamera.proj();
	XMMATRIX viewProj = mCamera.viewProj();

	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCamera.GetPosition());
	ID3DX11EffectTechnique* activeMeshTech = Effects::BasicFX->Light3Tech;

	if (GetAsyncKeyState('1') & 0x8000) {
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);
	}
	ID3D11Buffer* vb = mCar.GetVB();
	md3dImmediateContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mCar.GetIB(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = mCar.GetWorld();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;

	Effects::BasicFX->SetWorld(world);
	Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::BasicFX->SetWorldViewProj(worldViewProj);
	Effects::BasicFX->SetMaterial(mCarMat);

	activeMeshTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
	md3dImmediateContext->DrawIndexed(mCar.GetIndexCount(), 0, 0);

	// Restore default
	md3dImmediateContext->RSSetState(0);

	if (mPickedTriangle != -1) {
		md3dImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);
		Effects::BasicFX->SetMaterial(mPickedTriangleMat);
		activeMeshTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);
		md3dImmediateContext->OMSetDepthStencilState(0, 0);
	}


	

	HR(mSwapChain->Present(0, 0));
}

void TexturedHillsAndWavesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		mPickedTriangle=Pick(x, y,mCar);
		
	}
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





UINT TexturedHillsAndWavesApp::Pick(int sx, int sy, BasicModel& model) {
	XMMATRIX P = mCamera.proj();

	float vx = ((2.0f*sx/mClientWidth)  - 1.0f)/P(0,0);
	float vy = ((-2.0f*sy/mClientHeight) + 1.0f)/P(1,1);

	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);
	XMMATRIX V = mCamera.view();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	XMMATRIX W = model.GetWorld();
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX ToLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, ToLocal);
	rayDir = XMVector3TransformNormal(rayDir, ToLocal);

	rayDir = XMVector3Normalize(rayDir);

	UINT mPickedTriangle = -1;
	float tmin = 0.0f;

	XNA::AxisAlignedBox box = model.GetBox();


	if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &box, &tmin)) {
		
		tmin = MathHelper::Infinity;
		std::vector<UINT>& Indices = model.GetIndices();
		std::vector<Vertex::Basic32>& Vertices = model.GetVertices();

		for (UINT i = 0; i < Indices.size() / 3; ++i) {
			UINT i0 = Indices[i * 3 + 0];
			UINT i1 = Indices[i * 3 + 1];
			UINT i2 = Indices[i * 3 + 2];

			XMVECTOR v0 = XMLoadFloat3(&Vertices[i0].Pos);
			XMVECTOR v1 = XMLoadFloat3(&Vertices[i1].Pos);
			XMVECTOR v2 = XMLoadFloat3(&Vertices[i2].Pos);

			float t = 0.0f;

			if (XNA::IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &t))
			{
				if (t < tmin)
				{
					// This is the new nearest picked triangle.
					tmin = t;
					mPickedTriangle = i;
				}
			}
		}

		

	}

	return mPickedTriangle;

}