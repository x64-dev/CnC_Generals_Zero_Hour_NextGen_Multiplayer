// Direct3D 9 HLSL vertex shader ported from DirectX 8 assembly shader

// Uniform variables
float4x4 WorldViewProjectionMatrix : register(c0);     // Registers c0-c3
float4 TexProjScaleOffset          : register(c6);     // Texture scaling/offset (c6.xy = scale, c[6].zw = offset)

struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform vertex to clip space
    output.Position = mul(input.Position, WorldViewProjMatrix);

    // Perspective divide to get screen-space coordinates
    float invW = 1.0f / output.Position.w;
    float2 projCoords = output.Position.xy * invW;

    // Scale, flip, and offset texture coordinates
    output.TexCoord0 = input.TexCoord;
    output.TexCoord1 = projCoords * CV_TEXPROJ_0.xy + CV_TEXPROJ_0.zw;

    return output;
}
