#include "Common.hlsli"

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Calcula a posição 3D real e a direção da normal baseada na rotação do objeto
    vout.positionW = mul(float4(vin.position, 1.0f), wn).xyz;
    vout.normalW = mul(vin.normal, (float3x3) wn);
    
    // Posição final na tela para o Rasterizador
    vout.positionH = mul(float4(vin.position, 1.0f), wvp);
    
    vout.color = vin.color;
    return vout;
}