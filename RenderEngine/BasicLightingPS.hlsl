struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 worldPosition : WORLDPOSITION;
    float3 viewDirection : VIEWDIRECTION;
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
    
    // Albedo Color
    output.Color.rgb = ColorMap.Sample(ColorSmp, input.TexCoord);
    output.Color.a = 1.0f;
    
    // Lighting Lambert
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection);
    float diffuse = max(dot(normal, lightDir), 0.0f);
    float3 lambert = (LightColor * diffuse * LightIntensity);
    
    // Lighting Phong
    
    float3 reflectdir = reflect(-lightDir, normal);
    float3 viewDir = normalize(input.viewDirection);
    float intensity = max(0.0f, dot(reflectdir, viewDir));
    float3 phongColor = float3(1.0f, 1.0f, 1.0f);
    float3 phong = (phongColor * pow(intensity, 25.0f));
    
    // Output color with lighting
    output.Color.rgb *= (1.0f - ambientIntensity) * lambert + ambientIntensity + phong;
    
    return output;
}