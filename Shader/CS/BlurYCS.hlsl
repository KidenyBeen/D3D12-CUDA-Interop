Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);

cbuffer GaussianBlurInfo : register(b0)
{
    int blurRadius;
    float blurSigma;
    uint textureWidth;
    uint textureHeight;
}

float GaussianWeight(int offset)
{
    return exp(-(float)(offset * offset) / (2.0f * blurSigma * blurSigma));
}

[numthreads(32, 32, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    if (dtID.x >= textureWidth || dtID.y >= textureHeight)
    {
        return;
    }

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    for (int offset = -blurRadius; offset <= blurRadius; ++offset)
    {
        const int sampleY = clamp((int)dtID.y + offset, 0, (int)textureHeight - 1);
        const float weight = GaussianWeight(offset);
        color += inputTex.Load(int3(dtID.x, sampleY, 0)) * weight;
        totalWeight += weight;
    }

    outputTex[dtID.xy] = color / totalWeight;
}
