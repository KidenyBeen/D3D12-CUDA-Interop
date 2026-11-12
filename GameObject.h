#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Animation.h"

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CShader;
class COBBShader;
//class CAnimationCallbackHandler;
//class CAnimationClips;
//class CAnimationClip;
//class CAnimationController;

enum class EObjectTag //?ㅻ툕?앺듃 ?쇰━??援щ퀎???꾪븳 寃?==> ?닿굔 ?꾨쭏 異⑸룎 泥섎━ 濡쒖쭅?대굹 洹몃윴???곗씪??
{
	None = 0,
	Player = 1,
	Item = 2,
	MissionObject = 3,
	Environment = 4,
	UI = 5,
	Crop = 6,
	Floor = 7, //嫄대Ъ 諛붾떏
	Wall = 8,
	Ceiling = 9,
	Resource = 10,
};

//?대떦 ?ㅻ툕?앺듃媛 ?대뼡 紐⑤뜽??媛뽮퀬?덈뒗吏 媛숈?嫄곕굹 ?뱀? ?뱀젙 ?ㅻ툕?앺듃媛 媛?몄빞?섎뒗 ?ㅼ젙??
//?먮떒?섍린 ?꾪빐 洹몃━怨??쒕쾭?먯꽌???꾨떖 諛쏄퀬 愿由ы븷?ㅻ㈃ 以묒슂?좊벏.
enum class EObjectModelType 
{
	None = 0,

	// Characters (100 ?⑥쐞)
	WoodMan = 100,
	WorkMan = 101,

	// Corn (200 ?⑥쐞)
	Corn01 = 200,
	Corn02 = 201,
	Corn03 = 202,
	Corn04 = 203,
	CornSeed = 210,

	// Potato (300 ?⑥쐞)
	Potato01 = 300,
	Potato02 = 301,
	Potato03 = 302,
	Potato04 = 303,
	PotatoSeed = 310,

	// Pumpkin (400 ?⑥쐞)
	Pumpkin01 = 400,
	Pumpkin02 = 401,
	Pumpkin03 = 402,
	Pumpkin04 = 403,
	PumpkinSeed = 410,

	// Tomato (500 ?⑥쐞)
	Tomato01 = 500,
	Tomato02 = 501,
	Tomato03 = 502,
	Tomato04 = 503,
	TomatoSeed = 510,

	// Others (600 ?⑥쐞)
	LogPlanter = 600,
	Barn = 601,
	Rock01 = 610,
	Rock02 = 611,
	Wall = 620,
	Ceiling = 621,
	Floor = 622,
	Tree01 = 630,
	Tree02 = 631,
	Grass01 = 640,
	Grass02 = 641,
};


//寃뚯엫 媛앹껜???뺣낫瑜?Shader ???섍꺼二쇨린 ?꾪븳 援ъ“泥??곸닔 踰꾪띁)?대떎. 
struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
};

//struct CB_GAMEOBJECT_MATERIAL
//{
//	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//
//	UINT m_nType = 0x00;
//};

class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
	virtual ~CTexture();
private:
	int m_nReferences = 0 ; // Mesh? 媛숈씠 Texture ?먰븳 ?섎굹?? Texture瑜??щ윭媛쒖쓽
	//?ㅻ툕?앺듃媛 李몄“?섍굅???ъ슜?????덉쓬?쇰줈 References 媛?닔媛 ?꾩슂?섎떎.

	UINT m_nTextureType; //Texture??Type??吏?뺥빐以섏빞 ?쒕떎 Type?먮뒗
	/*
	* 1. Texture1D
	* 2. Texture1DArray
	* 3. Texture2D
	* 4. Texture2DArray
	* 5. Texture3D
	* 6. Texture3DArray
	* 7. TextureCube
	* 8. TextureCubeArray
	*/

	int m_nTextures = 0; //Texture 媛?닔
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	ComPtr<ID3D12Resource>* m_ppd3dTextures = NULL;
	ComPtr<ID3D12Resource>* m_ppd3dTextureUploadBuffers;

	ComPtr<ID3D12Resource> texsss;
	
	//ID3D12Resource* t = texsss.Get()[1];

	UINT* m_pnResourceTypes = NULL; //

	DXGI_FORMAT* m_pdxgiBufferFormats = NULL;
	int* m_pnBufferElements = NULL;

	int m_nRootParameters = 0;
	int* m_pnRootParameterIndices = NULL; //Compute?먯꽌 ?ъ슜?쒕떎硫?猷⑦듃 ?뚮씪誘명꽣媛 Graphics Pipline?섍낀 ?ㅻ?寃?
	int* m_pnComputeRootParameterIndices = NULL; //UAV瑜??꾪븳寃?

	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dUavGpuDescriptorHandles = NULL;

	int m_nSamplers = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

public:

	ComPtr<ID3D12Resource> CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, 
		D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex);
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateComputeShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, bool bIsSrvActive);
	void ReleaseShaderVariables();

	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	void LoadTextureFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetComputeRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);
	void SetUavGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);



	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	_TCHAR* GetTextureName(int nIndex) { return(m_ppstrTextureNames[nIndex]); }
	// ?띿뒪爾?由ъ냼?ㅻ? 媛?몄삤???⑥닔? ?붿뒪?щ┰???숈뿉 二쇱냼瑜?媛?몄삤???⑥닔
	//Texture??醫낅쪟???곕씪 index媛 ?꾩슂??寃껋씠??
	ComPtr<ID3D12Resource> GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int nIndex) { return(m_pd3dSrvGpuDescriptorHandles[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavGpuDescriptorHandle(int nIndex) { return(m_pd3dUavGpuDescriptorHandles[nIndex]); }
	int GetRootParameter(int nIndex) { return(m_pnRootParameterIndices[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();

};

/*
* TODO:
* Material??PBR???꾪븳 ?ъ쭏 援ъ“泥??먰븳 異붽?濡??ｌ뼱以??덉젙.
* Shader瑜?material??媛뽯뒗 援ъ“濡?媛쒖꽑???쒕떎.
* Shader????븷???뺣쭚 PSO ?곹깭瑜?Shader 醫낅쪟???곕씪 遺꾨쪟留??섎룄濡?蹂寃쏀븷 ?덉젙?대떎.
* ?꾩옱 Shader??Object? PSO 洹몃━怨?PS, VS ?ㅼ젙???꾨? 媛뽮퀬?덈떎?닿구 GraphicsCommon
* ?쇰줈 沅뚰븳????만 寃껋씠??
*/

//?ㅻ툕?앺듃 TextureMAP
#define MATERIAL_ALBEDO_MAP        0x01    // 0000 0001
#define MATERIAL_NORMAL_MAP        0x02    // 0000 0010
#define MATERIAL_ROUGHNESS_MAP     0x04    // 0000 0100
#define MATERIAL_METALNESS_MAP     0x08    // 0000 1000
#define MATERIAL_AO_MAP            0x10    // 0001 0000
#define MATERIAL_SPECULARF0_MAP    0x20    // 0010 0000
#define MATERIAL_EMISSION_MAP      0x40    // 0100 0000
#define MATERIAL_SMOOTHNESS_MAP 0x80 // 1000 0000

class CMaterial
{
	
public:
	CMaterial();
	CMaterial(int nTextures); //m_ppTextures瑜?珥덇린?뷀빐二쇨린 ?꾪븳 ?앹꽦??
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//湲곕낯 ?됱긽 ?섎??쒕떎

	int 							m_nTextures = 0;
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	CTexture** m_ppTextures = NULL; // 怨꾩링 援ъ“濡??대（?댁쭊 ?ㅻ툕?앺듃?ㅼ?
	//媛곸옄???뚰듃留덈떎 Texture瑜??곕줈 媛吏????덉쓬?쇰줈 ?댁쨷 ?ъ씤???쒖슜.

	//??遺遺꾩? ?꾨쭏 鍮쇱빞?섏?留? ?댁쟾??怨꾩링 援ъ“ ?녿뒗 ?쇰컲 ?ㅻ툕?앺듃?ㅼ씠 ??援ъ“濡?
	//?대（?댁쭊 ?곹깭??吏湲??뱀옣? ?덈틖 ?덉젙. 
	//TODELETE : 
	CTexture* m_pTexture = NULL;
	CShader* m_pShader = NULL;

	void SetAlbedo(XMFLOAT4 xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetTexture(CTexture* pTexture);
	void SetShader(CShader* pShader);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();

public:
	//
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//Glossiness => Roughness濡??ъ슜
	float							m_fRoughness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	UINT							m_nType = 0x00; 
	//?대뒓 醫낅쪟??Texture瑜??ъ슜?좎?瑜??섍꺼以??뚮옒洹?媛?



	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void RemoveMaterialType(UINT nType)
	{
		m_nType &= ~nType;  // ?뚮옒洹?OFF
	}
	
public:
	//Material??Shader瑜?臾댁“嫄?媛뽮퀬?덈뒗 援ъ“ ?섎룄 ??援ъ“濡?蹂寃쏀빐???ъ슜?좊젮 ?덉쓬.
	static CShader* m_pStandardShader; //?좊땲硫붿씠?섏씠 ?ㅼ젙?섏? ?딆? ?쇰컲 Material
	static CShader* m_pSkinnedAnimationShader; //?좊땲硫붿씠?섏씠 議댁옱?섎뒗 寃쎌슦

	//? 2媛쒖쓽 Shader瑜?珥덇린?뷀빐二쇰뒗 ?⑥닔 ==> ?꾩옱??洹몃?濡??ъ슜?섍퀬
	//?섏쨷??湲곕뒫??醫 ??異붽??섍굅???몃텇???댁빞?좊벏
	static void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }

	//?뚯씪濡?遺??Texture ?대쫫??諛쏆븘?ㅻ뒗 ?⑥닔?쇨퀬 蹂대㈃ ?쒕떎
	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//?붾?
	int LoadTextureDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader);
};

//濡쒕뱶??紐⑤뜽 ?먯궛????援곕뜲??紐⑥븘 愿由ы빐二쇰뒗 而⑦뀒?대꼫. ?닿굅 ?섎굹濡?
//1. ?쎌뼱???ㅻ툕?앺듃??Root ?뺣낫???묎렐?섏뿬 ?덈줈??猷⑦듃 ?꾨옒 ?먯떇?쇰줈 異붽? ==> 蹂댄넻??寃뚯엫 ?붿쭊?먯꽌 鍮??ㅻ툕?앺듃瑜?留뚮뱾怨?吏묒뼱?ｋ뒗寃껉낵 媛숈? ?숈옉???쎄쾶 泥섎━ 媛??
//2. Animation? 蹂댄넻 ?곸슜 ?붿쭊?먯꽌 Animatnio, AnimationController, Animator ?대젃寃??섎늻?붾뜲
//Animation ?ㅼ젙??AnimationConroller???섍린????븷??遺꾨챸???섍린 醫뗫떎.
//3. 遺??꾩슂??硫붾え由??ъ슜?놁씠 ?뺣낫留?鍮쇱꽌 ?묎렐?섍린 ?⑹씠?섎떎.
class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

	CGameObject* m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationClips* m_pAnimationClips = NULL;

	EObjectModelType m_ObjectType = EObjectModelType::None;

public:
	void PrepareSkinning();
};


//?ㅻ툕?앺듃 怨좎쑀 ID
typedef uint32_t ObjectID;

class CGameObject
{
public:
	CGameObject();
	CGameObject(int nMaterials); //Materials瑜?吏곸젒 ?좊떦?섎뒗 ?ㅻ툕?앺듃?ㅼ뿉 ??댁꽌 ?꾩슂??
	//?앹꽦??==> Import?댁삤???ㅻ툕?앺듃?ㅼ? m_nMaterials瑜??곕줈 ?쎌뼱?ㅼ?留?
	//?닿? 吏곸젒 ?뺤쓽?댁꽌 異붽??섎뒗 移쒓뎄?ㅼ? ?녿떎
	virtual ~CGameObject();

	//?ㅻ툕?앺듃 ?留곸쓣 ?꾪빐??異붽???珥덇린???⑥닔
	virtual void Init() {};

	//?덊띁?곗뒪 移댁슫?낆쑝濡?愿由ы븷爰쇰땲源??ш린??References 蹂?섎? 留뚮뱾怨?AddRef? Relese瑜?留뚮뱾?댁??? Direct12?먯꽌 湲곕낯?곸쑝濡??명꽣?섏씠?ㅻ뱾???덊띁?곗뒪 移댁슫?낆쑝濡?愿由ы븯?꾨줉 ?섏뼱 ?덇린 ?뚮Ц???숈씪???댁＜??寃껋씠??
private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

	XMFLOAT4X4 m_xmf4x4World; //?붾뱶 醫뚰몴怨꾩긽???먯떊???꾩튂 ?됰젹
	//遺紐??꾩튂 ==> ?닿구 ?뚯븘??怨꾩링 ?좊땲硫붿씠?섏씠 媛??
	XMFLOAT4X4						m_xmf4x4ToParent;

	int         m_nAnimation = 0;               // ?꾩옱 ?좊땲硫붿씠??踰덊샇
	float       m_fAnimationTime = 0.0f;        // ?꾩옱 ?좊땲硫붿씠???쒓컙

	//0 ?대㈃ 湲곕낯, 1 ?대㈃ ?뚮옉?됱긽, 2?대㈃ 鍮④컙 ?됱긽 
	UINT m_nIsBuildingObject = 0;

public:
	//異⑸룎 諛뺤뒪 
	BoundingOrientedBox m_obbBox;
	
	//異⑸룎 泥섎━瑜??ъ? ?덊궗吏 ==> ?닿굔 ?щ윭 怨꾩링 援ъ“ ?대（怨좎엳??GameObject?덉뿉 ? 
	//m_obbBox媛 ?щ윭媛??좊떦?섎땲源??뱀젙 怨꾩링???ㅻ툕?앺듃留??쒖꽦?뷀븯嫄곕굹 ?섎뒗 愿由щ? ?꾪빐??
	bool m_bIsBoxCollision = false; 
	bool m_bIsBoxTriggerCollision = false; //異⑸룎 泥섎━ ?곗궛 ?놁씠 ?곹샇?묒슜 ?섍린留??섎뒗 ?⑸룄濡??쒖꽦??

	//OBB諛뺤뒪 ?ㅼ젙 ==> ??湲곕낯 ?ㅼ젙? 異⑸룎泥댄겕瑜???諛뺤뒪?ㅼ뿉 ?쒗빐???묐룞?섎룄濡??쒕떎.
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};

	//obbmesh
	CMesh* m_pOBBMesh = NULL;

	//紐⑤뱺 ?ㅻ툕?앺듃???꾩튂 湲곗???諛쒕컮?μ씠??媛??諛묐떒?댁뿬??offset?쇰줈
	//媛??ㅻ툕?앺듃 ?ш린 ?뺤쓽??留욎떠????꺼以섏빞???꾩슂媛 ?덈떎.
	XMFLOAT3 m_xmf3OBBOffset = XMFLOAT3(0,0,0);

	COBBShader* m_pOBBShader = NULL;

private:
	bool m_bIsActive = false; //?ㅻ툕?앺듃 ?숈옉 ?곹깭 
	EObjectTag m_Tag = EObjectTag::None; //?ㅻ툕?앺듃 ?쇰━ ?쒓렇
	ObjectID m_nID = 0; // ?쒕쾭/?대씪 怨듭슜 ID
	EObjectModelType m_objType; //?뺥솗???대뼡 ?ㅻ툕?앺듃?몄? 援щ텇

public:

	inline void SetID(ObjectID id) { m_nID = id; }
	inline void SetTag(EObjectTag tag) { m_Tag = tag; }
	inline void SetType(EObjectModelType type) { m_objType = type; }
	inline EObjectTag GetTag() { return m_Tag; }
	inline bool HasTag(EObjectTag tag) const { return m_Tag == tag; }

	inline void Activate(bool bIsActive) { m_bIsActive = bIsActive; }
	inline bool GetActivie() { return m_bIsActive; }
	inline ObjectID GetID() { return m_nID; };
	inline EObjectModelType GetType() { return m_objType; }

	//?ㅻ툕?앺듃 ?留곸뿉??????섎룎???볦쓣???몄텧?섎뒗 ?⑥닔 
	void ResetState();
	
protected:

	//?덉젣?먯꽌???닿구 ?뱀젙???ㅻ툕?앺듃 泥섎━瑜??꾪빐?쒕쭔 ?섎뒗嫄?媛숇떎
	//?뺣쭚 諛섎났?섎뒗 ?ㅻ툕?앺듃?쒗뀒???닿구 ?덈꽔?댁????쒕뜲
	ComPtr<ID3D12Resource> m_pd3dcbObject_Info;
	CB_GAMEOBJECT_INFO* m_pcbMappedObject_Info;

	
	CMesh* m_pMesh = nullptr; // ?ㅻ툕?앺듃???먯떊???뺥깭瑜?媛?몄빞 ?⑥쑝濡?Mesh瑜?媛뽯뒗??
	CShader* m_pShader = nullptr; // ?먯떊???대뼸寃?洹몃┫吏 ?대뼡 Shader濡??곌껐?좎? PSO瑜??곌껐?좎? ?깅벑??寃곗젙吏볥뒗 Shader ?대옒???닿굔 ?ㅻ툕?앺듃媛 媛?몃룄 ?섍퀬 Shader ?대옒?ㅺ? ?ㅻ툕?앺듃瑜?媛?몃룄 ?쒕떎.
	CMaterial* m_pMaterial = nullptr;// Texture? Shader瑜?媛뽮퀬?덉쓣 ?덉젙 ?꾩뿉 Shader???곗씠吏 ?딆쓣 寃껋씠??

	int m_nMaterials = 0;
	CMaterial** m_ppMaterials = NULL; //GameObject媛 怨꾩링援ъ“濡?議댁옱?섍린??Material
	//Part 蹂꾨줈 ?곕줈 議댁옱?????덈떎.
public:
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh* pMesh); // Mesh瑜??ㅼ젙
	virtual void SetShader(CShader* pShader); // Shader瑜??ㅼ젙
	virtual void SetMaterial(CMaterial* pMaterial);
	virtual void SetMaterial(int nMaterial ,CMaterial* pMaterial);

	virtual void Animate(float fTimeElapsed);  // Animate濡??숈옉, 異⑸룎 泥섎━ ?깅벑 臾쇰━?곸씤 ?곗궛?깆쓣 Render ?댁쟾???섑뻾?쒗궗 寃껋씠硫?GameObject?ㅼ? 紐⑤몢 ?곸냽 諛쏆븘 ?낆옄?곸쑝濡??숈옉?좉볼??virtual ???⑥???
	virtual void OnPrepareRender(); //Direct12?먯꽌??CommanndList 紐낅졊?대? ?섑뻾?섍린 ?댁쟾??紐낆떆?곸쑝濡??곌껐?댁쨾?쇳븯嫄곕굹 ?곹깭瑜?吏?뺥빐以섏빞 ?⑥쑝濡?PrepareRender? 媛숈? ?숈옉???댁＜??寃껋씠 ?꾩슂?섎떎.
	//?꾩옱??PSO??猷⑦듃 ?쒓렇?덉퀜 ?곌껐?깆쓣 ?ш린??泥섎━?섎뒗嫄?媛먯븞?섏? ?딄퀬 ?섎?留??먭퀬 ?곌껐?대몦 遺遺?媛숇떎. 2025.07.04

	//
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }
	UINT64 GetCbvGPUDescriptorHandlePtr() { return(m_d3dCbvGPUDescriptorHandle.ptr); }
	

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera); //Animate -> Render濡??ㅼ젣 Draw肄쒖쓣 ?ㅽ뻾?섎뒗 紐낅졊???꾩옱 ?댁긽?쒕줈??Scene??諛곗튂??紐⑤뱺 GameObject 媛?닔留뚰겮 DrawCall??諛쒖깮??寃껋씠??
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera , bool bIsHir , bool bIsNormal, bool bIsWire, bool bIsShadow);
	virtual void DebugRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
	//?곸닔 踰꾪띁瑜??앹꽦?쒕떎.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	//?곸닔 踰꾪띁???댁슜??媛깆떊?쒕떎.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	//?꾩뿉???붿뒪?щ┰???뚯씠釉붿뿉 ?곸닔踰꾪띁瑜??댁? 寃쎌슦 ?꾨옒??洹몃깷
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void ReleaseShaderVariables();

	//寃뚯엫 媛앹껜???붾뱶 蹂???됰젹?먯꽌 ?꾩튂 踰≫꽣? 諛⑺뼢(x-異? y-異? z-異? 踰≫꽣瑜?諛섑솚?쒕떎.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetToParentPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	//寃뚯엫 媛앹껜???꾩튂瑜??ㅼ젙?쒕떎.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	//寃뚯엫 媛앹껜???ш린瑜?議곗젙?쒕떎.
	void SetScale(float x, float y, float z);
	
	//寃뚯엫 媛앹껜瑜?濡쒖뺄 x-異? y-異? z-異?諛⑺뼢?쇰줈 ?대룞?쒕떎.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	//寃뚯엫 媛앹껜瑜??붾뱶 醫뚰몴 湲곗? 諛⑺뼢?쇰줈 ?吏곸씠寃??쒕떎
	void Move(XMFLOAT3 xmf3Offset);
	//寃뚯엫 媛앹껜瑜??뚯쟾(x-異? y-異? z-異??쒕떎.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void SetRotation(float fPitch, float fYaw, float fRoll);

public:

	char							m_pstrFrameName[64]; //?대떦 ?ㅻ툕?앺듃???꾨젅???대쫫

	CGameObject* m_pParent = NULL; //遺紐?
	CGameObject* m_pChild = NULL; //?먯떇
	CGameObject* m_pSibling = NULL; // ?뺤젣 ?ㅻ툕?앺듃

	CAnimationController*			m_pSkinnedAnimationController = NULL;

	//Texutre??媛?닔瑜??쎌뼱?ㅻ뒗 ?⑥닔
	static int LoadCountTexturesFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

	static int LoadFrameHierarchyDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);


	//?좊땲硫붿씠???뺣낫? mesh ?뺣낫 ?깆쓣 ?쎌뼱?ㅻ룄濡??몄텧 ?섎뒗 ?⑥닔.
	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader , EObjectModelType objectType);

	//怨꾩링 援ъ“濡??대（?댁쭊 ?ㅻ툕?앺듃瑜?init ?섎뒗 ?⑥닔
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);


	//湲곕낯?곸쑝濡?紐⑤뱺 寃?됱? 遺紐?-> ?뺤젣 -> ?먯떇 源딆씠 ?먯깋 ?몃━???뺥깭瑜??멸쾬?대떎.
	CGameObject* GetParent() { return(m_pParent); } //?대떦 ?ㅻ툕?앺듃??遺紐⑤? 諛섑솚


	virtual void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL); //怨꾩링 援ъ“??留욎텛???몃━瑜??곕씪 ?吏곸엫???낅뜲?댄듃 ?쒕떎
	
	CGameObject* FindFrame(char* pstrFrameName); // ?뱀젙 ?꾩튂???ㅻ툕?앺듃瑜?寃?됲븯嫄곕굹 ?쒕떎
	//??FindFrame 湲곕뒫???댁젣 醫 ??鍮좊Ⅴ寃??쒕떎嫄곕굹 媛쒖꽑?????덈뒗 諛⑹떇???ㅼ뼇?섍쾶 異붽???以섏빞?쒕떎.
	//?댁쑀 1. Find 湲곕뒫??寃뚯엫 援ъ꽦???덉뼱??援됱옣??留롮씠 ?곗씠???ы빆
	// ++ ?욎쑝濡?Tag 湲곕뒫??異붽??섏뿬 Tag 寃??議곌굔??異붽??섏뿬 醫 ??寃??議곌굔??
	// 醫곹? ?멸쾶?꾨궡 寃?됱쓣 醫 ???먰솢?섍퀬 鍮좊Ⅴ寃???怨꾪쉷.
	// ???ㅻⅨ ?ы빆?쇰줎 ?뺤젣 ??寃?? 諛붾줈 ?꾨옒 ?먯떇?먯꽌留?寃???깅벑 理쒖쟻?뷀븯??
	//諛⑸쾿?ㅼ쓣 ?곸슜?붿쭊?먯꽌??吏?먯쨷???덈떎.

	//
	CTexture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	//Mesh 醫낅쪟???곕씪??Shader??InputLayout?대굹 ?ㅼ젙???щ씪?몄빞 ?⑥쑝濡??꾩슂??
	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); } 

	//?ъ쭏 ?뺣낫瑜??뚯씪濡?遺???쎌뼱?⑤떎. 蹂댄넻 ?뚯씪???대뒓 Texture瑜??대뵒??諛붿씤?⑺븷吏
	//?깅벑 ?뺣낫媛 ?닿꺼?덈떎
	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//?붾?
	int LoadMaterialsDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//?좊땲硫붿씠?섏씠 ?ы븿???ㅻ툕?앺듃???좊땲硫붿씠???뺣낫??媛숈씠 ?쎌뼱?⑤떎
	static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);

	//?꾩옱 ?ㅻ툕?앺듃??Frame ?뺣낫瑜?異쒕젰
	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);

	//?먯떇???ㅼ젙
	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	public:
		void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

		//
		void SetTrackAnimationClip(int nAnimationTrack, int nAnimationClip);
		//
		void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

		void SetRootMotion(bool bRootMotion);

		void PrintMatrixWorld() const;
		void PrintOBB() const;
		void PrintMatrixToParent() const;
		
		virtual void OnPrepareAnimate() { }
		virtual void OnLateUpdate() { }
	
		XMFLOAT4 GetQuaternion() const;
		void SetQuaternion(const XMFLOAT4& quat);
};


class CRotatingObject : public CGameObject //GameObject 以??뚯쟾???섎뒗 ?ㅻ툕?앺듃
{

public:
	CRotatingObject();
	virtual ~CRotatingObject();

	virtual void Init() {};
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;
public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; } //Speed媛믪쓣 ?ㅼ젙
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { // ?뚯쟾 媛곸쓣 ?ㅼ젙
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}

	virtual void Animate(float fTimeElapsed);

public:

	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle); //

};


///////////////////////////////////////////// ???꾨옒濡?援ъ“ 媛쒖꽑
/*
* TOWILL:
* ?꾩뿉 CGameObject??怨꾩링 援ъ“, 珥덇린???깅벑? ?꾨옒媛 媛??媛寃좎?留?
* ?댁젣 ?ㅻⅨ Class???愿怨꾩뿉 ?곕씪 履쇨컻???섎┫ ?덉젙
* ?섏쨷???대뒓?뺣룄 ?뺣━媛 ?앸굹硫? ?명꽣?섏씠???깆쓣 ?댁슜??由ы뙥?좊쭅??媛誘?
* 
* Material??Shader瑜?媛뽯뒗 援ъ“濡?援ъ꽦????寃껋씠硫?==> Shader??
* GraphicsCommon?먯꽌 珥덇린?뷀븳 PSO?ㅼ쓣 ?뱀젙 Shader 援ъ“??留욎떠 媛뽮퀬留?
* ?덈뒗 愿怨꾨줈 蹂寃?
* 
* 1. GameObject媛 Material瑜?媛뽮퀬?덈뒗 援ъ“
* - CDefaultGameObject ?대옒?ㅻ? ?곸냽 諛쏅뒗 援ъ“ 
* 
* 2. Material??GameObject瑜?媛뽮퀬 ?덈뒗 援ъ“
* - CFromMaterialObject???대떦?섎뒗 援ъ“
* 
* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
* ?ш린 ?댁슜?먯꽌 CDefaultGameObject? ?ъ슜 X load ?섎뒗 遺遺꾩뿉??
* 遺紐?愿怨꾧? ??댁쭚.
*/

class CDefaultGameObject : public CGameObject
{
public:
	CDefaultGameObject() : CGameObject(){};
	CDefaultGameObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CDefaultGameObject();

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);



};

class CFromMaterialObject : public CGameObject //
{
	//?ш릿 醫 ?섏쨷??

};

///////////////////////////////////////////////////////////////////////

class CDefulatHumanAnimationCallbackHandler : public CAnimationCallbackHandler
{
private:
	CAnimationController* m_pController = NULL;

public:
	CDefulatHumanAnimationCallbackHandler(CAnimationController* pController) { m_pController = pController; }
	~CDefulatHumanAnimationCallbackHandler() { delete m_pController; }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

//Test??
class CAnimatedObject : public CGameObject
{
	//TODO : 
public:
	/*
	* 1. ID3D12Device
	* 2. ID3D12GraphicsCommandList
	* 3. ID3D12RootSignature
	* 4. LoadModel
	* 5. nAnimationTracks
	*/
	CAnimatedObject() {};
	CAnimatedObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CAnimatedObject();

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);

	void SetAnimationState(EAnimationState State); //?좊땲硫붿씠?섏뿉 ???곹깭瑜?諛쏆븘? 諛섏쁺?섎뒗 ?⑥닔

};

///////////////////////////////////////////////////////////////////////

class CWoodManObject : public CAnimatedObject
{
	
public:

	CWoodManObject() {};
	CWoodManObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CWoodManObject();

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};

///////////////////////////////////////////////////////////////////////

class CSkyBoxObject : public CGameObject
{
public:
	CSkyBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pwstrSkyBoxHDRPath);
	virtual ~CSkyBoxObject();
};

///////////////////////////////////////////////////////////////////////

class CPlanObject : public CGameObject
{
public:
	CPlanObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CPlanObject();
};

class CTestObject : public CGameObject
{
public:
	CTestObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CTestObject() = default;

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
};
//////////////////////////////////////////////////
//

class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage* m_pHeightMapImage;

	int								m_nWidth;
	int								m_nLength;

	XMFLOAT3						m_xmf3Scale;

	int m_nSeasonIndex = 0;// 0,1,2,3 怨꾩젅??Terrain??遺꾧린?댁꽌 ?뚮젮二쇨린 ?꾪빐

public:
	float GetHeight(float x, float z) { return(m_pHeightMapImage->GetHeight(x, z) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir, bool bIsNormal, bool bIsWire, bool bIsShadow);
	
	void SetSeason(int num) { m_nSeasonIndex = num; }
};


//////////////////////////////////////////////////
//

class CCropObject : public CDefaultGameObject
{
public:
	CCropObject() {};
	CCropObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel) :CDefaultGameObject() {};

	virtual ~CCropObject() {};

	virtual void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel) {};

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};

	void CCropObject::ChangeModel(CLoadedModelInfo* pNewModel);
	void SetGrowth(float num) { m_fGrowth = num; }
	float GetGrowth() { return m_fGrowth; }

	void AddHP(int num) { m_nHP += num; }
	void SetHP(int num) { m_nHP = num; }
	int GetHP() { return m_nHP; }

protected:
	float m_fGrowth = 0.0f;
	int m_nHP = 1; // 泥대젰


};

//////////////////////////////////////////////////
//
// ?μ닔??
class CCornObject : public CCropObject
{
public:
	CCornObject() {};
	CCornObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CCornObject();

	virtual void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


};


//////////////////////////////////////////////////
// 媛먯옄
class CPotatoObject : public CCropObject
{
public:
	CPotatoObject() {};
	CPotatoObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CPotatoObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


};


//////////////////////////////////////////////////
// ?몃컯
class CPumpkinObject : public CCropObject
{
public:
	CPumpkinObject() {};
	CPumpkinObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CPumpkinObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


};


//////////////////////////////////////////////////
// ?좊쭏??
class CTomatoObject : public CCropObject
{
public:
	CTomatoObject() {};
	CTomatoObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CTomatoObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);



};


//////////////////////////////////////////////////
// 媛꾩씠 諛?Log Planter)
enum class ECropState
{
	Empty = 0,       // ?꾨Т寃껊룄?놁쓬
	Growing = 1,     // ?먮씪??以?
	Harvestable = 2, // ?섑솗 媛??
	Dead = 3,         // 怨좎궗
};

class CLogPlnaterObject : public CDefaultGameObject
{
public:
	CLogPlnaterObject() {};
	CLogPlnaterObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CLogPlnaterObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	//?꾩옱 ?ъ뼱?몄엳???묐Ъ???곹깭
	ECropState m_cropState = ECropState::Empty;

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};


//////////////////////////////////////////////////
// ?쏄컙
class CBarnObject : public CDefaultGameObject
{
public:
	CBarnObject() {};
	CBarnObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CBarnObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);
	void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};


/////////////////////////////////////////////////////
// 

class CRockObject : public CDefaultGameObject
{
public:
	CRockObject() {};
	CRockObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CRockObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void AddHP(int num) { m_nHP += num; }
	int GetHP() { return m_nHP; }
	void SetHP(int num) { m_nHP = num; }
private:
	int m_nHP = 10; // 泥대젰
};

/////////////////////////////////////////////////////
// 

class CWallObject : public CDefaultGameObject
{
public:
	CWallObject() {};
	CWallObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CWallObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	//2硫댁뿉 ?ㅼ튂 ?덈뒗吏 ?덊뻽?붿?
	UINT* m_npPlaceable = NULL;
	int m_nSelectIndex = -1;
};
/////////////////////////////////////////////////////
// 

class CCeilingObject : public CDefaultGameObject
{
public:
	CCeilingObject() {};
	CCeilingObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CCeilingObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	
	//4硫댁뿉 ?ㅼ튂 ?덈뒗吏 ?덊뻽?붿?
	UINT* m_npPlaceable = NULL;
	int m_nSelectIndex = -1;
};
/////////////////////////////////////////////////////
// 

class CFloorObject : public CDefaultGameObject
{
public:
	CFloorObject() {};
	CFloorObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CFloorObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	
	//4硫댁뿉 ?ㅼ튂 ?덈뒗吏 ?덊뻽?붿?
	UINT* m_npPlaceable = NULL;
	int m_nSelectIndex = -1;

};

/////////////////////////////////////////////////////
//?섎Т 

class CTreeObject : public CDefaultGameObject
{
public:
	CTreeObject() {};
	CTreeObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CTreeObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void AddHP(int num) { m_nHP += num; }
	int GetHP() { return m_nHP; }
	void SetHP(int num) { m_nHP = num; }
private :
	int m_nHP = 5; // 泥대젰
};

/////////////////////////////////////////////////////
// ?

class CGrassObject : public CDefaultGameObject
{
public:
	CGrassObject() {};
	CGrassObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CGrassObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void AddHP(int num) { m_nHP += num; }
	int GetHP() { return m_nHP; }
	void SetHP(int num) { m_nHP = num; }

private:
	int m_nHP = 1; // 泥대젰

};
//////////////////////////////////////////////////
/////////////////////////////////////////////////////
// 
class CParticleObject : public CGameObject
{
public:
	CParticleObject();
	virtual ~CParticleObject();

	virtual void UpdateShaderVariables(
		ID3D12GraphicsCommandList* pd3dCommandList,
		XMFLOAT4X4* pxmf4x4World);

	virtual void SetParticleState(
		float fTime,
		float fTimeElapsed,
		float fSpawnRadius,
		float fAlphaScale,
		UINT  nParticleFlags,
		XMFLOAT3 gParticleColor,
		UINT nBlockCount,
		WeatherBlock* blocks);

	void SetPreviousPosition(XMFLOAT3 pos);
	XMFLOAT3 GetPreviousPosition();
protected:
	ComPtr<ID3D12Resource> m_cpcbParticle;
	cbParticle* m_pcbParticle = nullptr;

	XMFLOAT3 m_xmf3PreviousPosition;
};

//?뚰떚???ㅻ툕?앺듃 (1) Rain

class CRainParticleObject : public CParticleObject
{
public:

	CRainParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);

	virtual ~CRainParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);


};
/////////////////////////////////////////////////////
// ?뚰떚???ㅻ툕?앺듃 (2) Snow

class CSnowParticleObject : public CParticleObject
{
public:
	CSnowParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);

	virtual ~CSnowParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);


};

/////////////////////////////////////////////////////
// ?뚰떚???ㅻ툕?앺듃 (3) Dust ==> ?⑹궗濡????덉젙

class CDustParticleObject : public CParticleObject
{
public:
	CDustParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);
	virtual ~CDustParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

};

/////////////////////////////////////////////////////
// ?뚰떚???ㅻ툕?앺듃 (4) punch

class CPunchParticleObject : public CParticleObject
{
public:
	CPunchParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);

	virtual ~CPunchParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	void Animate(float fTimeElapsed, float fTime);
	float m_fCircleTime = 0.0f;

	void PrintStartTIme();

	virtual void SetParticleState(
		float fTime,
		float fTimeElapsed,
		float fSpawnRadius,
		float fAlphaScale,
		UINT  nParticleFlags,
		XMFLOAT3 gParticleColor,
		UINT nBlockCount,
		WeatherBlock* blocks);

};


/////////////////////////////////////////////////////
//
struct PreOBBData 
{
	EObjectModelType m_objectType = EObjectModelType::None;
	CMesh* m_pOBBMesh = NULL;
	BoundingOrientedBox m_OBBBox;
	XMFLOAT3 m_xmf3Offset = XMFLOAT3(0, 0, 0);
};
class CBuildingObject : public CGameObject
{
public:
	CBuildingObject() {};
	CBuildingObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual ~CBuildingObject();

	PreOBBData* m_pOBBData = NULL;
	UINT m_nOBBData = 0;
	
	EObjectModelType m_objType = EObjectModelType::None;
	EObjectTag m_objTypeTag = EObjectTag::None;

	//?꾩옱 異⑸룎???ㅻ툕?앺듃媛 ?꾧뎔吏 媛瑜댄궎??蹂??
	CGameObject* m_pHitGameObject = NULL;

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir, bool bIsNormal, bool bIsWire, bool bIsShadow);
	//?ㅼ튂?????덈뒗 紐⑤뱺 紐⑤뜽????섏뿬 ?꾩떆 OBB瑜??ъ쟾??Data ??ν빐?붾떎
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//SetOBB?먯꽌 ?ㅼ젙??Data瑜?湲곗??쇰줈 OBB瑜??명똿?쒕떎
	void ResetOBB(PreOBBData m_OBBData);

};

///////////////////////////////////////////////////////////////////////

class CPostProcessingObject : public CGameObject
{
public:
	CPostProcessingObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CPostProcessingObject();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

//////////////////////////////////////////////////
//

/////////////////////////////////////////////////////
// ?ㅻ툕?앺듃 ?留?
template <typename T>
class CObjectPool 
{
private:
	vector<T*> m_vpList;

public:
	virtual ~CObjectPool()
	{
		for (T* obj : m_vpList)
		{
			delete obj;
		}
	}

	// 狩?媛蹂 ?몄옄 ?쒗뵆由?
	template <typename... ARGS>
	T* Acquire(ARGS&&... args)
	{
		T* obj = nullptr;

		if (!m_vpList.empty())
		{
			//std::cout << "?ъ궗??以? << endl;
			// ?ъ궗??媛앹껜
			obj = m_vpList.back();
			m_vpList.pop_back();

			// ?앹꽦?먮뒗 ?대? ?ㅽ뻾???곹깭?대?濡?Init留??몄텧?쒕떎
			obj->Init(std::forward<ARGS>(args)...);
		}
		else
		{
			//std::cout << "?덈줈 ?앹꽦 以? << endl;
			// 理쒖큹 ?앹꽦: new T( args... )
			obj = new T(std::forward<ARGS>(args)...);
		}
		//std::cout << m_vpList.size()<<"?꾩옱 ?留곸쓽 ?ш린" << endl;
		obj->Activate(true);
		return obj;
	}

	void Release(T* obj)
	{
		obj->ResetState();
		obj->Activate(false);
		m_vpList.push_back(obj);
	}
};

