// Direct3D 9 HLSL Pixel Shader ported from Direct8 assembly shader
// Command & Conquer Generals(tm) terrain shader

sampler2D RoadTexture : register(s0);       // t0
sampler2D CloudShadowTexture : register(s1);// t1
sampler2D NoiseTexture : register(s2);      // t2

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse  : COLOR0;
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample textures
    float4 road = tex2D(RoadTexture, input.TexCoord);
    float4 cloudShadow = tex2D(CloudShadowTexture, input.TexCoord);
    float4 noise = tex2D(NoiseTexture, input.TexCoord);

    // Modulate textures
    float4 color = road * cloudShadow;
    color *= noise;

    // Apply diffuse lighting
    color *= input.Diffuse;

    return color;
}
