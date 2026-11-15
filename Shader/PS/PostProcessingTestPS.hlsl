#include "../Common/Common.hlsli"


Texture2D gtxtTexture : register(t14); //HDR 후처리 Texture



//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_Target0;
};



float4  main(VS_OUTPUT input) : SV_TARGET
{

    //return shadowMaps[1].SampleLevel(glinearSamplerState, input.uv,0) * 1 ;
    //return float4(1.0f,0.0f,1.0f,1.0f);
    return gtxtTexture.SampleLevel(glinearSamplerState, input.uv, 0).rgba;
}