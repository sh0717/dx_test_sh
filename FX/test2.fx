
#include "LightHelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
   
};

cbuffer cbPerObject
{

    float4x4 gWorldViewProj;
    Material gMaterial;
}; 


Texture3D gGridMap;

struct VertexIn
{
    float3 PosL : POSITION;

};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
   
    float3 Tex : TEXCOORD0;
   
};


SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    //AddressU = WRAP;
    //AddressV = WRAP;
    
};



VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    float3 pos = 2.0 * vin.PosL - float3(1.0, 1.0, 1.0);
    pos = pos.xyz * float3(1, 1, 1);
    vout.PosH = mul(float4(pos, 1.0f), gWorldViewProj);
    
    vout.PosW = pos;
    vout.Tex = vin.PosL;
    return vout;
	// Transform to world space space.
    
}
 
float4 PS(VertexOut pin) : SV_Target
{

    return float4(1.0, 0.0, 0.0, 0.3);

}



technique11 Testtech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
