#include "Common.hlsli"

float4 PS(VertexOut pin) : SV_Target
{
    pin.normalW = normalize(pin.normalW);
    float3 L = normalize(-gLightDir);
    float3 V = normalize(gEyePosW - pin.positionW);
    
    float4 ambient = gAmbientLight * pin.color;
    float diffuseFactor = max(dot(pin.normalW, L), 0.0f);
    float4 diffuse = diffuseFactor * gLightColor * pin.color;
    
    float3 H = normalize(L + V);
    float specFactor = pow(max(dot(pin.normalW, H), 0.0f), 32.0f);
    if (diffuseFactor <= 0.0f)
        specFactor = 0.0f;
    float4 specular = specFactor * gLightColor;
    
    float4 finalColor = ambient + diffuse + specular;
    finalColor.a = pin.color.a;
    
    return finalColor;
}