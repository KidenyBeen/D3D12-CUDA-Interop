#include "../Common/Common.hlsli"
#include "../Common/Lighting.hlsli"
#include "../Common/PBRFun.hlsli"

//TODO: Texture 몇개 쓸껀지 구분 생각해야함


static const float gParallaxScale = 0.03f; //시차 매핑을 위한 상수
static const float gParallaxBias = -0.015f; // 시차 매핑을 위한 상수




//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD0;
	float2 uv2 : TEXCOORD1;
};

//노멀 매핑 함수
float3 GetNormal(VS_OUTPUT input , Texture2D normalMap)
{
	float3 normalW = input.normalW;

	//노멀맵을 사용할 것이며 노멀맵이 존재하는 경우
	if (gnTextureUseMask & MATERIAL_NORMAL_MAP_USE && gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normal = normalMap.SampleLevel(glinearSamplerState, input.uv, 0.0).rgb; // 샘플링
		normal = 2.0 * normal - 1.0; //범위 -1 ~ 1 변경 필요

		//만약 OpenGL용 노멀맵에 경우 y가 반대라는 것도 알아야 한다.

		float3 N = normalW;
		//float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N); //Tangent가 N과 수직이 아닐 수 있기에 만드는 것
		float3 T = float3(1, 0, 0);
		float3 B = cross(N, T);

		float3x3 TBN = float3x3(T, B, N);
		normalW = normalize(mul(normal, TBN));
	}

	return normalW;
}


struct PS_OUTPUT
{
	float4 pixelColor : SV_Target0;
	float4 nomalColor : SV_Target1;
	float4 ambientLight : SV_Target2;
};
[earlydepthstencil]
PS_OUTPUT  main(VS_OUTPUT input) : SV_TARGET
{
	//그리드 공간에 저장된 조명에 값에 인덱스를 읽어들이는 과정
	float3 posView = mul(float4(input.positionW, 1.0), gmtxView).xyz; //
	uint clusterIndex = GetClusterIndex_PS(posView); //
	ClusterLightList cluster = gClusterLightLists[clusterIndex];

	float3 albedo;
	float3 normalW;
	float  roughnessValue;
	float  metalnessValue;
	float3 emission;
	float  aoValue;
	float3 specularF0;

	//lod
	float lod = 0;

	//환경광
	float3 ambientLighting = float3(0, 0, 0);

	//직사광 ==> 직접 빛에 영향을 받는 경우
	float3 directLighting = float3(0, 0, 0);

	//눈을 향하는 방향
	float3 pixelToEyeD = gf3EyeWorld - input.positionW;

	float3 pixelToEye = normalize(pixelToEyeD);

	lod = ComputeLOD(length(pixelToEyeD));

	normalW = GetNormal(input , gtxtTextureNormal1);

	float4 unityMask1 =
		gtxtTextureMaskMap1.SampleLevel(glinearSamplerState, input.uv, lod);

	float4 unityMask2 =
		gtxtTextureMaskMap2.SampleLevel(glinearSamplerState, input.uv, lod);

	float4 unityMask3 =
		gtxtTextureMaskMap3.SampleLevel(glinearSamplerState, input.uv, lod);

	float4 unityMask4 =
		gtxtTextureMaskMap4.SampleLevel(glinearSamplerState, input.uv, lod);


	float height1 = unityMask1.b;
	float height2 = unityMask2.b;
	float height3 = unityMask3.b;
	float height4 = unityMask4.b;

	// UV 보정 (간이 Parallax)
	float2 parallaxOffset1 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height1 * gParallaxScale + gParallaxBias);

	float2 parallaxOffset2 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height2 * gParallaxScale + gParallaxBias);

	float2 parallaxOffset3 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height3 * gParallaxScale + gParallaxBias);

	float2 parallaxOffset4 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height4 * gParallaxScale + gParallaxBias);

	// UV 보정 적용
	float2 uvParallax1 = input.uv + parallaxOffset1;
	float2 uvParallax2 = input.uv + parallaxOffset2;
	float2 uvParallax3 = input.uv + parallaxOffset3;
	float2 uvParallax4 = input.uv + parallaxOffset4;

	 unityMask1 =
		gtxtTextureMaskMap1.SampleLevel(glinearSamplerState, uvParallax1, lod);

	unityMask2 =
		gtxtTextureMaskMap2.SampleLevel(glinearSamplerState, uvParallax2, lod);

	unityMask3 =
		gtxtTextureMaskMap3.SampleLevel(glinearSamplerState, uvParallax3, lod);

	unityMask4 =
		gtxtTextureMaskMap4.SampleLevel(glinearSamplerState, uvParallax4, lod);


	albedo =
		((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
		gtxtTextureAlbedo1.SampleLevel(glinearSamplerState, uvParallax1, lod).rgb :
		gMaterial.m_cAlbedo.rgb;

	albedo = pow(albedo, gf1Gamma);

	roughnessValue =
		((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
		1.0 - unityMask1.a :
		gf1Roughness;


	metalnessValue =
		((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
		unityMask1.r :
		gf1Metalness;


	// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
	aoValue =
		((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
		unityMask1.g :
		1.0f; // AO가 없으면 영향 없음

	// Specular F0
	specularF0 = F0return(albedo, metalnessValue);

	// Emission
	emission = gMaterial.m_cEmissive;


	float mask1 = gtxtTextureMasking01.SampleLevel(glinearSamplerState, input.uv2, 0).r;
	float mask2 = gtxtTextureMasking02.SampleLevel(glinearSamplerState, input.uv2, 0).r;

	// Layer 2
	if (mask1 > 0.0)
	{

		if (gnSeason & SEASON_SUMMER || gnSeason & SEASON_WINTER)
		{
			albedo =
				((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
				gtxtTextureAlbedo4.SampleLevel(glinearSamplerState, uvParallax4, lod).rgb :
				albedo;

			roughnessValue =
				((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
				1.0 - unityMask4.a :
				gf1Roughness;


			metalnessValue =
				((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
				unityMask4.r :
				gf1Metalness;


			// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
			aoValue =
				((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
				unityMask4.g :
				1.0f; // AO가 없으면 영향 없음

			normalW = GetNormal(input, gtxtTextureNormal4);
		}
		else
		{
			albedo =
				((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
				gtxtTextureAlbedo2.SampleLevel(glinearSamplerState, uvParallax2, lod).rgb :
				albedo;

			roughnessValue =
				((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
				1.0 - unityMask2.a :
				gf1Roughness;


			metalnessValue =
				((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
				unityMask2.r :
				gf1Metalness;


			// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
			aoValue =
				((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
				unityMask2.g :
				1.0f; // AO가 없으면 영향 없음
			normalW = GetNormal(input, gtxtTextureNormal2);
		}
	}
	// Layer 3 (우선순위 높음)
	if (mask2 > 0.0)
	{
		albedo =
			((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
			gtxtTextureAlbedo3.SampleLevel(glinearSamplerState, uvParallax3, lod).rgb :
			albedo;

		roughnessValue =
			((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
			1.0 - unityMask3.a :
			gf1Roughness;


		metalnessValue =
			((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
			unityMask3.r :
			gf1Metalness;


		// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
		aoValue =
			((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
			unityMask3.g :
			1.0f; // AO가 없으면 영향 없음

		normalW = GetNormal(input, gtxtTextureNormal3);
	}

	albedo = pow(albedo, gf1Gamma);
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
	//float4 albe2do = gtxtTextureAlbedo.Sample(glinearSamplerState, input.uv);

	ambientLighting = ambientLighting * 0.1;
	PS_OUTPUT output;
	//최종 색상은 환경광과 직접광 2개의 합으로 이루어진다.
	//output.pixelColor = float4(ambientLighting + directLighting, 1.0);
	output.pixelColor = float4(directLighting, 1.0);
	output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
	float3 normalV = normalize(mul(normalW, (float3x3)gmtxView));
	output.nomalColor = float4(normalV, 1.0);
	output.ambientLight = float4(ambientLighting,1.0);
	//output.nomalColor = float4(normalW, 1.0);
	//out.pixelColor = color;
	return(output);

}