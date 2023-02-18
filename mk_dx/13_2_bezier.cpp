
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
	void BuildQuadPatchBuffer();
private:
	ID3D11Buffer* mQuadPatchVB;

	CubeModel* msky;

	gCamera mCamera;

	
	RenderOptions mRenderOption;

	
	
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
: D3DApp(hInstance),mCamera(),msky(nullptr)
{
	
	
	mCamera.SetPosition(0.0f, 20.0f, -25.0f);
	mMainWndCaption = L"TexturedHillsAndWaves Demo";

	mRenderOption = RenderOptions::Texture;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	
	




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

	msky = new CubeModel(md3dDevice, L"../Textures/grasscube1024.dds", 5000.0f);

	 BuildQuadPatchBuffer();



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



	
	mCamera.Update();
}

void TexturedHillsAndWavesApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	
	msky->Draw(md3dImmediateContext, mCamera);


	Effects::BezierFX->SetEyePosW(mCamera.GetPosition());
	Effects::BezierFX->SetFogColor(Colors::Silver);
	Effects::BezierFX->SetFogStart(15.0f);
	Effects::BezierFX->SetFogRange(175.0f);



	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32.Get());
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
 //

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
	ID3DX11EffectTechnique* activeTech = Effects::BezierFX->TessTech;

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = mCamera.view();
	XMMATRIX proj = mCamera.proj();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;

	Effects::BezierFX->SetWorld(world);
	Effects::BezierFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::BezierFX->SetWorldViewProj(worldViewProj);
	Effects::BezierFX->SetTexTransform(XMMatrixIdentity());
	Effects::BezierFX->SetDiffuseMap(0);

	activeTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
	RenderStates::WireOn(md3dImmediateContext);
	md3dImmediateContext->Draw(16, 0);
	RenderStates::WireOff(md3dImmediateContext);
	//
	md3dImmediateContext->HSSetShader(0, 0, 0);
	md3dImmediateContext->DSSetShader(0, 0, 0);

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

void TexturedHillsAndWavesApp::BuildQuadPatchBuffer() {
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 16;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	Vertex::Basic32 vertices[16];

	vertices[0].Pos = XMFLOAT3(-10.0f, -10.0f, +15.0f);
	vertices[1].Pos = XMFLOAT3(-5.0f, 0.0f, +15.0f);
	vertices[2].Pos = XMFLOAT3(+5.0f, 0.0f, +15.0f);
	vertices[3].Pos = XMFLOAT3(+10.0f, 0.0f, +15.0f);
	vertices[4].Pos = XMFLOAT3(-15.0f, 0.0f, +5.0f);
	vertices[5].Pos = XMFLOAT3(-5.0f, 0.0f, +5.0f);
	vertices[6].Pos = XMFLOAT3(+5.0f, 20.0f, +5.0f);
	vertices[7].Pos = XMFLOAT3(+15.0f, 0.0f, +5.0f);
	vertices[8].Pos = XMFLOAT3(-15.0f, 0.0f, -5.0f);
	vertices[9].Pos = XMFLOAT3(-5.0f, 0.0f, -5.0f);
	vertices[10].Pos = XMFLOAT3(+5.0f, 0.0f, -5.0f);
	vertices[11].Pos = XMFLOAT3(+15.0f, 0.0f, -5.0f);
	vertices[12].Pos = XMFLOAT3(-10.0f, 10.0f, -15.0f);
	vertices[13].Pos = XMFLOAT3(-5.0f, 0.0f, -15.0f);
	vertices[14].Pos = XMFLOAT3(+5.0f, 0.0f, -15.0f);
	vertices[15].Pos = XMFLOAT3(+25.0f, 10.0f, -15.0f);



	D3D11_SUBRESOURCE_DATA viniData;
	viniData.pSysMem = vertices;

	HR(md3dDevice->CreateBuffer(&vbd, &viniData, &mQuadPatchVB));


}