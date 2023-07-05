
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
    float3 pos = 2.0 * vin.PosL - float3(1.0,1.0,1.0);
    pos = pos.xyz * float3(1, 1, 1);
    vout.PosH = mul(float4(pos, 1.0f),gWorldViewProj);
    
    vout.PosW = pos;
    vout.Tex = vin.PosL;
    return vout;
	// Transform to world space space.
    
}
 
float4 PS(VertexOut pin) : SV_Target
{

    //return float4(1.0, 0.0, 0.0, 1.0);
    float absorption = 5.0;
    float num = 10;
    
    float3 lightpos = float3(10, 10, 10);
    
    
    float3 ratio = float3(1, 1, 1);
    float maxDist = length(ratio);
    
    
    float scale = maxDist / num;
    float lscale = scale;
    float3 pos = pin.Tex;
    float3 wpos = pin.PosW;
    float3 eyeDir = normalize(pin.PosW - gEyePosW) * scale;
    
    
    //float density = gGridMap.Sample(samAnisotropic, pos).r;G
    //return float4(density, 0.0, 0.0, 0.3);
    
    float Trans = 1.0;
    float3 LO = float3(0.0, 0.0, 0.0);
    
    //[unroll(64)]
    for(int i = 0; i < num; ++i)
    {
        float density = gGridMap.Sample(samAnisotropic, pos).r;
        
       
        if(density > 0.0)
        {
            Trans *= (1.0 - density * scale * absorption);
            if(Trans <= 0.001)
            {
                break;
            }
            float3 lightDir = normalize(lightpos-wpos)*lscale;

            float TL = 1.0;
            float3 lpos = pos + lightDir;
            
            for(int j = 0; j < num; j++)
            {
                float ld = gGridMap.Sample(samAnisotropic, lpos).r;
                TL *= (1.0 - absorption * lscale * ld);

                if(TL <= 0.001)
                {  
                    break;
                }
                lpos += lightDir;    
            }
            float3 Li = TL * float3(2,1,2);
            LO += Li * ( Trans) * density * scale;
        }
        pos += eyeDir;
        wpos += eyeDir;
        
    }
    
    return float4(LO.rgb , 1.0-Trans);

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
