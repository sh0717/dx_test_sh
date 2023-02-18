

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
	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildCrateGeometryBuffers();



	void BuildTreeSpritesBuffer();
	void DrawTreeSprites(CXMMATRIX viewProj);
private:
	NormalMapModel mCylinderModel1;


	CubeModel* msky;

	gCamera mCamera;

	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;

	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11Buffer* mMarbleVB;
	ID3D11Buffer* mMarbleIB;



	ID3D11Buffer* mTreeSpritesVB;

	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWavesMapSRV;
	ID3D11ShaderResourceView* mBoxMapSRV;
	ID3D11ShaderResourceView* mTreeTextureMapArraySRV;


	Waves mWaves;

	DirectionalLight mDirLights[3];
	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;
	Material mTreeMat;
	Material mMarbeMat;


	XMFLOAT4X4 mGrassTexTransform;
	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;

	

	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mMarbleWorld;
	
	UINT mLandIndexCount;
	XMFLOAT2 mWaterTexOffset;
	UINT mMarbleIndexCount;
	

	
	RenderOptions mRenderOption;

	bool mAlphaToCoverage;

	static const UINT TreeCount = 16;
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
: D3DApp(hInstance), mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0), mGrassMapSRV(0), mWavesMapSRV(0), mWaterTexOffset(0.0f, 0.0f),
  mLandIndexCount(0), mAlphaToCoverage(false),mCamera(),msky(nullptr)
{
	
	
	mCamera.SetPosition(0.0f, 20.0f, -25.0f);
	mMainWndCaption = L"TexturedHillsAndWaves Demo";

	mRenderOption = RenderOptions::Texture;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, XMMatrixScaling(10.0, 1.0, 10.0));
	XMStoreFloat4x4(&mWavesWorld, XMMatrixScaling(10.0,1.0,10.0));
	

	XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&mBoxWorld, boxScale * boxOffset);


	XMMATRIX marbleScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX marbleOffset = XMMatrixTranslation(8.0f, 25.0f, -15.0f);
	XMStoreFloat4x4(&mMarbleWorld, marbleScale* marbleOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

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

	mLandMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWavesMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mMarbeMat.Ambient = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mMarbeMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mMarbeMat.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);
	mMarbeMat.Reflect = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	mTreeMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mTreeMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

}

TexturedHillsAndWavesApp::~TexturedHillsAndWavesApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTreeSpritesVB);
	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	ReleaseCOM(mBoxMapSRV);
	ReleaseCOM(mTreeTextureMapArraySRV);
	Effects::Shutdown();
	InputLayouts::Shutdown();
	RenderStates::Shutdown();

}

bool TexturedHillsAndWavesApp::Init()
{
	if(!D3DApp::Init())
		return false;

	mWaves.Init(160, 160, 1.0, 0.03f, 3.25f, 0.4f);

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::Initialize(md3dDevice);
	InputLayouts::Initialize(md3dDevice);
	RenderStates::Initialize(md3dDevice);

	msky = new CubeModel(md3dDevice, L"../Textures/grasscube1024.dds", 5000.0f);

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, 
		L"../Textures/grass.dds", 0, 0, &mGrassMapSRV, 0 ));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, 
		L"../Textures/water1.dds", 0, 0, &mWavesMapSRV, 0 ));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"../Textures/WireFence.dds", 0, 0, &mBoxMapSRV, 0));


	vector<wstring> trees;
	
	trees.push_back(L"../Textures/tree0.dds");
	trees.push_back(L"../Textures/tree1.dds");
	trees.push_back(L"../Textures/tree2.dds");
	trees.push_back(L"../Textures/tree3.dds");
	mTreeTextureMapArraySRV = d3dHelper::CreateTexture2DArraySRV(
		md3dDevice, md3dImmediateContext, trees, DXGI_FORMAT_R8G8B8A8_UNORM);

	mCylinderModel1.Initialize(md3dDevice,Geometry::CYLINDER,mBoxMat,XMMatrixIdentity(),L"../Textures/bricks.dds", L"../Textures/bricks_nmap.dds");
	XMMATRIX scale = XMMatrixScaling(15, 15, 15);
	XMMATRIX translate = XMMatrixTranslation(0.0f, 10.0f, 0.0f);
	mCylinderModel1.SetWorld(scale * translate);
	



	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildCrateGeometryBuffers();
	BuildTreeSpritesBuffer();


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
	static float t_base = 0.0f;
	if( (mTimer.TotalTime() - t_base) >= 0.25f )
	{
		t_base += 0.25f;
 
		DWORD i = 5 + rand() % (mWaves.RowCount()-10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount()-10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//
	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for(UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos    = mWaves[i];
		v[i].Normal = mWaves.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x  = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y  = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	md3dImmediateContext->Unmap(mWavesVB, 0);

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);

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

	
	msky->Draw(md3dImmediateContext, mCamera);

	//DrawTreeSprites(mCamera.viewProj());

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
 
	XMMATRIX view = mCamera.view();
	XMMATRIX proj = mCamera.proj();
	XMMATRIX viewProj = view*proj;
	XMFLOAT3 eyePos = mCamera.GetPosition();
	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(eyePos);
	Effects::BasicFX->SetFogColor(Colors::Red);
	Effects::BasicFX->SetFogStart(15.0f);
	Effects::BasicFX->SetFogRange(1750.0f);

	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(eyePos);
	Effects::NormalMapFX->SetFogColor(Colors::Red);
	Effects::NormalMapFX->SetFogStart(15.0f);
	Effects::NormalMapFX->SetFogRange(1750.0f);






	ID3DX11EffectTechnique* boxTech= Effects::BasicFX->Light3TexTech;
	ID3DX11EffectTechnique* landAndWavesTech= Effects::BasicFX->Light3TexTech;


	ID3DX11EffectTechnique* marbleTech = Effects::BasicFX->Light3ReflectTech;


	switch (mRenderOption) {
		case RenderOptions::Light:
			boxTech = Effects::BasicFX->Light3Tech;
			landAndWavesTech = Effects::BasicFX->Light3Tech;
			break;
		case RenderOptions::Texture:
			boxTech = Effects::BasicFX->Light3TexTechAlphaclip;
			landAndWavesTech = Effects::BasicFX->Light3TexTech;
			break;
		case RenderOptions::TextureAndFog:
			boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
			landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
			break;
	}

    D3DX11_TECHNIQUE_DESC techDesc;
	boxTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mBoxMapSRV);

		RenderStates::NoCullOn(md3dImmediateContext);
		boxTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(36, 0, 0);

		// Restore default render state.
		RenderStates::NoCullOff(md3dImmediateContext);
	}

	/* draw marble 
	
	*/
	marbleTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mMarbleVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mMarbleIB, DXGI_FORMAT_R32_UINT, 0);
	
		XMMATRIX world = XMLoadFloat4x4(&mMarbleWorld);
		
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		
		Effects::BasicFX->SetMaterial(mMarbeMat);
		Effects::BasicFX->SetCubeMap(msky->GetSRV());
		marbleTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mMarbleIndexCount, 0, 0);

	}

	
	landAndWavesTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		//
		// Draw the hills.
		//
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
		Effects::BasicFX->SetMaterial(mLandMat);
		Effects::BasicFX->SetDiffuseMap(mGrassMapSRV);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

		//
		// Draw the waves.
		//
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		world = XMLoadFloat4x4(&mWavesWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
		Effects::BasicFX->SetMaterial(mWavesMat);
		Effects::BasicFX->SetDiffuseMap(mWavesMapSRV);

		RenderStates::TransparentOn(md3dImmediateContext);
		landAndWavesTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(3*mWaves.TriangleCount(), 0, 0);
		RenderStates::TransparentOff(md3dImmediateContext);
		
    }


	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNorTexTan);


	ID3DX11EffectTechnique* cyltech = Effects::NormalMapFX->Light3TexTech;

	ID3D11Buffer* vb = mCylinderModel1.GetVB();
	ID3D11Buffer* ib = mCylinderModel1.GetIB();
	int sex = mCylinderModel1.GetIndexCount();

	UINT s = mCylinderModel1.GetStride();
	UINT o = 0;
	md3dImmediateContext->IASetVertexBuffers(0,1,&vb,&s,&o);
	md3dImmediateContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);



	XMMATRIX world = mCylinderModel1.GetWorld();
	
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;

	Effects::NormalMapFX->SetWorld(world);
	Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
	Effects::NormalMapFX->SetTexTransform(XMMatrixIdentity());
	Effects::NormalMapFX->SetMaterial(mBoxMat);
	Effects::NormalMapFX->SetDiffuseMap(mCylinderModel1.GetDiffuse());
	Effects::NormalMapFX->SetNormalMap(mCylinderModel1.GetNormal());

	cyltech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
	md3dImmediateContext->DrawIndexed(mCylinderModel1.GetIndexCount(), 0, 0);


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

float TexturedHillsAndWavesApp::GetHillHeight(float x, float z)const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

XMFLOAT3 TexturedHillsAndWavesApp::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));
	
	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void TexturedHillsAndWavesApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
 
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex::Basic32> vertices(grid.Vertices.size());
	for(size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);
		
		vertices[i].Pos    = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex    = grid.Vertices[i].TexC;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
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
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));
}

void TexturedHillsAndWavesApp::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3*mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for(UINT i = 0; i < m-1; ++i)
	{
		for(DWORD j = 0; j < n-1; ++j)
		{
			indices[k]   = i*n+j;
			indices[k+1] = i*n+j+1;
			indices[k+2] = (i+1)*n+j;

			indices[k+3] = (i+1)*n+j;
			indices[k+4] = i*n+j+1;
			indices[k+5] = (i+1)*n+j+1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));




}


void TexturedHillsAndWavesApp::BuildCrateGeometryBuffers() {
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData marble;
	GeometryGenerator gen;

	gen.CreateBox(1.0, 1.0, 1.0, box);
	gen.CreateSphere(0.5f, 200, 200, marble);



	std::vector<Vertex::Basic32> vertices(box.Vertices.size());

	for (UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * box.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * box.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &box.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));

	mMarbleIndexCount = marble.Indices.size();


	std::vector<Vertex::Basic32> verticesOfMarble(marble.Vertices.size());


	for (UINT i = 0; i < marble.Vertices.size(); ++i) {
		verticesOfMarble[i].Pos = marble.Vertices[i].Position;
		verticesOfMarble[i].Normal = marble.Vertices[i].Normal;
		verticesOfMarble[i].Tex = marble.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd2;
	vbd2.Usage = D3D11_USAGE_IMMUTABLE;
	vbd2.ByteWidth = sizeof(Vertex::Basic32) * marble.Vertices.size();
	vbd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd2.CPUAccessFlags = 0;
	vbd2.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData2;
	vinitData2.pSysMem = &verticesOfMarble[0];
	HR(md3dDevice->CreateBuffer(&vbd2, &vinitData2, &mMarbleVB));


	D3D11_BUFFER_DESC ibd2;
	ibd2.Usage = D3D11_USAGE_IMMUTABLE;
	ibd2.ByteWidth = sizeof(UINT) * marble.Indices.size();
	ibd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd2.CPUAccessFlags = 0;
	ibd2.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData2;


	iinitData2.pSysMem = &marble.Indices[0];

	
	HR(md3dDevice->CreateBuffer(&ibd2, &iinitData2, &mMarbleIB));

}

void TexturedHillsAndWavesApp::BuildTreeSpritesBuffer() {

	
	Vertex::Tree32 v[TreeCount];

	for (UINT i = 0; i < TreeCount; ++i) {
	
		float x = MathHelper::RandF(-35.0f, 35.0f);
		float z = MathHelper::RandF(-35.0f, 35.0f);
		float y = GetHillHeight(x, z);

		// Move tree slightly above land height.
		y += 10.0f;

		v[i].Pos = XMFLOAT3(x, y, z);
		v[i].Size = XMFLOAT2(24.0f, 24.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Tree32) * TreeCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mTreeSpritesVB));

}

void TexturedHillsAndWavesApp::DrawTreeSprites(CXMMATRIX viewProj) {

	Effects::TreeFX->SetDirLights(mDirLights);
	Effects::TreeFX->SetEyePosW(mCamera.GetPosition());
	Effects::TreeFX->SetFogColor(Colors::Silver);
	Effects::TreeFX->SetFogStart(15.0f);
	Effects::TreeFX->SetFogRange(175.0f);
	Effects::TreeFX->SetViewProj(viewProj);
	Effects::TreeFX->SetMaterial(mTreeMat);
	Effects::TreeFX->SetTreeTextureMapArray(mTreeTextureMapArraySRV);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(InputLayouts::Tree32);
	UINT stride = sizeof(Vertex::Tree32);
	UINT offset = 0;

	ID3DX11EffectTechnique* treeTech= Effects::TreeFX->Light3Tech;

	switch (mRenderOption)
	{
	case RenderOptions::Light:
		treeTech = Effects::TreeFX->Light3Tech;
		break;
	case RenderOptions::Texture:
		treeTech = Effects::TreeFX->Light3TexAlphaClipTech;
		break;
	case RenderOptions::TextureAndFog:
		treeTech = Effects::TreeFX->Light3TexAlphaClipFogTech;
		break;

	}

	D3DX11_TECHNIQUE_DESC techDesc;
	treeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mTreeSpritesVB, &stride, &offset);

		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		if (mAlphaToCoverage)
		{
			RenderStates::AlphaToCoverageOn(md3dImmediateContext);
		}
		treeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->Draw(TreeCount, 0);

		if (mAlphaToCoverage)
		{
			RenderStates::AlphaToCoverageOff(md3dImmediateContext);
		}
	}

}