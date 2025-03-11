// DX9 Pixel Shader - HLSL version with 4 textures
sampler2D Tex0 : register(s0);
sampler2D Tex1 : register(s1);
sampler2D Tex2 : register(s2);
sampler2D Tex3 : register(s3);

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    float4 Diffuse  : COLOR0; // Diffuse color; its alpha is used as blend factor
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample all four textures
    float4 color0 = tex2D(Tex0, input.TexCoord);
    float4 color1 = tex2D(Tex1, input.TexCoord1);
    float4 color2 = tex2D(Tex2, input.TexCoord);
    float4 color3 = tex2D(Tex3, input.TexCoord);
    
    // Perform linear interpolation between texture1 and texture0 using diffuse alpha.
    // This matches the DX8 "lrp r0, v0.a, t1, t0" instruction:
    // blended = (1 - input.Diffuse.a) * color1 + input.Diffuse.a * color0
    float4 blended = lerp(color1, color0, input.Diffuse.a);
    
    // Apply diffuse lighting
    float4 lit = blended * input.Diffuse;

    // Modulate with texture 2 and texture 3 sequentially
    float4 modulated = lit * color2;
    return modulated;
}
