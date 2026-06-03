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

cbuffer TransformBuffer : register(b0)
{
    float xOffset; // Value the CPU will update to move the mesh in the X axis
};


PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.position = float4(input.position.x + xOffset, input.position.y, input.position.z, 1.0f); // (X, Y, Z, W)
    result.color = input.color;
    
    return result;
}

// Pixel Shader -> Runs for every pixel
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}