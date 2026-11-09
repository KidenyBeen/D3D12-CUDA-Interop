#include "../Common/Common.hlsli"


TextureCube gtxtSkyCubeTexture : register(t10); // t10 : skybox


struct VS_OUTPUT
{
	float3 positionL : POSITION;
	float4 position : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{

	float4 color = gtxtSkyCubeTexture.Sample(glinearSamplerState, input.positionL);

	return color;
}