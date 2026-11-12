#include "../Common/Common.hlsli"
#include "../Common/Lighting.hlsli"
#include "../Common/PBRFun.hlsli"



struct PS_OUTPUT
{
	float4 pixelColor : SV_Target0;
	float4 nomalColor : SV_Target1;
	float4 ambientLight : SV_Target2;
};


[earlydepthstencil]
PS_OUTPUT main(VS_PBR_OUTPUT input) : SV_TARGET
{

	float3 posView = mul(float4(input.positionW, 1.0), gmtxView).xyz; //
	uint clusterIndex = GetClusterIndex_PS(posView); //
	ClusterLightList cluster = gClusterLightLists[clusterIndex];

	float4 albedoATest;
	float3 albedo;
	float3 normalW;
	float  roughnessValue;
	float  metalnessValue;
	float3 emission;
	float  aoValue;
	float3 specularF0;
	float lod = 0;
	//환경광
	float3 ambientLighting = float3(0,0,0);

	//직사광 ==> 직접 빛에 영향을 받는 경우
	float3 directLighting = float3(0, 0, 0);

	//눈을 향하는 방향
	float3 pixelToEye = normalize(gf3EyeWorld - input.positionW);

	normalW = GetNormal(input);
	float3 pixelToEyeD = gf3EyeWorld - input.positionW;
	lod = ComputeLOD(length(pixelToEyeD));


	// Albedo ==> 사용안하거나 없는 경우 우선 그냥 하얀색 일종의 오류로 볼 수 도 있다
	albedoATest =
		((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
		gtxtTextureAlbedo.SampleLevel(glinearSamplerState, input.uv, lod) :
		gMaterial.m_cAlbedo;
	//albedo = float3(0, 0, 0);
	albedo = albedoATest.rgb;
	albedo = pow(albedo, gf1Gamma);
	// Roughness
	roughnessValue =
		((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
		gtxtTextureRoughnness.SampleLevel(glinearSamplerState, input.uv , lod).r :
		gf1Roughness;
	//roughnessValue = 0;
	// Metalness
	metalnessValue =
		((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
		gtxtTextureMetal.SampleLevel(glinearSamplerState, input.uv, lod).r :
		gf1Metalness;
	//metalnessValue = 0
	// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
	aoValue =
		((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
		gtxtTextureOcclusion.SampleLevel(glinearSamplerState, input.uv, lod).r :
		1.0f; // AO가 없으면 영향 없음

	// Specular F0
	specularF0 =
		((gnTexturesMask & MATERIAL_SPECULARF0_MAP) && (gnTextureUseMask & MATERIAL_SPECULARF0_MAP_USE)) ?
		gtxtTextureSpecluarF0.SampleLevel(glinearSamplerState, input.uv, lod).rgb :
		F0return(albedo, metalnessValue);

	// Emission
	emission =
		((gnTexturesMask & MATERIAL_EMISSION_MAP) && (gnTextureUseMask & MATERIAL_EMISSION_MAP_USE)) ?
		gtxtTextureEmission.SampleLevel(glinearSamplerState, input.uv, lod).rgb :
		gMaterial.m_cEmissive;
	
	clip(albedoATest.a - 0.1f);

	//DirectionLight는 클러스터링에 포함 x
	if(gnLightTypeMask & DIRECTION_LIGHT_USE)
	{
		directLighting += ComputeDirectionalLightPBR(light[0], normalW, pixelToEye, input.positionW, 
			albedo, metalnessValue, roughnessValue);
	}

	//클러스터링된 Light에 대하여 Spot 조명들만 처리하는 중
	for (uint i = 0; i < cluster.lightCount; ++i)
	{
		// cluster index 0 → light[1]
		uint lightIndex = cluster.lightIndices[i] + 1;
		Light L = light[lightIndex];
	
		if (L.bIsUse == 0)
			continue;

		//Spot Light에 대해서만 있기 때문에 이렇게 해둔 것 만일 Point Light도 생긴다면 조건을 추가해서 분류 한다.
		directLighting += ComputeSpotLightPBR(L, normalW, pixelToEye, input.positionW,
			albedo, metalnessValue, roughnessValue, lightIndex);
	}

//	for (int i = 0; i < MAX_LIGHTS; ++i)
//	{
//		if (light[i].bIsUse == 1)
//		{
//			float3 lightVec = light[i].type & LIGHT_DIRECTIONAL ? -light[i].direction : light[i].position - input.positionW;
//			float lightDist = length(lightVec);
//			lightVec /= lightDist;
//			float3 halfway = normalize(pixelToEye + lightVec);
//
//			float NdotI = max(0.0, dot(normalW, lightVec));
//			float NdotH = max(0.0, dot(normalW, halfway));
//			float NdotO = max(0.0, dot(normalW, pixelToEye));
///**/
//			//Fresnel
//			float3 F = SchlickFresnel(specularF0, max(0.0, dot(halfway, pixelToEye)));
//
//			//Diffuse
//			float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalnessValue);
//			float3 diffuseBRDF = kd * albedo;
//
//			float D = NdfGGX(NdotH, roughnessValue);
//			float3 G = SchlickGGX(NdotI, NdotO, roughnessValue);
//			float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);
//
//			float3 radiance = 0.0f;
//
//			if (light[i].bIsShadowSet && gnShadowOnOFF) // 그림자를 지원하는 경우
//			{
//				radiance = LightRadiance(light[i], input.positionW, normalW, shadowMaps[i]);
//			}
//			else //그림자를 지원하지 않는 경우
//			{
//				radiance = LightRadiance(light[i], input.positionW, normalW);
//			}
//
//			directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
//		}
//	}


	ambientLighting = AmbientLightingByIBL(albedo, normalW, pixelToEye, aoValue, metalnessValue, roughnessValue);
	//DIRECT_LIGHT_USE, AMBIENT_LIGHT_USE
	ambientLighting = ambientLighting * 0.1;
	//재질이 어떤 조명 타입에 영향을 받을지를 지정해주는 것.
	ambientLighting = gnLightTypeMask & AMBIENT_LIGHT_USE ? ambientLighting : float3(0, 0, 0);
	directLighting = gnLightTypeMask & DIRECT_LIGHT_USE ? directLighting : float3(0, 0, 0);

	PS_OUTPUT output;
	//최종 색상은 환경광과 직접광 2개의 합으로 이루어진다.
	//output.pixelColor = float4(ambientLighting + directLighting, 1.0);
	output.pixelColor = float4(directLighting, 1.0);
	//output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
	float3 normalV = normalize(mul(normalW, (float3x3)gmtxView));
	output.nomalColor = float4(normalV, 1.0);
	output.ambientLight = float4(ambientLighting, 1.0);

	if (gnIsBuildingObj == 1)
	{
		float3 buildColor = float3(0, 0, 1);
		output.pixelColor.rgb = lerp(output.pixelColor.rgb, buildColor, 0.5);
	}
	else if (gnIsBuildingObj == 2)
	{
		float3 buildColor = float3(1, 0, 0);
		output.pixelColor.rgb = lerp(output.pixelColor.rgb, buildColor, 0.5);

	}

	return(output);
}