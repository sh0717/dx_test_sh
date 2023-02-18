#include "LightHelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    
    float gFogStart;
    float gFogRange;
    float4 gFogColor;
    
    float gHeightScale;
    float gMaxTessDistance;
    float gMinTessDistance;
    float gMinTessFactor;
    float gMaxTessFactor;
    
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gViewProj;
    float4x4 gWolrdViewProj;
    float4x4 gTexTransform;
    Material gMaterial;
    
 
};

Texture2D gDiffuseMap;
Texture2D gNormalMap;
TextureCube gCubeMap;
Texture2D gShadowMap;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};
 


struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    float3 TangentL : TANGENT;
    
};


struct VertexOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
    float TessFactor : TESS;
};


/*VS 에서 tess factor 를 정해준다 . 거리에 따라서 '
*/

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3) gWorld);
    
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    float d = distance(vout.PosW, gEyePosW);
    
    float tess = saturate((gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance));
    
    vout.TessFactor = gMinTessFactor + tess * (gMaxTessFactor - gMinTessFactor);
    return vout;
    
}


struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
    
};



PatchTess PatchHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    pt.EdgeTess[0] = 0.5f * (patch[1].TessFactor + patch[2].TessFactor);
    pt.EdgeTess[1] = 0.5f * (patch[2].TessFactor + patch[0].TessFactor);
    pt.EdgeTess[2] = 0.5f * (patch[0].TessFactor + patch[1].TessFactor);
    pt.InsideTess = pt.EdgeTess[0];
	
    return pt;
   
    
}


struct HullOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
};

/*HS 에서 하는일은 말이죠
patch 를 보고 어떻게 자를지 얼만큼 자를지 결정해주는 겁니다. 
patchhs  함수를 이용해서 
지금까지 배운거에서는 hout= vertexout 이 아닌게 업삳.
*/

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<VertexOut,3> p, uint i : SV_OutputControlPointID,
uint patchId : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosW = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.TangentW = p[i].TangentW;
    hout.Tex = p[i].Tex;
    
    return hout;
}


/*
DS 가 보통의 VS 같은 역할을 한다. 

PosW 가 필요한 이유는 light 나 등등 계산할 떄 

*/

struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
};

[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;
    
    dout.PosW = bary.x * tri[0].PosW + bary.y * tri[1].PosW + bary.z * tri[2].PosW;
    dout.NormalW = bary.x * tri[0].NormalW + bary.y * tri[1].NormalW + bary.z * tri[2].NormalW;
    dout.TangentW = bary.x * tri[0].TangentW + bary.y * tri[1].TangentW + bary.z * tri[2].TangentW;
    dout.Tex = bary.x * tri[0].Tex + bary.y * tri[1].Tex + bary.z * tri[2].Tex;
    dout.NormalW = normalize(dout.NormalW);
    
    
    
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
	
	// Sample height map (stored in alpha channel).
    float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;
	
	// Offset vertex along normal.
    dout.PosW += (gHeightScale * (h - 1.0)) * dout.NormalW;
	
	// Project to homogeneous clip space.
    dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	
    return dout;
    
    

}




float4 PS(DomainOut pin, uniform int gLightCount,
    uniform bool gUseTexture, uniform bool gAlphaClip, uniform bool gFogEnable
, uniform bool gReflectEnable
) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    float3 toEye = gEyePosW - pin.PosW;
    float distToEye = length(toEye);
    toEye /= distToEye;
    
    float4 texColor = float4(1, 1, 1, 1);
    
    if(gUseTexture)
    {
		// Sample texture.
        texColor = gDiffuseMap.Sample(samLinear, pin.Tex);

        if(gAlphaClip)
        {
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
            clip(texColor.a - 0.05f);
        }
    }
    
    float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
    float3 realNormal = NormalMapExtract(normalMapSample, pin.NormalW, pin.TangentW);
    
    
    float4 litColor = texColor;
    if(gLightCount > 0)
    {
		// Start with a sum of zero. 
        float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

        
        /*if there is no shadow  shadow factor is 1 and full shadow is => shadow factor 0 */
        
        float3 shadow = float3(1.0f, 1.0f, 1.0f);
       // shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);
        
		// Sum the light contribution from each light source.  
		[unroll]
        for(int i = 0; i < gLightCount; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], realNormal, toEye,
				A, D, S);

            ambient += A; /*ambient 는 shadow 영향 xx  */
            diffuse += shadow[i] * D;
            spec += shadow[i] * S;
        }

        litColor = texColor * (ambient + diffuse) + spec;

        if(gReflectEnable)
        {
            float3 incident = -toEye;
            float3 reflectionVector = reflect(incident, realNormal);
            float4 reflectionColor = gCubeMap.Sample(samLinear, reflectionVector);

            litColor += gMaterial.Reflect * reflectionColor;
        }
    }
 
    
    
    
    if(gFogEnable)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// Blend the fog color and the lit color.
        litColor = lerp(litColor, gFogColor, fogLerp);
    }

	// diffuse alpha 와 texture 의 alpha 에서 color.a 결정 
    litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
    
};


technique11 Light1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false, false)));
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
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false, false)));
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
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false, false)));
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, false, false)));
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, false, false)));
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, false, false)));
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, false, false)));
    }
}


technique11 Light1TexAlpha
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, false, false)));
    }
}

technique11 Light2TexAlpha
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, false, false)));
    }
}

technique11 Light3TexAlpha
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, false, false)));
    }
}


technique11 Light1TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, true, false)));
    }
}

technique11 Light2TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, true, false)));
    }
}

technique11 Light3TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, true, false)));
    }
}


technique11 Light1TexAlphaFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, true, false)));
    }
}

technique11 Light2TexAlphaFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, true, false)));
    }
}

technique11 Light3TexAlphaFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, true, false)));
    }
}



technique11 Light1Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false, true)));
    }
}

technique11 Light2Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false, true)));
    }
}

technique11 Light3Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false, true)));
    }
}
