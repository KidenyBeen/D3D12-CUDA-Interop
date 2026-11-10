#include "../Common/Common.hlsli"

#ifndef __PBRFUN_HLSLI__
#define __PBRFUN_HLSLI__

//프레넬 방정식을 계산하여 값을 구하는 과정
float3 SchlickFresnel(float3 F0, float NdotH)
{

    float3 F = F0 + (float3(1.0, 1.0, 1.0) - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);

    return F;

}
float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
	float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic); //F0의 비율이 얼만큼 넣을지를 metallic으로 조절 해주고 있다.
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);

    float3 irradiance = gtxtTextureDiffuseIrradianceIBL.Sample(glinearSamplerState, normalWorld);
    irradiance *= albedo;

    return kd * irradiance;
}


float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
	float metallic, float roughness)
{
	// Environment BRDF
    float R = 1 - roughness;
    float c1 = dot(normalWorld, pixelToEye);
	//SpecularBRDF의 x = cos , y = roughness 값
    float2 specularBRDF = gtxtTextureSpecluarBRDF.Sample(gpointSamplerState, float2(c1, R)).rg;

    float3 specularIrradiance = gtxtTextureSpecluarIBL.SampleLevel(glinearSamplerState, reflect(-pixelToEye, normalWorld), roughness * 10.0f).rgb;

    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (specularBRDF.r * F0 + specularBRDF.g) * specularIrradiance;

}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
	float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);

	//return specularIBL;
	//return diffuseIBL;
	//ao는 애초에 환경맵에 적용하기 위해서 사용하는 값
    return (diffuseIBL + specularIBL) * ao;
}


//F0 읽어오는 함수
float3 F0return(float3 albedo, float metallic)
{
    float3 dielectricF0 = float3(0.04, 0.04, 0.04);
    float3 F = lerp(dielectricF0, albedo, metallic);
    return F;
}

float NdfGGX(float NdotH, float roughness)
{

    float alp = roughness * roughness;
    float alpSq = alp * alp;
	//float D = (alp * alp) / (pi * pow((NdotH * NdotH) * ((alp * alp) - 1) + 1, 2));
    float denom = (NdotH * NdotH) * (alp - 1.0) + 1.0;
    return alpSq / (pi * denom * denom);

	//return D;
}

float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float k = pow(roughness + 1, 2) / 8.0;
    float G1 = NdotI / (NdotI * (1.0 - k) + k);
    float G2 = NdotO / (NdotO * (1.0 - k) + k);
    return G1 * G2;
}
#endif 