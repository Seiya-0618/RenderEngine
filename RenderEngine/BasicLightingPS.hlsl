struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};

struct PSOutput
{
    float4 Color : SV_TARGET0;
};

cbuffer LightData : register(b2)
{
    float3 LightDirection : packoffset(c0);
    float3 LightColor : packoffset(c1);
    float LightIntensity : packoffset(c2);
}

SamplerState ColorSmp : register(s0);
Texture2D<float4> ColorMap : register(t0);

PSOutput main (VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    float ambientIntensity = 0.2f;
    output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    float4 tex = ColorMap.Sample(ColorSmp, input.TexCoord);
    output.Color.r = tex.r;
    output.Color.a = 1.0f;
    float3 normal = input.Normal;
    float3 lightDir = normalize(-LightDirection);
    float diffuse = max(dot(normal, lightDir), 0.0f);
    output.Color.rgb *= (1.0f - ambientIntensity) * (LightColor * diffuse * LightIntensity) + ambientIntensity;
    return output;
}