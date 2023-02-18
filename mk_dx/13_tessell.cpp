
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
	BasicModel skull;
	Material mSkullMat;
	CubeModel* msky;

	gCamera mCamera;
	
	
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
: D3DApp(hInstance),mCamera(),msky(nullptr)
{
	
	
	mCamera.SetPosition(0.0f, 20.0f, -25.0f);
	mMainWndCaption = L"TexturedHillsAndWaves Demo";

	mRenderOption = RenderOptions::Texture;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);



	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	




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


	
	 XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	 

	 skull.Initialize(md3dDevice, L"../Models/skull.txt", mSkullMat, skullScale, nullptr);

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


	Effects::TessellFX->SetEyePosW(mCamera.GetPosition());
	Effects::TessellFX->SetFogColor(Colors::Silver);
	Effects::TessellFX->SetFogStart(15.0f);
	Effects::TessellFX->SetFogRange(175.0f);



	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
 

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
	ID3DX11EffectTechnique* activeTech = Effects::TessellFX->TessTech;

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = mCamera.view();
	XMMATRIX proj = mCamera.proj();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;

	Effects::TessellFX->SetWorld(world);
	Effects::TessellFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::TessellFX->SetWorldViewProj(worldViewProj);
	Effects::TessellFX->SetTexTransform(XMMatrixIdentity());
	Effects::TessellFX->SetDiffuseMap(0);


	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		
		RenderStates::WireOn(md3dImmediateContext);
		md3dImmediateContext->Draw(4, 0);
		RenderStates::WireOff(md3dImmediateContext);
		
	}
	
	

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	activeTech = Effects::BasicFX->Light3Tech;
	{
		ID3D11Buffer* vb = skull.GetVB();
		ID3D11Buffer* ib = skull.GetIB();
		md3dImmediateContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		world = skull.GetWorld();


		XMMATRIX view = mCamera.view();
		XMMATRIX proj = mCamera.proj();
		XMMATRIX viewProj = mCamera.viewProj();
		XMFLOAT3 eyePos = mCamera.GetPosition();
		Effects::BasicFX->SetDirLights(mDirLights);
		Effects::BasicFX->SetEyePosW(eyePos);
		Effects::BasicFX->SetFogColor(Colors::Red);
		Effects::BasicFX->SetFogStart(15.0f);
		Effects::BasicFX->SetFogRange(1750.0f);


		XMMATRIX world = skull.GetWorld();
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetViewProj(viewProj);
		Effects::BasicFX->SetWorldViewProj(world*viewProj);

		Effects::BasicFX->SetMaterial(mSkullMat);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		


		activeTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(skull.GetIndexCount(), 0, 0);
	}

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
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	Vertex::Basic32 vertices[4];

	vertices[0].Pos = XMFLOAT3(-10.0, 0.0, 10.0);
	vertices[1].Pos = XMFLOAT3(+10.0, 0.0, 10.0);
	vertices[2].Pos = XMFLOAT3(-10.0, 0.0, -10.0);
	vertices[3].Pos = XMFLOAT3(+10.0, 0.0, -10.0);


	D3D11_SUBRESOURCE_DATA viniData;
	viniData.pSysMem = vertices;

	HR(md3dDevice->CreateBuffer(&vbd, &viniData, &mQuadPatchVB));


}