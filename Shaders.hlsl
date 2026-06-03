// Waht comes in (comes from the Vertex Shader)
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


PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.position = float4(input.position, 1.0f); // (X, Y, Z, W)
    result.color = input.color;
    
    return result;
}

// Pixel Shader -> Runs for every pixel
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}