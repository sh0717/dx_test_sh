//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dUtil.h"
using namespace std;


#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const WCHAR* filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};
#pragma endregion



class ColorEffect : public Effect {

public:
	ColorEffect(ID3D11Device* device, const WCHAR* filename);
	~ColorEffect();

	void SetWorldViewProj(CXMMATRIX M) {
		WolrdViewProj->SetMatrix(reinterpret_cast<const float*>(&M));
	}

	ID3DX11EffectTechnique* ColorTech;
	ID3DX11EffectMatrixVariable* WolrdViewProj;




};


class NormalMapEffect : public Effect {

public:
	NormalMapEffect(ID3D11Device* device, const WCHAR* filename);
	~NormalMapEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowTransform(CXMMATRIX M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }



	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetNormalMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }
	void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }

	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

	ID3DX11EffectTechnique* Light0TexTech;
	ID3DX11EffectTechnique* Light1TexTech;
	ID3DX11EffectTechnique* Light2TexTech;
	ID3DX11EffectTechnique* Light3TexTech;


	ID3DX11EffectTechnique* Light1TexTechAlphaclip;
	ID3DX11EffectTechnique* Light2TexTechAlphaclip;
	ID3DX11EffectTechnique* Light3TexTechAlphaclip;


	ID3DX11EffectTechnique* Light1TexFogTech;
	ID3DX11EffectTechnique* Light2TexFogTech;
	ID3DX11EffectTechnique* Light3TexFogTech;


	ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;
	/// <summary>
	///  이 밑으로는 reflect of 환경 반영
	/// </summary>
	ID3DX11EffectTechnique* Light1ReflectTech;
	ID3DX11EffectTechnique* Light2ReflectTech;
	ID3DX11EffectTechnique* Light3ReflectTech;

	/// <summary>
	/// //
	/// </summary>
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* ViewProj;



	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* ShadowTransform;


	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;



	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;


};

class DisplacementMapEffect : public  NormalMapEffect {
	
public: 
	DisplacementMapEffect(ID3D11Device* device, const WCHAR* filename);
	~DisplacementMapEffect();

	ID3DX11EffectScalarVariable* HeightScale;
	ID3DX11EffectScalarVariable* MaxTessDistance;
	ID3DX11EffectScalarVariable* MinTessDistance;
	ID3DX11EffectScalarVariable* MaxTessFactor;
	ID3DX11EffectScalarVariable* MinTessFactor;
	


	void SetHeightScale(float f) { HeightScale->SetFloat(f); }
	void SetMaxTessDistance(float f) { MaxTessDistance->SetFloat(f); }
	void SetMinTessDistance(float f) { MinTessDistance->SetFloat(f); }
	void SetMaxTessFactor(float f) { MaxTessFactor->SetFloat(f); }
	void SetMinTessFactor(float f) { MinTessFactor->SetFloat(f); }

};




#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const WCHAR* filename);
	~BasicEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetShadowTransform(CXMMATRIX M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }



	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }

	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

	ID3DX11EffectTechnique* Light0TexTech;
	ID3DX11EffectTechnique* Light1TexTech;
	ID3DX11EffectTechnique* Light2TexTech;
	ID3DX11EffectTechnique* Light3TexTech;


	ID3DX11EffectTechnique* Light1TexTechAlphaclip;
	ID3DX11EffectTechnique* Light2TexTechAlphaclip;
	ID3DX11EffectTechnique* Light3TexTechAlphaclip;


	ID3DX11EffectTechnique* Light1TexFogTech;
	ID3DX11EffectTechnique* Light2TexFogTech;
	ID3DX11EffectTechnique* Light3TexFogTech;
	

	ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;
	/// <summary>
	///  이 밑으로는 reflect of 환경 반영
	/// </summary>
	ID3DX11EffectTechnique* Light1ReflectTech;
	ID3DX11EffectTechnique* Light2ReflectTech;
	ID3DX11EffectTechnique* Light3ReflectTech;

	/// <summary>
	/// //
	/// </summary>
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;

	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* ShadowTransform;


	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;



	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

};
#pragma endregion

#pragma region BuildShadowMapEffect
class BuildShadowMapEffect : public Effect {

public:
	BuildShadowMapEffect(ID3D11Device* device, const WCHAR* filename);
	~BuildShadowMapEffect();
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }



	ID3DX11EffectTechnique* BuildShadowMapTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* TexTransform;
};





#pragma endregion



#pragma region TreeEffect 

class TreeEffect :public Effect
{
public:
	TreeEffect(ID3D11Device* device, const WCHAR* filename);
	~TreeEffect();


	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetTreeTextureMapArray(ID3D11ShaderResourceView* tex) { TreeTextureMapArray->SetResource(tex); }




	ID3DX11EffectTechnique* Light3Tech;
	ID3DX11EffectTechnique* Light3TexAlphaClipTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;



	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectShaderResourceVariable* TreeTextureMapArray;
};



#pragma endregion

#pragma region TestEffect
class TestEffect :public Effect {
public:
	TestEffect(ID3D11Device* device, const WCHAR* filename);
	~TestEffect();
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetGridMap(ID3D11ShaderResourceView* gridmap) { GridMap->SetResource(gridmap); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	ID3DX11EffectTechnique* Testtech;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectShaderResourceVariable* GridMap;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectVectorVariable* EyePosW;


};
#pragma endregion

#pragma region CubeEffect
class CubeEffect:public Effect {
public:
	CubeEffect(ID3D11Device* device, const WCHAR* filename);
	~CubeEffect();
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetCubeMap(ID3D11ShaderResourceView* cubemap) { CubeMap->SetResource(cubemap); }

	ID3DX11EffectTechnique* SkyTech;
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3DX11EffectMatrixVariable* WorldViewProj;

};
#pragma endregion


class ParticleEffect :public Effect {
public:
	ParticleEffect(ID3D11Device* device, const WCHAR* filename);
	~ParticleEffect();
	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetGameTime(float f) { GameTime->SetFloat(f); }
	void SetTimeStep(float f) { TimeStep->SetFloat(f); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEmitPosW(const XMFLOAT3& v) { EmitPosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEmitDirW(const XMFLOAT3& v) { EmitDirW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	void SetTexArray(ID3D11ShaderResourceView* tex) { TexArray->SetResource(tex); }
	void SetRandomTex(ID3D11ShaderResourceView* tex) { RandomTex->SetResource(tex); }


	ID3DX11EffectTechnique* StreamOutTech;
	ID3DX11EffectTechnique* DrawTech;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectScalarVariable* GameTime;
	ID3DX11EffectScalarVariable* TimeStep;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* EmitPosW;
	ID3DX11EffectVectorVariable* EmitDirW;
	ID3DX11EffectShaderResourceVariable* TexArray;
	ID3DX11EffectShaderResourceVariable* RandomTex;
};

class TessellationEffect : public Effect {
public:
	TessellationEffect(ID3D11Device* device, const WCHAR* filename);
	~TessellationEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }


	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }


	ID3DX11EffectTechnique* TessTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;

	ID3DX11EffectMatrixVariable* TexTransform;

	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;



	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;





};

#pragma region TerrainEffect
class TerrainEffect : public Effect {
public:
	TerrainEffect(ID3D11Device* device, const WCHAR* filename);
	~TerrainEffect();

	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }

	void SetMinDist(float f) { MinDist->SetFloat(f); }
	void SetMaxDist(float f) { MaxDist->SetFloat(f); }
	void SetMinTess(float f) { MinTess->SetFloat(f); }
	void SetMaxTess(float f) { MaxTess->SetFloat(f); }
	void SetTexelCellSpaceU(float f) { TexelCellSpaceU->SetFloat(f); }
	void SetTexelCellSpaceV(float f) { TexelCellSpaceV->SetFloat(f); }
	void SetWorldCellSpace(float f) { WorldCellSpace->SetFloat(f); }
	void SetWorldFrustumPlanes(XMFLOAT4 planes[6]) { WorldFrustumPlanes->SetFloatVectorArray(reinterpret_cast<float*>(planes), 0, 6); }

	void SetLayerMapArray(ID3D11ShaderResourceView* tex) { LayerMapArray->SetResource(tex); }
	void SetBlendMap(ID3D11ShaderResourceView* tex) { BlendMap->SetResource(tex); }
	void SetHeightMap(ID3D11ShaderResourceView* tex) { HeightMap->SetResource(tex); }


	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;
	ID3DX11EffectTechnique* Light1FogTech;
	ID3DX11EffectTechnique* Light2FogTech;
	ID3DX11EffectTechnique* Light3FogTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectScalarVariable* MinDist;
	ID3DX11EffectScalarVariable* MaxDist;
	ID3DX11EffectScalarVariable* MinTess;
	ID3DX11EffectScalarVariable* MaxTess;
	ID3DX11EffectScalarVariable* TexelCellSpaceU;
	ID3DX11EffectScalarVariable* TexelCellSpaceV;
	ID3DX11EffectScalarVariable* WorldCellSpace;
	ID3DX11EffectVectorVariable* WorldFrustumPlanes;

	ID3DX11EffectShaderResourceVariable* LayerMapArray;
	ID3DX11EffectShaderResourceVariable* BlendMap;
	ID3DX11EffectShaderResourceVariable* HeightMap;

};


#pragma endregion

class DebugTextureEffect : public Effect {
public:
	DebugTextureEffect(ID3D11Device* device, const WCHAR* filename);
	~DebugTextureEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexture(ID3D11ShaderResourceView* tex) { Texture->SetResource(tex); }

	ID3DX11EffectTechnique* ViewRGBATech;
	ID3DX11EffectTechnique* ViewRRRRTech;
	ID3DX11EffectTechnique* ViewGGGGTech;
	ID3DX11EffectTechnique* ViewBBBBTech;
	ID3DX11EffectTechnique* ViewAAAATech;


	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectShaderResourceVariable* Texture;
};




#pragma region Effects
class Effects
{
public:
	static void Initialize(ID3D11Device* device);
	static void Shutdown();


	static unique_ptr<ColorEffect> ColorFX;
	static unique_ptr<BasicEffect> BasicFX;
	
	static unique_ptr<BasicEffect> InstanceBasicFX;
	static  unique_ptr < NormalMapEffect> NormalMapFX;
	static  unique_ptr < DisplacementMapEffect> DisplacementMapFX;
	static  unique_ptr < TreeEffect> TreeFX;
	static  unique_ptr < CubeEffect> CubeFX;

	static  unique_ptr<TestEffect> TestFX;
	static  unique_ptr<TestEffect> Test2FX;
	static  unique_ptr < ParticleEffect> FireFX;
	static  unique_ptr < ParticleEffect> RainFX;

	static  unique_ptr < TessellationEffect> TessellFX;
	static  unique_ptr < TessellationEffect> BezierFX;

	static  unique_ptr < BuildShadowMapEffect> ShadowFX;

	static  unique_ptr < TerrainEffect> TerrainFX;

	static  unique_ptr < DebugTextureEffect> DebugFX;

};
#pragma endregion

#endif // EFFECTS_H