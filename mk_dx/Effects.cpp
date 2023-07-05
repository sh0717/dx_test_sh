//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const WCHAR* filename)
	: mFX(0)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(filename, 0, 0, 0, "fx_5_0", shaderFlags,
		0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs can store errors or warnings.
	/*if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}*/

	// Even if there are no compilationMsgs, check to make sure there were no other errors.
	if (FAILED(hr))
	{
		DXTrace((const WCHAR*)__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, device, &mFX));

	// Done with compiled shader.
	ReleaseCOM(compiledShader);

}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}
#pragma endregion

ColorEffect::ColorEffect(ID3D11Device* device, const WCHAR* filename)
	:Effect(device,filename)
{
	ColorTech = mFX->GetTechniqueByName("ColorTech");
	WolrdViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();

}
ColorEffect::~ColorEffect() {}


#pragma region NormalMapEffect
NormalMapEffect::NormalMapEffect(ID3D11Device* device, const WCHAR* filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");
	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");
	Light1TexTechAlphaclip = mFX->GetTechniqueByName("Light1TexAlpha");
	Light2TexTechAlphaclip = mFX->GetTechniqueByName("Light2TexAlpha");
	Light3TexTechAlphaclip = mFX->GetTechniqueByName("Light3TexAlpha");

	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaFog");


	Light1ReflectTech = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech = mFX->GetTechniqueByName("Light3Reflect");


	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	ShadowTransform = mFX->GetVariableByName("gShadowTransform")->AsMatrix();

	
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");

	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	ShadowMap = mFX->GetVariableByName("gShadowMap")->AsShaderResource();

	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
}

NormalMapEffect::~NormalMapEffect()
{

}
#pragma endregion

DisplacementMapEffect::DisplacementMapEffect(ID3D11Device* device, const WCHAR* filename)
	:NormalMapEffect(device, filename)
{
	HeightScale = mFX->GetVariableByName("gHeightScale")->AsScalar();
	MaxTessDistance = mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance = mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	MaxTessFactor = mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
	MinTessFactor = mFX->GetVariableByName("gMinTessFactor")->AsScalar();

}
DisplacementMapEffect::~DisplacementMapEffect() {}



BasicEffect::BasicEffect(ID3D11Device* device, const WCHAR* filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");
	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");
	Light1TexTechAlphaclip = mFX->GetTechniqueByName("Light1TexAlpha");
	Light2TexTechAlphaclip = mFX->GetTechniqueByName("Light2TexAlpha");
	Light3TexTechAlphaclip = mFX->GetTechniqueByName("Light3TexAlpha");

	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaFog");


	Light1ReflectTech = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech = mFX->GetTechniqueByName("Light3Reflect");


	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	ShadowTransform = mFX->GetVariableByName("gShadowTransform")->AsMatrix();

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();

	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");

	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	ShadowMap = mFX->GetVariableByName("gShadowMap")->AsShaderResource();

	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
}

BasicEffect::~BasicEffect()
{
}

BuildShadowMapEffect::BuildShadowMapEffect(ID3D11Device* device, const WCHAR* filename)
	:Effect(device,filename)
{

	BuildShadowMapTech = mFX->GetTechniqueByName("BuildShadowMapTech");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();

}

BuildShadowMapEffect::~BuildShadowMapEffect() {}



#pragma region TreeEffect

TreeEffect::TreeEffect(ID3D11Device* device, const WCHAR* filename)
	:Effect(device,filename)
{
	Light3Tech = mFX->GetTechniqueByName("Light3");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	TreeTextureMapArray = mFX->GetVariableByName("gTreeMapArray")->AsShaderResource();
}
TreeEffect::~TreeEffect()
{
}

#pragma endregion


#pragma  region TestEffect
TestEffect::TestEffect(ID3D11Device* device, const WCHAR* filename) :Effect(device, filename) {
	Testtech = mFX->GetTechniqueByName("Testtech");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	GridMap = mFX->GetVariableByName("gGridMap")->AsShaderResource();

	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = mFX->GetVariableByName("gDirLights");
};

TestEffect::~TestEffect() {}
#pragma endregion


#pragma region CubeEffect

CubeEffect::CubeEffect(ID3D11Device* device, const WCHAR* filename) :Effect(device, filename) {
	SkyTech = mFX->GetTechniqueByName("SkyTech");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();

};

CubeEffect::~CubeEffect() {}
#pragma endregion

#pragma region ParticleEffect
ParticleEffect::ParticleEffect(ID3D11Device* device, const WCHAR* filename)
	:Effect(device,filename)
{
	StreamOutTech = mFX->GetTechniqueByName("StreamOutTech");
	DrawTech = mFX->GetTechniqueByName("DrawTech");


	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	GameTime = mFX->GetVariableByName("gGameTime")->AsScalar();
	TimeStep = mFX->GetVariableByName("gTimeStep")->AsScalar();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	EmitPosW = mFX->GetVariableByName("gEmitPosW")->AsVector();
	EmitDirW = mFX->GetVariableByName("gEmitDirW")->AsVector();

	TexArray = mFX->GetVariableByName("gTexArray")->AsShaderResource();
	RandomTex = mFX->GetVariableByName("gRandomTex")->AsShaderResource();


}

ParticleEffect::~ParticleEffect() {}

#pragma endregion


TessellationEffect::TessellationEffect(ID3D11Device* device, const WCHAR* filename)
	: Effect(device, filename)
{
	
	TessTech = mFX->GetTechniqueByName("Tess");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");

	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	

	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
}

TessellationEffect::~TessellationEffect()
{
}
#pragma region TerrainEffect
TerrainEffect::TerrainEffect(ID3D11Device* device, const WCHAR* filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");
	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");

	MinDist = mFX->GetVariableByName("gMinDist")->AsScalar();
	MaxDist = mFX->GetVariableByName("gMaxDist")->AsScalar();
	MinTess = mFX->GetVariableByName("gMinTess")->AsScalar();
	MaxTess = mFX->GetVariableByName("gMaxTess")->AsScalar();
	TexelCellSpaceU = mFX->GetVariableByName("gTexelCellSpaceU")->AsScalar();
	TexelCellSpaceV = mFX->GetVariableByName("gTexelCellSpaceV")->AsScalar();
	WorldCellSpace = mFX->GetVariableByName("gWorldCellSpace")->AsScalar();
	WorldFrustumPlanes = mFX->GetVariableByName("gWorldFrustumPlanes")->AsVector();

	LayerMapArray = mFX->GetVariableByName("gLayerMapArray")->AsShaderResource();
	BlendMap = mFX->GetVariableByName("gBlendMap")->AsShaderResource();
	HeightMap = mFX->GetVariableByName("gHeightMap")->AsShaderResource();
}

TerrainEffect::~TerrainEffect()
{
}
#pragma endregion

DebugTextureEffect::DebugTextureEffect(ID3D11Device* device, const WCHAR* filename) 
	:Effect(device, filename)
{
	ViewRGBATech = mFX->GetTechniqueByName("ViewArgbTech");
	ViewRRRRTech = mFX->GetTechniqueByName("ViewRedTech");
	ViewGGGGTech = mFX->GetTechniqueByName("ViewGreenTech");
	ViewBBBBTech = mFX->GetTechniqueByName("ViewBlueTech");
	ViewAAAATech = mFX->GetTechniqueByName("ViewAlphaTech");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	Texture = mFX->GetVariableByName("gTexture")->AsShaderResource();
}

DebugTextureEffect::~DebugTextureEffect() {}





#pragma region Effects

unique_ptr < BasicEffect> Effects::BasicFX = nullptr;
unique_ptr < ColorEffect> Effects::ColorFX = nullptr;
unique_ptr < BasicEffect> Effects::InstanceBasicFX = nullptr;


unique_ptr < TreeEffect> Effects::TreeFX = nullptr;
unique_ptr < CubeEffect> Effects::CubeFX = nullptr;
unique_ptr < TestEffect> Effects::TestFX = nullptr;
unique_ptr < TestEffect> Effects::Test2FX = nullptr;
unique_ptr < NormalMapEffect> Effects::NormalMapFX = nullptr;
unique_ptr < DisplacementMapEffect> Effects::DisplacementMapFX = nullptr;


unique_ptr < ParticleEffect> Effects::FireFX = nullptr;
unique_ptr < ParticleEffect> Effects::RainFX = nullptr;
unique_ptr < TessellationEffect> Effects::TessellFX = nullptr;
unique_ptr < TessellationEffect> Effects::BezierFX = nullptr;

unique_ptr < BuildShadowMapEffect> Effects::ShadowFX = nullptr;
unique_ptr < TerrainEffect> Effects::TerrainFX = nullptr;
unique_ptr < DebugTextureEffect> Effects::DebugFX = nullptr;


void Effects::Initialize(ID3D11Device* device)
{
	ColorFX = make_unique<ColorEffect>(device, L"../FX/color.fx");
	BasicFX = make_unique< BasicEffect>(device, L"../FX/Basic.fx");
	NormalMapFX = make_unique < NormalMapEffect>(device, L"../FX/NormalMap.fx");
	DisplacementMapFX = make_unique < DisplacementMapEffect>(device, L"../FX/DisplacementMap.fx");
	TreeFX = make_unique < TreeEffect>(device, L"../FX/TreeSprite.fx");
	CubeFX = make_unique < CubeEffect>(device, L"../FX/CubeMap.fx");
	TestFX = make_unique<TestEffect>(device, L"../FX/test.fx");
	Test2FX = make_unique<TestEffect>(device, L"../FX/test2.fx");


	InstanceBasicFX = make_unique < BasicEffect>(device, L"../FX/InstanceBasic.fx");
	FireFX = make_unique < ParticleEffect>(device, L"../FX/Fire.fx");
	RainFX = make_unique < ParticleEffect>(device, L"../FX/Rain.fx");
	TessellFX = make_unique < TessellationEffect>(device, L"../FX/tessel.fx");
	BezierFX = make_unique < TessellationEffect>(device, L"../FX/bezier.fx");
	ShadowFX = make_unique < BuildShadowMapEffect>(device, L"../FX/BuildShadowMap.fx");
	TerrainFX = make_unique < TerrainEffect>(device, L"../FX/Terrain2.fx");

	DebugFX = make_unique < DebugTextureEffect>(device, L"../FX/DebugTexture.fx");

}

void Effects::Shutdown()
{
}
#pragma endregion