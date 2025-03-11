//--------------------------------------------------------------------------------------
// D3D9-style HLSL Vertex Shader
//--------------------------------------------------------------------------------------

// Input vertex structure
struct VS_INPUT
{
    float4 Position : POSITION;   // v0
    float4 Color    : COLOR0;     // v1
    float4 Tex0     : TEXCOORD0;  // v2 (not used in final output, but declared)
    float4 Tex1     : TEXCOORD1;  // v3
};

// Output vertex structure
struct VS_OUTPUT
{
    float4 Position : POSITION;   // o0    
    float2 TexCoord : TEXCOORD0;  // o3.xy -> mapped to TEXCOORD0 in D3D9
    float4 Color    : COLOR0;     // o1
};

// -----------------------------------------------------------------------------
// Match D3D12’s "cb0[3..6]" with c3..c6 in D3D9.
// This means c3 = row0, c4 = row1, c5 = row2, c6 = row3 of the matrix.
// -----------------------------------------------------------------------------
float4x4 gTransform : register(c0);

//--------------------------------------------------------------------------------------
// Main Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    float4 pos = mul(float4(IN.Position.xyz, 1.0), gTransform);
   // pos.xy += gOffset.xy * pos.w;
    OUT.Position = pos;
    OUT.Color = saturate(IN.Color);
    OUT.TexCoord = IN.Tex1.xy;

    return OUT;
}
