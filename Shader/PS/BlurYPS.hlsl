Texture2D<float4> inputTex : register(t14);

cbuffer GaussianBlurInfo : register(b3)
{
    int blurRadius;
    float blurSigma;
    uint textureWidth;
    uint textureHeight;
}

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
};

float GaussianWeight(int offset)
{
    return exp(-(float)(offset * offset) / (2.0f * blurSigma * blurSigma));
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
    const int2 pixel = int2(input.position.xy);
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    for (int offset = -blurRadius; offset <= blurRadius; ++offset)
    {
        const int sampleY = clamp(pixel.y + offset, 0, (int)textureHeight - 1);
        const float weight = GaussianWeight(offset);
        color += inputTex.Load(int3(pixel.x, sampleY, 0)) * weight;
        totalWeight += weight;
    }

    return color / totalWeight;
}
