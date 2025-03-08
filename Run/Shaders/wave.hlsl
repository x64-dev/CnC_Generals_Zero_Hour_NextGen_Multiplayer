// Direct3D 9 HLSL Pixel Shader ported from Direct8 assembly shader
// Command & Conquer Generals(tm) shader port

sampler2D BumpMap : register(s0);       // t0
sampler2D EnvMap : register(s1);        // t1
sampler2D AlphaTexture : register(s2);  // Optional alpha texture

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse  : COLOR0;
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample bump map to get offset (texbem)
    float2 bumpOffset = tex2D(BumpMap, input.texCoord).rg;
    bumpOffset = (bumpOffset - 0.5f) * 2.0f; // Convert to [-1, 1] range

    // Calculate perturbed texture coordinates
    float2 reflectionUV = input.texCoord + bumpOffset;

    // Sample environment reflection texture using offset coordinates
    float4 reflection = tex2D(EnvMap, bumpOffset);

    // Output color calculation
    float4 finalColor = reflection * input.color;

    // Uncomment and adjust if using DO_WATER_ALPHA_TEXTURE
    // float alpha = tex2D(alphaSampler, input.texCoord).r;
    // finalColor.a *= alpha;

    return finalColor;
}