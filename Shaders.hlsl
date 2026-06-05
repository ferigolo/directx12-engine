// What comes in (comes from the Vertex Shader)
struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

// Vertex Shader -> Rasterizer -> Pixel Shader
struct PSInput
{
    float4 position : SV_POSITION; // SV = System Value
    float4 color : COLOR;
};

cbuffer EntiryConstants : register(b0)
{
    float4x4 wvp; // WVP = World * View * Projection
};

// Vertex Shader -> Runs for every vertex
PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.position = mul(float4(input.position, 1.0f), wvp); // (X, Y, Z, W) * WVP
    result.color = input.color;
    
    return result;
}

// Pixel Shader -> Runs for every pixel
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}