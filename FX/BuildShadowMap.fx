cbuffer cbPerFrame
{
    float3 gEyePosW;
    
    
};
cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gViewProj;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
   
};


Texture2D gDiffuseMap;
Texture2D gNormalMap;



struct VertexIn
{
    float3 PosL : POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD;
    
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
    
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}


RasterizerState Depth
{
	// [From MSDN]
	// If the depth buffer currently bound to the output-merger stage has a UNORM format or
	// no depth buffer is bound the bias value is calculated like this: 
	//
	// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
	//
	// where r is the minimum representable value > 0 in the depth-buffer format converted to float32.
	// [/End MSDN]
	// 
	// For a 24-bit depth buffer, r = 1 / 2^24.
	//
	// Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006

	// You need to experiment with these values for your scene.
    DepthBias = 100000;
    DepthBiasClamp = 0.0f;
    SlopeScaledDepthBias = 1.0f;
};


technique11 BuildShadowMapTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(NULL);

        SetRasterizerState(Depth);
    }
}
