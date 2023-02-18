#include "LightHelper.fx"

cbuffer cbPerFrame
{
    
    DirectionalLight gDirLights[3];
    float4x4 gShadowTransform;
    float3 gEyePosW;
    
    
    float gFogStart;
    float gFogRange;
    float4 gFogColor;
    
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gViewProj;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
    
    
    
    Material gMaterial;
};


Texture2D gDiffuseMap;
Texture2D gNormalMap;
Texture2D gShadowMap;

TextureCube gCubeMap;

SamplerState samLinear
{
    Filter=MIN_MAG_MIP_LINEAR;
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
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

SamplerComparisonState samShadow
{
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    AddressW = BORDER;
    BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ComparisonFunc = LESS_EQUAL;
};



VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3) gWorld);
    
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    vout.ShadowPosH = mul(float4(vout.PosW, 1.0f), gShadowTransform);
    
    return vout;
    
}



float4 PS(VertexOut pin, uniform int gLightCount,
    uniform bool gUseTexture, uniform bool gAlphaClip, uniform bool gFogEnable
, uniform bool gReflectEnable
) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    float3 toEye = gEyePosW - pin.PosW;
    float distToEye = length(toEye);
    toEye /= distToEye;
    
    float4 texColor = float4(1, 1, 1, 1);
    
    if (gUseTexture)
    {
		// Sample texture.
        texColor = gDiffuseMap.Sample(samLinear, pin.Tex);

        if (gAlphaClip)
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
    if (gLightCount > 0)
    {
		// Start with a sum of zero. 
        float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

        
        /*if there is no shadow  shadow factor is 1 and full shadow is => shadow factor 0 */
        
        float3 shadow = float3(1.0f, 1.0f, 1.0f);
        shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);
        
		// Sum the light contribution from each light source.  
		[unroll]
        for (int i = 0; i < gLightCount; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], realNormal, toEye,
				A, D, S);

            ambient += A;/*ambient 는 shadow 영향 xx  */
            diffuse += shadow[i]*D;
            spec += shadow[i]*S;
        }

        litColor = texColor * (ambient + diffuse) + spec;

        if (gReflectEnable)
        {
            float3 incident = -toEye;
            float3 reflectionVector = reflect(incident, realNormal);
            float4 reflectionColor = gCubeMap.Sample(samLinear, reflectionVector);

            litColor += gMaterial.Reflect * reflectionColor;
        }
    }
 
    
    
    
    if (gFogEnable)
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
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false, false)));
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false, false)));
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false, false)));
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, false, false)));
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, false, false)));
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, false, false)));
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, false, false)));
    }
}


technique11 Light1TexAlpha
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, false, false)));
    }
}

technique11 Light2TexAlpha
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, false, false)));
    }
}

technique11 Light3TexAlpha
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, false, false)));
    }
}


technique11 Light1TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, true, false)));
    }
}

technique11 Light2TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, true, false)));
    }
}

technique11 Light3TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, true, false)));
    }
}


technique11 Light1TexAlphaFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, true, false)));
    }
}

technique11 Light2TexAlphaFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, true, false)));
    }
}

technique11 Light3TexAlphaFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, true, false)));
    }
}



technique11 Light1Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false, true)));
    }
}

technique11 Light2Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false, true)));
    }
}

technique11 Light3Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false, true)));
    }
}
