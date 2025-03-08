// DX9 Pixel Shader - jmarshall - HLSL version
sampler2D Tex0 : register(s0);
sampler2D Tex1 : register(s1);

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse  : COLOR0;  // Diffuse lighting, using alpha as blend factor
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample both textures using the interpolated texture coordinate
    float4 color0 = tex2D(Tex0, input.TexCoord);
    float4 color1 = tex2D(Tex1, input.TexCoord);
    
    // Linearly interpolate between the two textures using the alpha value
    float4 blended = lerp(color0, color1, input.Diffuse.a);
    
    // Multiply by the diffuse lighting
    return blended * input.Diffuse;
}
