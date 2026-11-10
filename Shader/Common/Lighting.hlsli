
#include "../Common/PBRFun.hlsli"

#ifndef __LIGHTING_HLSLI__
#define __LIGHTING_HLSLI__

//그림자 맵을 염두해두고 계산하는 함수
float3 LightRadiance(Light light, float3 posWorld, float3 normalWorld, Texture2D shadowMap)
{
    // Directional light Direction Light라면
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : light.position - posWorld;
        
    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light ==> 감쇠 모델이 정욕되어야 함으로
    float spotFator = light.type & LIGHT_SPOT
                     ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
                      : 1.0f;
        
    // Distance attenuation
    float att = light.type & LIGHT_SPOT ? saturate((light.fallOffEnd - lightDist)
                         / (light.fallOffEnd - light.fallOffStart)) : 1.0f;

    // Shadow map
    float shadowFactor = 1.0;

    
    const float nearZ = 0.1; // 카메라 설정과 동일
        
     // 1. Project posWorld to light screen    
    float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
    lightScreen.xyz /= lightScreen.w;
        
     // 2. 카메라(광원)에서 볼 때의 텍스춰 좌표 계산
    float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
    lightTexcoord += 1.0;
    lightTexcoord *= 0.5;
        
    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);
        
    // Texel size
    float dx = 2.0 / (float) width;
    
    if (lightTexcoord.x < 0.0 || lightTexcoord.x > 1.0 ||
        lightTexcoord.y < 0.0 || lightTexcoord.y > 1.0)
    {
        shadowFactor = 1.0;
    }
    else
    {
   
        shadowFactor = PCF_Filter(lightTexcoord.xy, lightScreen.z - 0.001, dx, shadowMap);
        //shadowFactor = PCF_Filter(lightTexcoord.xy, lightScreen.z - 0.0000001, dx, shadowMap);
        //shadowFactor = PCSS(lightTexcoord, lightScreen.z - 0.01, shadowMap, light.invProj, light.radius);
    }

    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}


//그림자 맵 자체가 부여되지 않은 조명에 대한 강도 함수
float3 LightRadiance(in Light light, in float3 posWorld, in float3 normalWorld)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : light.position - posWorld;
        
    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator = light.type & LIGHT_SPOT
                     ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
                      : 1.0f;
        
    // Distance attenuation
    float att = light.type & LIGHT_SPOT ? saturate((light.fallOffEnd - lightDist)
                         / (light.fallOffEnd - light.fallOffStart)) : 1.0f;

    // Shadow map
    float shadowFactor = 1.0;
    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye, Material mat)
{
    float3 halfway = normalize(toEye + lightVec);
    float hdotn = dot(halfway, normal);
    float3 specular = mat.specular * pow(max(hdotn, 0.0f), mat.shininess);

    return mat.ambient + (mat.diffuse + specular) * lightStrength;
}

//블린퐁 모델로 조명 계산
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
                                float3 toEye)
{
    float3 lightVec = -L.direction;

    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.radiance * ndotl;

    // Luna DX12 책에서는 Specular 계산에도
    // Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.radiance * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0f), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
   
}

float3 ComputeDirectionalLightPBR(Light L, float3 normalW,
                                float3 toEye,float3 posW, float3 albedo, float metalness, float roughness )
{
    Light dirLight = L;

    float3 lightVec = -dirLight.direction;
    float3 halfway = normalize(toEye + lightVec);

    float NdotI = max(0.0, dot(normalW, lightVec));
    float NdotH = max(0.0, dot(normalW, halfway));
    float NdotO = max(0.0, dot(normalW, toEye));

    float3 F0 = lerp(Fdielectric, albedo, metalness);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, toEye)));

    float3 kd = lerp(1.0 - F, 0.0, metalness);
    float3 diffuseBRDF = kd * albedo;

    float D = NdfGGX(NdotH, roughness);
    float G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

    float3 radiance =
			dirLight.bIsShadowSet && gnShadowOnOFF
			? LightRadiance(dirLight, posW, normalW, shadowMaps[0])
			: LightRadiance(dirLight, posW, normalW);

    return (diffuseBRDF + specularBRDF) * radiance * NdotI;
}

float3 ComputeSpotLightPBR( Light L, float3 normalW,
                                float3 toEye, float3 posW, float3 albedo, float metalness, float roughness, uint lightIndex)
{

    float3 lightVec = L.position - posW;

    float lightDist = length(lightVec);
    lightVec /= lightDist;

    float3 halfway = normalize(toEye + lightVec);

    float NdotI = max(0.0, dot(normalW, lightVec));
    float NdotH = max(0.0, dot(normalW, halfway));
    float NdotO = max(0.0, dot(normalW, toEye));

    float3 F0 = lerp(Fdielectric, albedo, metalness);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, toEye)));

    float3 kd = lerp(1.0 - F, 0.0, metalness);
    float3 diffuseBRDF = kd * albedo;

    float D = NdfGGX(NdotH, roughness);
    float G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF =
			(F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

    float3 radiance =
			(L.bIsShadowSet && gnShadowOnOFF)
			? LightRadiance(L, posW, normalW, shadowMaps[lightIndex])
			: LightRadiance(L, posW, normalW);

    return (diffuseBRDF + specularBRDF) * radiance * NdotI;
}


#endif 