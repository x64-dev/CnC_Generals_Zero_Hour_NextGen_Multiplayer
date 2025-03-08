// Direct3D 9 HLSL Pixel Shader ported from Direct8 assembly shader
// Command & Conquer Generals(tm) terrain shader with dynamic fade and color modulation

sampler2D Texture0 : register(s0);

float3 FilterVector : register(c0);
float3 FilterColor : register(c1);
float BlendFactor : register(c2);

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample base texture
    float4 texColor = tex2D(Texture0, input.TexCoord);

    // Convert texture to black & white using dot product
    float luminance = dot(texColor.rgb, FilterVector);

    // Modulate luminance with filter color
    float3 modulatedColor = luminance * FilterColor;

    // Smooth blend modulated color into original texture
    float3 finalColor = lerp(texColor.rgb, modulatedColor, BlendFactor);

    return float4(finalColor, texColor.a);
}
