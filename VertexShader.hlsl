#include "Common.hlsli"

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.positionW = mul(float4(vin.position, 1.0f), wn).xyz;
    vout.normalW = mul(vin.normal, (float3x3) wn);

    vout.positionH = mul(float4(vin.position, 1.0f), wvp);
    
    vout.color = vin.color;
    return vout;
}