struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET0;
};

SamplerState ColorSmp : register(s0);
Texture2D<float4> ColorMap : register(t0);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    float4 tex = ColorMap.Sample(ColorSmp, input.TexCoord);
    //output.Color = float4(input.TexCoord, 0, 1);
    output.Color.r = tex.r;
    output.Color.a = 1.0f;
    
    return output;

}