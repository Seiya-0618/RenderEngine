struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};

cbuffer CameraTransform : register(b0)
{
    float4x4 View : packoffset(c0);
    float4x4 Projection : packoffset(c4);
}

cbuffer ObjectTransform : register(b1)
{
    float4x4 World : packoffset(c0);
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float4 localPos = float4(input.Position, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Projection, viewPos);
    float3 worldNormal = mul((float3x3) World, input.Normal);
    
    output.Position = projPos;
    output.TexCoord = input.TexCoord;
    output.Normal = worldNormal.xyz;

    return output;
}