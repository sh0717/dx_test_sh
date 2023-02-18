#include "LightHelper.fx"



cbuffer cbPerFrame
{
    
    DirectionalLight gDirLights[3];
    float3 gEyePosW;

    float gFogStart;
    float gFogRange;
    float4 gFogColor;
    
    
    float gMinDist;
    float gMaxDist;
    float gMinTess;
    float gMaxTess;
    
    float gTexelCellSpaceU;
    float gTexelCellSpaceV;
    float gWorldCellSpace;
    float2 gTexScale = 50.0f;
    
    
    
    
    
    float4 gWorldFrustumPlanes[6];
    
    
    
};


cbuffer cbPerObject
{
    float4x4 gViewProj;
    Material gMaterial;
};



Texture2DArray gLayerMapArray;
Texture2D gBlendMap;
Texture2D gHeightMap;





SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;

    AddressU = WRAP;
    AddressV = WRAP;
};

SamplerState samHeightmap
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;

    AddressU = CLAMP;
    AddressV = CLAMP;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
    
};

struct VertexOut
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
  
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    // no world matrix 
    vout.PosW = vin.PosL;
    
    vout.PosW.y = gHeightMap.SampleLevel(samHeightmap, vin.Tex, 0).r;
    
    vout.Tex = vin.Tex;
    vout.BoundsY = vin.BoundsY;

    return vout;
}


struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};



//clac tessfactor at point p 
//based on distance if it is far we dont need to tess strongly but if
//it is 

float CalcTessFactor(float3 p)
{
    float d = distance(p, gEyePosW);
    float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));
    float powFactor = lerp(gMaxTess, gMinTess, s);
    return pow(2, powFactor);
}

////// ax+by+cz+d=0
////// plabe
/////plane 뒤 에 있으면 true 반환한다.
//////그리고 여기 plane 의 xyz 벡터는 들어올때부터 normalize 되어있다. 
bool aabbPalneTest(float3 center, float3 extents, float4 plane)
{
    float3 n = abs(plane.xyz);
    float r = dot(extents, n);
    
    
    float center_dist = dot(float4(center, 1.0f), plane);
    
    return (center_dist + r) < 0.0f;

}

/*if out of frustum return true else return false  */
bool aabbOutsideFrustum(float3 center, float3 extents, float4 frustumPlanes[6])
{
    for (int i = 0; i < 6; i++)
    {
        if (aabbPalneTest(center, extents, frustumPlanes[i]))
        {
            return true;
        }
        
    }
    return false;
}

/*
0   1

2   3
patch vertex
*/
PatchTess ConstantHS(
    InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    float minY = patch[0].BoundsY.x;
    float maxY = patch[0].BoundsY.y;
    
    float3 vMin = float3(patch[2].PosW.x, minY, patch[2].PosW.z);
    float3 vMax = float3(patch[1].PosW.x, maxY, patch[1].PosW.z);
    
    float3 center = 0.5f*(vMin + vMax);
    float3 extents = 0.5f * (vMax - vMin);
    
    if (aabbOutsideFrustum(center, extents, gWorldFrustumPlanes))
    {
        pt.EdgeTess[0] = 0.0f;
        pt.EdgeTess[1] = 0.0f;
        pt.EdgeTess[2] = 0.0f;
        pt.EdgeTess[3] = 0.0f;
        
        pt.InsideTess[0] = 0.0f;
        pt.InsideTess[1] = 0.0f;
        return pt;
    }
    else
    {
        float3 e0 = 0.5f * (patch[0].PosW + patch[2].PosW);
        float3 e1 = 0.5f * (patch[0].PosW + patch[1].PosW);
        float3 e2 = 0.5f * (patch[1].PosW + patch[3].PosW);
        float3 e3 = 0.5f * (patch[2].PosW + patch[3].PosW);
        float3 c = 0.25f * (patch[0].PosW + patch[1].PosW + patch[2].PosW + patch[3].PosW);
        
        pt.EdgeTess[0] = CalcTessFactor(e0);
        pt.EdgeTess[1] = CalcTessFactor(e1);
        pt.EdgeTess[2] = CalcTessFactor(e2);
        pt.EdgeTess[3] = CalcTessFactor(e3);
        
        
        pt.InsideTess[0] = CalcTessFactor(c);
        pt.InsideTess[1] = pt.InsideTess[0];
	
        return pt;
        
    }
  
}


struct HullOut
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    
};


[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> patch, uint i : SV_OutputControlPointID, uint patchid : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosW = patch[i].PosW;
    hout.Tex = patch[i].Tex;
    
    return hout;
}


struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITIONT;
    float2 Tex : TEXCOORD0;
    float2 TiledTex : TEXCOORD1;
    
};

[domain("quad")]
DomainOut DS(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;
    
    dout.PosW = lerp(
        lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x), uv.y
    );
    
    dout.Tex = lerp(
		lerp(quad[0].Tex, quad[1].Tex, uv.x),
		lerp(quad[2].Tex, quad[3].Tex, uv.x),
		uv.y);
    
    dout.TiledTex = dout.Tex * gTexScale;
    
    dout.PosW.y = gHeightMap.SampleLevel(samHeightmap, dout.Tex, 0).r;
    
    
    dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	
    return dout;
    
}


float4 PS(DomainOut pin, 
          uniform int gLightCount, 
		  uniform bool gFogEnabled) : SV_Target
{
	//
	// Estimate normal and tangent using central differences.
	//
	float2 leftTex   = pin.Tex + float2(-gTexelCellSpaceU, 0.0f);
	float2 rightTex  = pin.Tex + float2(gTexelCellSpaceU, 0.0f);
	float2 bottomTex = pin.Tex + float2(0.0f, gTexelCellSpaceV);
	float2 topTex    = pin.Tex + float2(0.0f, -gTexelCellSpaceV);
	
	float leftY   = gHeightMap.SampleLevel( samHeightmap, leftTex, 0 ).r;
	float rightY  = gHeightMap.SampleLevel( samHeightmap, rightTex, 0 ).r;
	float bottomY = gHeightMap.SampleLevel( samHeightmap, bottomTex, 0 ).r;
	float topY    = gHeightMap.SampleLevel( samHeightmap, topTex, 0 ).r;
	
	float3 tangent = normalize(float3(2.0f*gWorldCellSpace, rightY - leftY, 0.0f));
	float3 bitan   = normalize(float3(0.0f, bottomY - topY, -2.0f*gWorldCellSpace)); 
	float3 normalW = cross(tangent, bitan);


	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
	//
	// Texturing
	//
	
	// Sample layers in texture array.
	float4 c0 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 0.0f) );
	float4 c1 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 1.0f) );
	float4 c2 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 2.0f) );
	float4 c3 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 3.0f) );
	float4 c4 = gLayerMapArray.Sample( samLinear, float3(pin.TiledTex, 4.0f) ); 
	
	// Sample the blend map.
	float4 t  = gBlendMap.Sample( samLinear, pin.Tex ); 
    
    // Blend the layers on top of each other.
    float4 texColor = c0;
    texColor = lerp(texColor, c1, t.r);
    texColor = lerp(texColor, c2, t.g);
    texColor = lerp(texColor, c3, t.b);
    texColor = lerp(texColor, c4, t.a);
 
	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{  
		// Start with a sum of zero. 
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], normalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		litColor = texColor*(ambient + diffuse) + spec;
	}
 
	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

    return litColor;
}


technique11 Light1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false)));
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false)));
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false)));
    }
}

technique11 Light1Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true)));
    }
}

technique11 Light2Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true)));
    }
}

technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true)));
    }
}
