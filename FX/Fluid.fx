#include "LightHelper.fx"

cbuffer cbPerFrame
{
    float3 gEyePosW;
    float gGameTime;
    float4x4 gViewProj;
};

cbuffer cbPerObject
{
    Material gMaterial;   
};

cbuffer cbFixed
{
    float2 gTexC[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f)
    };
};

// Array of textures for texturing the particles.
Texture2DArray gTexArray;
// Random texture used to generate random numbers in shaders.
Texture1D gRandomTex;
 

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexIn
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
    
};

struct GeoOut
{
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
    

};



VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;
    return vout;
};

[maxvertexcount(4)]
void GS(point VertexOut gin[1], uint primID : SV_PrimitiveID, inout TriangleStream<GeoOut> tristream)
{
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f;
    look = normalize(look);
    float3 right = cross(up, look);
    
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);
    
    GeoOut gout;
    [unroll]
    for(int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.Tex = gTexC[i];
        //gout.PrimID = primID;
		
        tristream.Append(gout);
    }
}


float4 PS(GeoOut pin) : SV_Target
{
    return gTexArray.Sample(samLinear, float3(pin.Tex, 0));
    
}



technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
