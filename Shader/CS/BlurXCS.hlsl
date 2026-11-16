Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);

cbuffer GaussianBlurInfo : register(b0)
{
    int blurRadius;
    float blurSigma;
    uint textureWidth;
    uint textureHeight;
}

static const uint GROUP_SIZE = 16;
static const uint MAX_BLUR_RADIUS = 20;
groupshared float4 sharedTile[GROUP_SIZE][GROUP_SIZE + MAX_BLUR_RADIUS * 2];

float GaussianWeight(int offset)
{
    return exp(-(float)(offset * offset) / (2.0f * blurSigma * blurSigma));
}

[numthreads(GROUP_SIZE, GROUP_SIZE, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID,
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int radius = clamp(blurRadius, 0, (int)MAX_BLUR_RADIUS);
    const uint tileWidth = GROUP_SIZE + (uint)(radius * 2);
    const uint tileElementCount = GROUP_SIZE * tileWidth;
    const uint threadIndex = groupThreadID.y * GROUP_SIZE + groupThreadID.x;

    for (uint tileIndex = threadIndex; tileIndex < tileElementCount;
        tileIndex += GROUP_SIZE * GROUP_SIZE)
    {
        const uint tileY = tileIndex / tileWidth;
        const uint tileX = tileIndex % tileWidth;
        const int sourceX = clamp((int)(groupID.x * GROUP_SIZE + tileX) - radius,
            0, (int)textureWidth - 1);
        const int sourceY = clamp((int)(groupID.y * GROUP_SIZE + tileY),
            0, (int)textureHeight - 1);
        sharedTile[tileY][tileX] = inputTex.Load(int3(sourceX, sourceY, 0));
    }

    GroupMemoryBarrierWithGroupSync();

    if (dispatchThreadID.x >= textureWidth || dispatchThreadID.y >= textureHeight)
    {
        return;
    }

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    for (int offset = -radius; offset <= radius; ++offset)
    {
        const float weight = GaussianWeight(offset);
        const int sharedX = (int)groupThreadID.x + radius + offset;
        color += sharedTile[groupThreadID.y][sharedX] * weight;
        totalWeight += weight;
    }

    outputTex[dispatchThreadID.xy] = color / totalWeight;
}
