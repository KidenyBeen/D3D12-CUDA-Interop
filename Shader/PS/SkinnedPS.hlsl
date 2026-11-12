#include "../Common/Common.hlsli"
#include "../Common/Lighting.hlsli"
#include "../Common/PBRFun.hlsli"

//참고자료
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl

struct PS_OUTPUT
{
	float4 pixelColor : SV_Target0;
	float4 nomalColor : SV_Target1;
	float4 ambientLight : SV_Target2;
};


[earlydepthstencil]
PS_OUTPUT main(VS_PBR_OUTPUT input)
{

	/// 클러스터 부분
	float3 posView = mul(float4(input.positionW, 1.0), gmtxView).xyz; //클러스터링을 위해 View Space로 가공
	uint clusterIndex = GetClusterIndex_PS(posView); //View Space 기준으로 클러스터링이 되어 있기에 view Space 기준으로 해당 Pixel의 영향을 주는 Grid 공간에 접근
	ClusterLightList cluster = gClusterLightLists[clusterIndex]; //해당 그리드 공간에 어떤 조명이 영향을 주는지 저장되어 있음.

	//uint count = gClusterLightLists[clusterIndex].lightCount;
	//PS_OUTPUT o;
	//o.pixelColor = float4(count / 10.0, 0, 0, 1);
	//o.nomalColor = float4(0, 0, 1, 1);
	//o.ambientLight = float4(0, 0, 0, 1);
	//return o;

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
	//pixelToEye = normalize(input.positionW - gf3EyeWorld);
	float3 pixelToEyeD = gf3EyeWorld - input.positionW;
	normalW = GetNormal(input);

	//최소한의 환경광이고 이제 IBL이 나오면 좀 내용이 달라진다.
	//ambientLighting = gMaterial.m_cAmbient;

	// Albedo ==> 사용안하거나 없는 경우 우선 그냥 하얀색 일종의 오류로 볼 수 도 있다
	lod = ComputeLOD(length(pixelToEyeD));

	// Albedo ==> 사용안하거나 없는 경우 우선 그냥 하얀색 일종의 오류로 볼 수 도 있다
	albedo =
		((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
		gtxtTextureAlbedo.SampleLevel(glinearSamplerState, input.uv, lod).rgb :
		gMaterial.m_cAlbedo.rgb;
	//albedo = float3(0, 0, 0);
	
	albedo = pow(albedo, gf1Gamma); //
	// Roughness
	roughnessValue =
		((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
		gtxtTextureRoughnness.SampleLevel(glinearSamplerState, input.uv, lod).r :
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

	//DirectionLight는 클러스터링에 포함 x
	if (gnLightTypeMask & DIRECTION_LIGHT_USE)
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



	ambientLighting = AmbientLightingByIBL(albedo, normalW, pixelToEye, aoValue, metalnessValue, roughnessValue);
	//DIRECT_LIGHT_USE, AMBIENT_LIGHT_USE

	//재질이 어떤 조명 타입에 영향을 받을지를 지정해주는 것.
	ambientLighting = gnLightTypeMask & AMBIENT_LIGHT_USE ? ambientLighting : float3(0, 0, 0);
	directLighting = gnLightTypeMask & DIRECT_LIGHT_USE ? directLighting : float3(0, 0, 0);

	ambientLighting = ambientLighting * 0.1;

	PS_OUTPUT output;
	//최종 색상은 환경광과 직접광 2개의 합으로 이루어진다.
	//output.pixelColor = float4(ambientLighting + directLighting, 1.0);
	output.pixelColor = float4(directLighting, 1.0);
	//output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
	float3 normalV = normalize(mul(normalW, (float3x3)gmtxView));
	output.nomalColor = float4(normalV, 1.0);
	output.ambientLight = float4(ambientLighting, 1.0);
	//output.nomalColor = float4(normalW, 1.0);
	//output.pixelColor = float4(albedo,1.0);


	return(output);
}