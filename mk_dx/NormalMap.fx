
#include "LightHelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    
    float gFogStart;
    float gFogRange;
    float4 gFogColor;
    
    
};