#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// ============================================================
// Frame / Engine Limits
// ============================================================
#define FRAME_BUFFER_WIDTH 1280
#define FRAME_BUFFER_HEIGHT 720

#define MAX_LIGHTS_PER_CLUSTER 128

#define MAX_LIGHTS 9 //이거 이제 안씀 클러스터드 렌더링에선 필요 x
#define POINT_LIGHT 1
#define SPOT_LIGHT 8
#define DIRECTIONAL_LIGHT 1

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256
#define MAX_BLOCK 4  // 

#define NEAR_PLANE 2
// #define LIGHT_WORLD_RADIUS 0.001
//#define LIGHT_FRUSTUM_WIDTH 0.34641 // <- 계산해서 찾은 값
#define LIGHT_FRUSTUM_WIDTH 0.11547

// ============================================================
// Material Texture Flags
// ============================================================

//오브젝트 TextureMAP
#define MATERIAL_ALBEDO_MAP        0x01    // 0000 0001
#define MATERIAL_NORMAL_MAP        0x02    // 0000 0010
#define MATERIAL_ROUGHNESS_MAP     0x04    // 0000 0100
#define MATERIAL_METALNESS_MAP     0x08    // 0000 1000
#define MATERIAL_AO_MAP            0x10    // 0001 0000
#define MATERIAL_SPECULARF0_MAP    0x20    // 0010 0000
#define MATERIAL_EMISSION_MAP      0x40    // 0100 0000
#define MATERIAL_SMOOTHNESS_MAP 0x80 // 1000 0000

// On/Off용 플래그 _MAP_USE
#define MATERIAL_ALBEDO_MAP_USE        0x01
#define MATERIAL_NORMAL_MAP_USE        0x02
#define MATERIAL_ROUGHNESS_MAP_USE     0x04
#define MATERIAL_METALNESS_MAP_USE     0x08
#define MATERIAL_AO_MAP_USE            0x10
#define MATERIAL_SPECULARF0_MAP_USE    0x20
#define MATERIAL_EMISSION_MAP_USE      0x40

// On/Off용 플래그  PBR, Direct, Ambient 활성화 여부
#define DIRECT_LIGHT_USE        0x01
#define AMBIENT_LIGHT_USE        0x02
#define DIRECTION_LIGHT_USE    0x04 //Direction Light를 꺼서 클러스터드 렌더링에 성능 테스트를 보여주기 위해

//Direction Light인지 Spot Light인지 
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_SPOT        0x02

// ============================================================
// Season / Weather
// ============================================================

#define SEASON_SPRING 0x01
#define SEASON_SUMMER 0x02
#define SEASON_FALL 0x04
#define SEASON_WINTER 0x08

#define WEATHER_CLEAR   0x01   // 맑음
#define WEATHER_RAIN    0x02   // 비
#define WEATHER_SNOW    0x04   // 눈
#define WEATHER_DUST    0x08   // 황사 / 먼지
#define WEATHER_FOG     0x10   // 안개

// ============================================================
// Particle Flags
// ============================================================

//Particle 설정에 필요
#define PF_RAIN         0x01    
#define PF_SNOW         0x02   
#define PF_DUST         0x04   
#define PF_PUNCH         0x08   //무언가 캘때 나오는 파티클을 생각중

// Particle Behavior Flags (상위 비트)
#define PF_USE_COLOR    0x0100  // (1 << 8)  색상 적용 (황사/연기)
#define PF_USE_NOISE    0x0200  // (1 << 9)  노이즈 이동
#define PF_USE_WIND     0x0400  // (1 << 10) 바람 영향

// 재질
struct Material  //우선 블린퐁 모델 기준으로 Materail 정의 나중에 변경 가능
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; // 16 bytes 맞춰주기 위해 추가
    float3 specular;
    float dummy2;
};

//PBR 재질
struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cAlbedo;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
    float m_cRoughness;
    float m_cSmoothness;
    float m_Metallic;
};

struct Light
{
    float3 radiance; //강도
    float fallOffStart; // 범위 시작 
    float3 direction; // 방향
    float fallOffEnd; //범위 끝
    float3 position; //위치
    float spotPower; //spot 조명에 경우 모이는 힘
    
    float radius;
    uint bIsShadowSet; //0 : bool, 1 : true
    uint bIsUse; //0 : bool, 1 : true
    uint type; //조명 type
    
    Matrix viewProj; //Shadow Map 연산에사용해야함
    Matrix invProj;
};

struct ClusterLightList
{
    uint lightCount; // 이 클러스터에 들어온 조명 수
    uint lightIndices[MAX_LIGHTS_PER_CLUSTER]; // gLights의 인덱스
};

struct WeatherBlock
{
    float3 center;
    float _pad0; // 16-byte align

    float3 extent;
    float _pad1; // 16-byte align
};

//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_PBR_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

// ============================================================
// Textures
// ============================================================

//그림자 맵
Texture2D shadowMaps[5] : register(t30);
StructuredBuffer<ClusterLightList> gClusterLightLists : register(t15); //구조체 버퍼로 

Texture2D gtxtTextureAlbedo : register(t1);
Texture2D gtxtTextureNormal : register(t2);
Texture2D gtxtTextureRoughnness : register(t3);
Texture2D gtxtTextureMetal : register(t4);
Texture2D gtxtTextureOcclusion : register(t5);
Texture2D gtxtTextureSpecluarF0 : register(t6);
Texture2D gtxtTextureEmission : register(t7);

Texture2D gtxtTextureSpecluarBRDF : register(t11); // t11 : IBL BRDF LUT
TextureCube gtxtTextureSpecluarIBL : register(t12); // t12 : Specluar IBL
TextureCube gtxtTextureDiffuseIrradianceIBL : register(t13); // t13 : Irradiance

//Terrain꺼
Texture2D gtxtTextureAlbedo1 : register(t40);
Texture2D gtxtTextureNormal1 : register(t41);
Texture2D gtxtTextureMaskMap1 : register(t42);

Texture2D gtxtTextureAlbedo2 : register(t43);
Texture2D gtxtTextureNormal2 : register(t44);
Texture2D gtxtTextureMaskMap2 : register(t45);

Texture2D gtxtTextureAlbedo3 : register(t46);
Texture2D gtxtTextureNormal3 : register(t47);
Texture2D gtxtTextureMaskMap3 : register(t48);

Texture2D gtxtTextureMasking01 : register(t49); //Terrain MaskingTexture
Texture2D gtxtTextureMasking02 : register(t50);

Texture2D gtxtTextureAlbedo4 : register(t51);
Texture2D gtxtTextureNormal4 : register(t52);
Texture2D gtxtTextureMaskMap4 : register(t53);

//////////////////////////////////////////////////////////////
/// 샘플러
SamplerState glinearSamplerState : register(s0);
SamplerState gpointSamplerState : register(s1);
SamplerComparisonState shadowCompareSampler : register(s2);
SamplerState shadowPointSampler : register(s3);
SamplerState gLinearmipPointSampler : register(s4);

//
static const Material g_DebugMaterial =
{
    /* ambient   */
    float3(0.1f, 0.1f, 0.1f),
    /* shininess */ 32.0f,

    /* diffuse   */ float3(0.8f, 0.8f, 0.8f),
    /* dummy1    */ 0.0f,

    /* specular  */ float3(1.0f, 1.0f, 1.0f),
    /* dummy2    */ 0.0f
};



//////////////////////////////////////////////////////////////
/// 

static const MATERIAL gMaterial =
{
    /* m_cAmbient   */
    float4(0.03, 0.03, 0.03, 1.0),
    /* m_cAlbedo    */ float4(1.0, 1.0, 1.0, 1.0),
    /* m_cSpecular  */ float4(1.0, 1.0, 1.0, 32.0),
    /* m_cEmissive  */ float4(0.0, 0.0, 0.0, 1.0),
    /* m_cRoughness */ 0.5,
    /* m_cSmoothness*/ 0.5,
    /* m_Metallic   */ 0.0
};

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
static const float pi = 3.141593;



//Camera가 갖고있는 정보를 넘겨주는 CB로 Object와 달리 이건 한 프레임에 한 번만 Update가 된다.
cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProj : packoffset(c4);
    matrix gmtxinvProj : packoffset(c8);
    float3 gf3EyeWorld : packoffset(c12); //카메라의 월드상 좌표
};


//Player 자체의 Wordl matrix를 넘기는건데 현재 사용하지 않고 있음
cbuffer cbPlayerInfo : register(b1)
{
    matrix gmtxplayerWorld : packoffset(c0);
};

cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxWorld : packoffset(c0);
   // MATERIAL gMaterial : packoffset(c4); //개별로 갖고있는 경우에 대해서
   // uint gnTexturesMask : packoffset(c8);
};

/* 이건 예전 Object에 필요한 CB인데 루트 파라미터에 크기 DWORD가 64를 넘기게 되면서 저 gMaterial 부분을 날렸음
cbuffer cbGameObjectMaterial : register(b3)
{
    matrix gmtxWorldCb : packoffset(c0); //
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8.w);
    uint gnIsBuildingObj : packoffset(c9);
};*/

cbuffer cbGameObjectMaterial : register(b3)
{
    matrix gmtxWorldCb : packoffset(c0); //
    uint gnTexturesMask : packoffset(c4.x);
    uint gnIsBuildingObj : packoffset(c4.y);
};

//ImGui로 넘겨주는 Falga 값들
cbuffer cbImGuiMaterialFlages : register(b4)
{
    uint gnTextureUseMask : packoffset(c0); //어떤 좋류에의 Texture를 사용할 수 있는지 여부
    float3 gf3Dummy : packoffset(c0.y);
    uint gnLightTypeMask : packoffset(c1.x); //현재 Scene에 배정해서 사용할 Light에 Type을 설정
    float3 gf3Dummy2 : packoffset(c1.y);
    float gf1Exposure : packoffset(c2.x); //노출도
    float gf1Gamma : packoffset(c2.y); //감마
    float gf1Roughness : packoffset(c2.z); //모든 오브젝트의 거칠기를 한 번에 제어
    float gf1Metalness : packoffset(c2.w); //모든 오브젝트의 금속도를 제어
    uint gnShadowOnOFF : packoffset(c3.x); //Shadow를 사용할지
    uint gnSeason : packoffset(c3.y); //게임의 계절을 조절하기 위한 것.
    uint gnWeather : packoffset(c3.z); // 게임의 날씨
};

cbuffer cbShadowMapIndex : register(b5)
{
    uint nShadowIndex : packoffset(c0);
};

cbuffer cbLighht : register(b10)
{
    Light light[MAX_LIGHTS];
};

//Skinned Mesh를 사용하고 Animation을 적용할 오브젝트들은 BoneOffset과 Transform에 정보를 넘겨받아야 한다.
cbuffer cbBoneOffsets : register(b11)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b12)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

cbuffer cbParticle : register(b15)
{
    float fTime : packoffset(c0);
    float fDeltaTime : packoffset(c0.y);
    float fStartTime : packoffset(c0.z);
    float fSpawnRadius : packoffset(c0.w);
    
    float fAlphaScale : packoffset(c1.x);
    float3 _pad1 : packoffset(c1.y); // cbuffer alignment
    
    uint nParticleFlags : packoffset(c2.x);
    float3 gParticleColor : packoffset(c2.y);

    uint blockCount : packoffset(c3.x);
    float3 _pad2 : packoffset(c3.y); // cbuffer alignment

    WeatherBlock blocks[MAX_BLOCK] : packoffset(c4.x);
};

cbuffer ClusterCB : register(b16)
{
    uint3 clusterDim; // (X, Y, Z)
    uint totalClusters; // X * Y * Z

    float nearZ;
    float farZ;

    float2 screenSize;
    float2 invScreenSize;
};

static const float2 poissonDisk[16] =
{
    float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
            float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
            float2(-0.91588581, 0.45771432), float2(-0.81544232, -0.87912464),
            float2(-0.38277543, 0.27676845), float2(0.97484398, 0.75648379),
            float2(0.44323325, -0.97511554), float2(0.53742981, -0.47373420),
            float2(-0.26496911, -0.41893023), float2(0.79197514, 0.19090188),
            float2(-0.24188840, 0.99706507), float2(-0.81409955, 0.91437590),
            float2(0.19984126, 0.78641367), float2(0.14383161, -0.14100790)
};

static const float2 offsets[9] =
{
    float2(-1, -1), float2(0, -1), float2(1, -1),
            float2(-1, 0), float2(0, 0), float2(1, 0),
            float2(-1, +1), float2(0, +1), float2(1, +1)
};

static const float2 diskSamples64[64] =
{
    float2(0.0, 0.0),
            float2(-0.12499844227275288, 0.000624042775189866), float2(0.1297518688031755, -0.12006020382326336),
            float2(-0.017851253586055427, 0.21576916541852392), float2(-0.1530983013115895, -0.19763833164521946),
            float2(0.27547541035593626, 0.0473106572479027), float2(-0.257522587854559, 0.16562643733622642),
            float2(0.0842605283808073, -0.3198048832600703), float2(0.1645196099088727, 0.3129429627830483),
            float2(-0.3528833088400373, -0.12687935349026194), float2(0.36462214742013344, -0.1526456341030772),
            float2(-0.17384046457324884, 0.37637015407303087), float2(-0.1316547617859344, -0.4125130588224921),
            float2(0.3910687393754993, 0.2240317858770442), float2(-0.45629121277761536, 0.10270505898899496),
            float2(0.27645268679640483, -0.3974278701387824), float2(0.06673001731984558, 0.49552709793561556),
            float2(-0.39574431915605623, -0.33016879600548193), float2(0.5297612167716342, -0.024557141621887494),
            float2(-0.3842909284448636, 0.3862583103507092), float2(0.0230336562454131, -0.5585422550532486),
            float2(0.36920334463249477, 0.43796562686149154), float2(-0.5814490172413539, -0.07527974727019048),
            float2(0.4903718680780365, -0.3448339179919178), float2(-0.13142003698572613, 0.5981043168868373),
            float2(-0.31344141845114937, -0.540721256470773), float2(0.608184438565748, 0.19068741092811003),
            float2(-0.5882602609696388, 0.27536315179038107), float2(0.25230610046544444, -0.6114259003901626),
            float2(0.23098706800827415, 0.6322736546883326), float2(-0.6076303951666067, -0.31549215975943595),
            float2(0.6720886334230931, -0.1807536135834609), float2(-0.37945598830371974, 0.5966683776943834),
            float2(-0.1251555455510758, -0.7070792667147104), float2(0.5784815570900413, 0.44340623372555477),
            float2(-0.7366710399837763, 0.0647362251696953), float2(0.50655463562529, -0.553084443034271),
            float2(8.672987356252326e-05, 0.760345311340794), float2(-0.5205650355786364, -0.5681215043747359),
            float2(0.7776435491294021, 0.06815798190547596), float2(-0.6273416101921778, 0.48108471615868836),
            float2(0.1393236805531513, -0.7881712453757264), float2(0.4348773806743975, 0.6834703093608201),
            float2(-0.7916014213464706, -0.21270211499241704), float2(0.7357897682897174, -0.38224784745000717),
            float2(-0.2875567908732709, 0.7876776574352392), float2(-0.3235695699691864, -0.7836151691933712),
            float2(0.7762165924462436, 0.3631291803355136), float2(-0.8263007976064866, 0.2592816844184794),
            float2(0.4386452756167397, -0.7571098481588484), float2(0.18988542402304126, 0.8632459242554175),
            float2(-0.7303253445407815, -0.5133224046555819), float2(0.8939004035324556, -0.11593993515830946),
            float2(-0.5863762307291154, 0.6959079795748251), float2(-0.03805753378232556, -0.9177699189461416),
            float2(0.653979655650389, 0.657027860897389), float2(-0.9344208130797295, -0.04310155546401203),
            float2(0.7245109901504777, -0.6047386420191574), float2(-0.12683493131695708, 0.9434844461875473),
            float2(-0.5484582700240663, -0.7880790100251422), float2(0.9446610338564589, 0.2124041692463835),
            float2(-0.8470120123194587, 0.48548496473788055), float2(0.29904134279525085, -0.9377229203230629),
            float2(0.41623562331748715, 0.9006236205438447),
};

// NdcDepthToViewDepth
float N2V(float ndcDepth, matrix invProj)
{
    float4 pointView = mul(float4(0, 0, ndcDepth, 1), invProj);
    return pointView.z / pointView.w;
}

//PCF 연산
float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV, Texture2D shadowMap)
{
    float sum = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        float2 offset = offsets[i] * filterRadiusUV;
        sum += shadowMap.SampleCmpLevelZero(shadowCompareSampler, uv + offset, zReceiverNdc);
    }
    return sum / 9.0;
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

//ClusterIndex 정보를 읽어오는 함수
uint GetClusterIndex_PS(float3 posView)
{
	// Screen space (View → Clip → NDC → [0,1])
    float4 clip = mul(float4(posView, 1.0), gmtxProj);
    float2 ndc = clip.xy / clip.w;
    ndc = ndc * 0.5f + 0.5f;
    ndc = saturate(ndc);

    uint x = min(clusterDim.x - 1, uint(ndc.x * clusterDim.x));
    uint y = min(clusterDim.y - 1, uint(ndc.y * clusterDim.y));

	// Z slice (View space 기준)
    float z = posView.z; // LH, forward +
    float zNorm = saturate((z - nearZ) / (farZ - nearZ));
    uint zSlice = min(clusterDim.z - 1, uint(zNorm * clusterDim.z));

    return x
		+ y * clusterDim.x
		+ zSlice * clusterDim.x * clusterDim.y;
}

float ComputeLOD(float dist)
{

    float distMin = 5.0;
    float distMax = 20.0;

    float lod = 5.0 * saturate(dist / (distMax - distMin));
    return lod;
}


//노멀 매핑 함수
float3 GetNormal(VS_PBR_OUTPUT input)
{
    float3 normalW = input.normalW;

	//노멀맵을 사용할 것이며 노멀맵이 존재하는 경우
    if (gnTextureUseMask & MATERIAL_NORMAL_MAP_USE && gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3 normal = gtxtTextureNormal.SampleLevel(glinearSamplerState, input.uv, 0.0).rgb; // 샘플링
        normal = 2.0 * normal - 1.0; //범위 -1 ~ 1 변경 필요


		//만약 OpenGL용 노멀맵에 경우 y가 반대라는 것도 알아야 한다.

        float3 N = normalW;
        float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N); //Tangent가 N과 수직이 아닐 수 있기에 만드는 것
        float3 B = cross(N, T);

        float3x3 TBN = float3x3(T, B, N);
        normalW = normalize(mul(normal, TBN));
    }

    return normalW;
}
#endif 