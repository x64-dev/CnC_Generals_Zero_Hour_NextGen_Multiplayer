// DX9 Pixel Shader - jmarshall - HLSL version
sampler2D Tex0 : register(s0);
sampler2D Tex1 : register(s1);
sampler2D Tex2 : register(s2);

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse  : COLOR0; // Diffuse color, using its alpha for blending
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample textures using the provided texture coordinates
    float4 color0 = tex2D(Tex0, input.TexCoord);
    float4 color1 = tex2D(Tex1, input.TexCoord);
    float4 color2 = tex2D(Tex2, input.TexCoord);
    
    // Blend between color1 and color0 using diffuse alpha:
    // Result = (1 - input.Diffuse.a) * color1 + input.Diffuse.a * color0
    float4 blended = lerp(color1, color0, input.Diffuse.a);
    
    // Apply diffuse lighting
    float4 lit = blended * input.Diffuse;
    
    // Modulate with texture 2
    return lit * color2;
}
