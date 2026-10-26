#ifndef APP_CROWD_REPLACE_SOURCE_NODES_H
#define APP_CROWD_REPLACE_SOURCE_NODES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/containers/map.h>
#include <system/misc/Random.h>

#include <crowd_system/CrowdDefines.h>

namespace AftkCrowdReplaceSourceNodes
{
	class C_CrowdRandomNumberGenerator
	{
	public:
		C_CrowdRandomNumberGenerator();

		void	Seed(const U32 seed);

		S32		operator()(S32 range);

	private:
		Util::C_Random	m_Random;
	};

	///////////////////////////////////////////////////////////////////////////////////////

	enum E_CrowdFlagGroup
	{
		E_FlagGroup_Unknown				= -1,

		E_FlagGroup_UpperClothing		= 0,
		E_FlagGroup_LowerClothing		= 1,
		E_FlagGroup_FullBodyClothing	= 2,

		E_FlagGroup_Skin				= 3,
		E_FlagGroup_Head				= 4,
		E_FlagGroup_Hair				= 5,

		E_FlagGroup_Hat					= 6,
		E_FlagGroup_Shoes				= 7,

		E_FlagGroup_Accessory			= 8,

		E_FlagGroup_Prop				= 9,

		E_FlagGroup_Count
	};

	enum E_CrowdAppearance
	{
		E_CrowdAppearance_Generic				= 0,
		
		// Workers
		E_CrowdAppearance_Worker_Overalls		= 1,
		E_CrowdAppearance_Worker_Overalls_J		= 2,
		E_CrowdAppearance_Worker_HighVisVest	= 3,
		E_CrowdAppearance_Worker_HighVisJacket	= 4,

		E_CrowdAppearance_Count
	};

	enum E_CrowdGender
	{
		E_CrowdGender_Female	= 0,
		E_CrowdGender_Male		= 1,

		E_CrowdGender_Unisex	= 2,

		E_CrowdGender_Count
	};

	enum E_CrowdClothingStyle
	{
		E_CrowdClothingStyle_Baggy	= 0,
		E_CrowdClothingStyle_Skinny	= 1,

		E_CrowdClothingStyle_All	= 2,

		E_CrowdClothingStyle_Count
	};

	enum E_CrowdClothingConditions
	{
		E_CrowdClothingConditions_Hot	= 0,
		E_CrowdClothingConditions_Cold	= 1,

		E_CrowdClothingConditions_All	= 2,

		E_CrowdClothingConditions_Count
	};

	enum E_CrowdClothingAppearance
	{
		E_CrowdClothingAppearance_Generic				= 0,
		
		E_CrowdClothingAppearance_Worker_Any			= 1,
		E_CrowdClothingAppearance_Worker_Overalls		= 2,
		E_CrowdClothingAppearance_Worker_HighVisVest	= 3,
		E_CrowdClothingAppearance_Worker_HighVisJacket	= 4,
      
		E_CrowdClothingAppearance_Count
	};

	enum E_CrowdSleeveType
	{
		E_CrowdSleeveType_Long	= 0,
		E_CrowdSleeveType_Short	= 1,

		E_CrowdSleeveType_Count
	};

	enum E_CrowdNeckType
	{
		E_CrowdNeckType_Closed	= 0,
		E_CrowdNeckType_Open	= 1,
		E_CrowdNeckType_Strap	= 2,

		E_CrowdNeckType_Count
	};

	enum E_CrowdSkinType
	{
		E_CrowdSkinType_Unknown		= 0x00000000,

		E_CrowdSkinType_ShortArms	= 0x00000001,
		E_CrowdSkinType_LongArms	= 0x00000002,
		E_CrowdSkinType_Neck		= 0x00000004,
		E_CrowdSkinType_Bust		= 0x00000008,
		E_CrowdSkinType_Stomach		= 0x00000010,
		E_CrowdSkinType_Legs		= 0x00000020,
	};

	enum E_CrowdIncludeHair
	{
		E_CrowdIncludeHair_Yes			= 0,
		E_CrowdIncludeHair_No			= 1,
		E_CrowdIncludeHair_NotFussed	= 2,

		E_CrowdIncludeHair_Count
	};

	enum E_CrowdAccessoryType
	{
		E_CrowdAccessoryType_Unknown	= -1,

		E_CrowdAccessoryType_Glasses	= 0,
		E_CrowdAccessoryType_Scarf		= 1,
		E_CrowdAccessoryType_Watch		= 2,
		E_CrowdAccessoryType_Visor		= 3,

		E_CrowdAccessoryType_Count
	};

	enum E_CrowdPropType
	{
		E_CrowdPropType_None			= 0,

		E_CrowdPropType_SprayCan		= 1,
		E_CrowdPropType_Skateboard		= 2,
		E_CrowdPropType_BMX				= 3,
		E_CrowdPropType_GlowStick		= 4,
		E_CrowdPropType_CellPhone		= 5,
		E_CrowdPropType_FlagPole		= 6,

		E_CrowdPropType_Count
	};

	enum E_CrowdHandPropFlags
	{
		E_CrowdHandPropFlag_None		= 0,
		E_CrowdHandPropFlag_Left		= 1,
		E_CrowdHandPropFlag_Right		= 2,
		E_CrowdHandPropFlag_Both		= 3,

		E_CrowdHandPropFlag_Count
	};

	///////////////////////////////////////////////////////////////////////////////////////

	struct C_CrowdImposterInfo
	{
	public:
		C_CrowdImposterInfo();

		const bool	IsValid() const;

		enum E_ImposterInfo
		{
			E_ImposterInfo_Front	= 0,
			E_ImposterInfo_Back		= 1,
			E_ImposterInfo_Side		= 2,
			
			E_ImposterInfo_Count,
			E_ImposterInfo_Unknown	= E_ImposterInfo_Count
		};

		std::string							ImagePath[E_ImposterInfo_Count];
	};

	///////////////////////////////////////////////////////////////////////////////////////

	struct C_CrowdPlaceholderData
	{
	public:
		C_CrowdPlaceholderData();

		void Create(Aftk::C_SceneNode* pSceneNode);

		std::string				Name;
		Aftk::C_SceneNode*		ParentPtr;
		U32						BehaviourHash;
		E_CrowdAppearance		Appearance;
		
		E_CrowdPropType			PropType;
		E_CrowdHandPropFlags	PropHandFlags;
		bool					PropOptional;
		
		E_CrowdPropType			SecondaryPropType;
		E_CrowdHandPropFlags	SecondaryPropHandFlags;
		bool					SecondaryPropOptional;

		U32						SecondaryBehaviourHash;
		
		U32						NumInstances;
		Aftk::C_Bounds			Bounds;
		C_CrowdImposterInfo		ImposterInfo;

		inline bool operator == (const C_CrowdPlaceholderData& d) const
		{
			return ((BehaviourHash == d.BehaviourHash) && (Appearance == d.Appearance) && (PropType == d.PropType)
				&& (PropHandFlags == d.PropHandFlags) && (PropOptional == d.PropOptional));
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////

	class C_CrowdRandomActorDesc
	{
	public:
		C_CrowdRandomActorDesc();

		void				Seed(const U32 seed, const E_CrowdAppearance appearance);

		Util::C_Random&		GetRandom();

		const bool			Female() const;
		const bool			Cold() const;
		const E_CrowdAppearance
							Appearance() const;

		const E_CrowdClothingStyle
							Style() const;

		const bool			UpperClothing() const;
		void				UpperClothing(const bool b);
		const bool			Arms() const;
		void				Arms(const bool b);
		const bool			Torso() const;
		void				Torso(const bool b);
		const bool			Stomach() const;
		void				Stomach(const bool b);

		const bool			LowerClothing() const;
		void				LowerClothing(const bool b);
		const bool			Legs() const;
		void				Legs(const bool b);
		const bool			Shoes() const;
		void				Shoes(const bool b);

		const bool			Head() const;
		void				Head(const bool b);
		const bool			Hair() const;
		void				Hair(const bool b);
		const bool			Hat() const;
		void				Hat(const bool b);

		const bool			Glasses() const;
		void				Glasses(const bool b);
		const bool			Scarf() const;
		void				Scarf(const bool b);
		const bool			Watch() const;
		void				Watch(const bool b);
		const bool			Visor() const;
		void				Visor(const bool b);

		const bool			Prop() const;
		void				Prop(const bool b);

		const E_CrowdSleeveType	SleeveType() const;
		void					SleeveType(const E_CrowdSleeveType type);

		const E_CrowdNeckType	NeckType() const;
		void					NeckType(const E_CrowdNeckType type);

		const bool				Hooded() const;
		void					Hooded(const bool hooded);

		const bool				Cropped() const;
		void					Cropped(const bool cropped);

		const Util::C_String&	ColourDesc() const;
		void					ColourDesc(const Util::C_String& desc);

		const E_CrowdIncludeHair
								IncludeHair() const;
		void					IncludeHair(const E_CrowdIncludeHair includeHair);

		const bool				RequiresLegs() const;
		void					RequiresLegs(const bool requiresLegs);

		const bool				RequiresHat();
		void					RequiresHat(const bool requiresHat);

		const E_CrowdPropType	PropType() const;
		void					PropType(const E_CrowdPropType type);

		const E_CrowdHandPropFlags
								PropHandFlags() const;
		void					PropHandFlags(const E_CrowdHandPropFlags flag);

		const bool				UseSecondaryBehaviour() const;
		void					UseSecondaryBehaviour(const bool useSecondaryBehaviour);
		
	private:
		Util::C_Random		m_Random;

		// General
		bool				m_Female;
		bool				m_Cold;
		E_CrowdAppearance	m_Appearance;

		E_CrowdClothingStyle
							m_Style;

		union
		{
			U16				m_iFlags;
			struct
			{
				U32			m_UpperClothing : 1;
				U32			m_Arms : 1;
				U32			m_Torso : 1;
				U32			m_Stomach : 1;

				U32			m_LowerClothing : 1;
				U32			m_Legs : 1;
				U32			m_Shoes : 1;

				U32			m_Head : 1;
				U32			m_Hair : 1;
				U32			m_Hat : 1;

				U32			m_Glasses : 1;
				U32			m_Scarf : 1;
				U32			m_Watch : 1;
				U32			m_Visor : 1;

				U32			m_Prop : 1;
			};
		};

		// Upper Clothing Related
		E_CrowdSleeveType	m_SleeveType;
		E_CrowdNeckType		m_NeckType;
		bool				m_Hooded;
		bool				m_Cropped;

		// Skin Related
		Util::C_String		m_ColourDesc;

		// Head Related
		E_CrowdIncludeHair	m_IncludeHair;

		// Lower Clothing Related
		bool				m_RequiresLegs;

		// Hair/Hat Related
		bool				m_RequiresHat;

		// Prop Related
		E_CrowdPropType			m_PropType;
		E_CrowdHandPropFlags	m_PropHandFlag;
		
		// Behaviour related
		bool				m_UseSecondaryBehaviour;
	};

	///////////////////////////////////////////////////////////////////////////////////////

	class C_CrowdRandomActor
	{
	public:
		C_CrowdRandomActor(const U32 seed, const E_CrowdAppearance appearance);

		bool							AddActorPart(Aftk::C_SceneNode* pSceneNode);

		const S32						GetNumActorParts() const;
		Aftk::C_SceneNode*				GetActorPart(const S32 index) const;

		C_CrowdRandomActorDesc&			GetActorDesc();
		const C_CrowdRandomActorDesc&	GetActorDesc() const;

		void							SetName(const Util::C_String& name);
		const Util::C_String&			GetName() const;

	private:
		bool							UpdateUpperClothingFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateSkinFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateLowerClothingFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateHeadFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateHairFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateHatFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateShoeFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdateAccessoryFlags(AftkUt::C_NodeFlagParser& nodeFlags);
		bool							UpdatePropFlags(AftkUt::C_NodeFlagParser& nodeFlags);

		Ut::Vector<Aftk::C_SceneNode*>	m_ActorParts;
		C_CrowdRandomActorDesc			m_ActorDesc;

		Util::C_String					m_Name;
	};

	///////////////////////////////////////////////////////////////////////////////////////

	class C_CrowdSkinnedSceneDescription
	{
	public:
		C_CrowdSkinnedSceneDescription();
		~C_CrowdSkinnedSceneDescription();

		void		Build(Aftk::C_SceneNode* pSceneNode, const U32 seed);
		void		End();

		const U32	GetRandomSeed() const;
		const bool	HasSkinnedNodes() const;

		const S32	GetSkinnedNodeIndex(const Aftk::C_SceneNode* pSceneNode) const;
		Btl::Vector<Aftk::C_SceneMeshNode*>*		GetMeshNodeList(const S32 index) const;
		Btl::Vector<Aftk::C_SceneLodGroupNode*>*	GetLODGroupNodeList(const S32 index) const;

		const bool	GenerateRandomActor(C_CrowdRandomActor& actor) const;

	private:
		const bool GenerateRandomUpper(C_CrowdRandomActor& actor) const;
		const bool GenerateRandomLower(C_CrowdRandomActor& actor) const;
		const bool GenerateRandomHead(C_CrowdRandomActor& actor) const;
		const bool GenerateRandomAccessories(C_CrowdRandomActor& actor) const;
		const bool GenerateRandomProp(C_CrowdRandomActor& actor) const;

		const bool GenerateRandomClothing(C_CrowdRandomActor& actor, const E_CrowdFlagGroup flagGroup,
			const E_CrowdAccessoryType accessoryType = E_CrowdAccessoryType_Unknown) const;

		const bool GenerateRandomSkin(C_CrowdRandomActor& actor, const E_CrowdFlagGroup flagGroup,
			const U32 skinType = 0) const;

	private:
		struct SkinnedNodeData
		{
			Btl::Vector<Aftk::C_SceneMeshNode*>	m_MeshNodes;
		};

		struct SkinnedLODGroupNodeData
		{
			Btl::Vector<Aftk::C_SceneLodGroupNode*>	m_LODGroupNodes;
		};

		mutable Ut::Vector<Aftk::C_SceneNode*>			m_SkinnedNodes; // scene nodes could be const?
		mutable Ut::Vector<SkinnedNodeData>				m_SkinnedNodeDataList;
		mutable Ut::Vector<SkinnedLODGroupNodeData>		m_SkinnedLODGroupNodeDataList;

		mutable Util::C_Random					m_Random;
	};

	///////////////////////////////////////////////////////////////////////////////////////

	class C_CrowdSpectatorData
	{
	public:
		C_CrowdSpectatorData();
		~C_CrowdSpectatorData();

		U32			Build(Aftk::C_Scene* pAftkScene, const C_CrowdPlaceholderData& placeholderData, const U32 numNodes,
						const C_CrowdSkinnedSceneDescription& desc, const U32 seed, const bool rotateY180);

		const U32							GetNumGroupNodes() const;
		const Aftk::C_SceneLodGroupNode*	GetRandomGroupNode(Util::C_Random& random);

	private:
		U32			AddMesh(const C_CrowdRandomActor& actor, const C_CrowdSkinnedSceneDescription& desc, Aftk::C_Scene* pScene,
						const C_CrowdPlaceholderData& placeholderData, const bool rotateY180);

		void		GenerateLodLevelMesh(Aftk::C_SceneLodGroupNode& groupNode, const F32 fThreshold,
						const char* pName, Aftk::C_Scene& aftkScene, const C_CrowdPlaceholderData& placeholderData, const bool rotateY180, bool bImposterMesh = false);

		void		GenerateImposterLodMesh(Aftk::C_SceneMeshNode& meshNode, Aftk::C_Scene& aftkScene,
						const C_CrowdPlaceholderData& placeholderData, const bool rotateY180);
		
		static bool	AddMeshToLodGroupNode(Aftk::C_SceneLodGroupNode* pGroupNode, Aftk::C_SceneMeshNode* pMeshNode, const U32 lodLevel);
		static bool	AddLodGroupToLodGroupNode(Aftk::C_SceneLodGroupNode* pGroupNode, Aftk::C_SceneLodGroupNode* pLodGroupNode, const U32 maxLevelsOfDetail = 0);

	private:
		Ut::Vector<Aftk::C_SceneLodGroupNode*>	m_LodGroupNodes;

		S32	m_LastRandomIndex;
	};

	///////////////////////////////////////////////////////////////////////////////////////

	class C_CrowdSpectatorDatabase
	{
	public:
		C_CrowdSpectatorDatabase();
		~C_CrowdSpectatorDatabase();

		U32		Build(Ut::Vector<Aftk::C_SceneNode*> & placeholderNodes, Ut::Vector<C_CrowdPlaceholderData> & placeholderDataList,
			Btl::Map<U32, U32> & placeholderDataMap, const C_CrowdSkinnedSceneDescription& desc, Aftk::C_Scene* pScene, const U32 seed, const bool rotateY180);
		void	End();

		C_CrowdSpectatorData*	GetSpectator(const U32 index);

	private:
		Ut::Vector<C_CrowdSpectatorData>			m_Spectators;

		mutable Util::C_Random						m_Random;
	};

	///////////////////////////////////////////////////////////////////////////////////////

	U32 Do(Aftk::C_Scene* pAftkScene, Aftk::C_SceneNode* pSceneNode, const U32 seed, const bool rotateY180);
}; // namespace ReplaceCrowdSourceNodes

#endif
