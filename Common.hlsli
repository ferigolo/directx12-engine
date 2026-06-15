// What comes in (comes into the Vertex Shader)
struct VertexIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
};

// Vertex Shader -> Rasterizer -> Pixel Shader
struct VertexOut
{
    float4 positionH : SV_POSITION; // Posição no Ecrã (2D)
    float3 positionW : POSITION;    // Posição no Mundo (3D)
    float3 normalW   : NORMAL;      // Normal no Mundo (3D)
    float4 color     : COLOR;
};

cbuffer EntityConstants : register(b0)
{
    float4x4 wvp; // WVP = World * View * Projection
    float4x4 wn;  // Normal for 3D world
};

cbuffer cbPass : register(b1)
{
    float3 gEyePosW;        // Camera position
    float cbPerObjectPad1;
    float4 gAmbientLight;   // Ambient light strenght
    float3 gLightDir;       // Light direction -> L
    float cbPerObjectPad2;
    float4 gLightColor;     // Global light color
};