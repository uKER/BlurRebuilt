#include "stdafx.h"

#include <AftkUt/AftkUt.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <process/CrowdReplaceSourceNodes.h>

#include <system/misc/random.h>

#include <Btl/Utility/Hash.h>

//#define CROWD_DEBUG_USE_MEMBER_TYPE_AS_PALETTE_ID
//#define CROWD_REPLACE_SOURCE_NODE
//#define CROWD_REPLICATE_BASE_SCENE_MESH_NODE
//#define CROWD_DEBUG_BODY_PARTS
//#define CROWD_DEBUG_UPPER_CLOTHING_ONLY
//#define CROWD_DEBUG_LOWER_CLOTHING_ONLY
//#define CROWD_DEBUG_FORCE_RANDOMNESS
//#define CROWD_DEBUG_FORCE_PROPS
//#define CROWD_DEBUG_FORCE_LOOK_FOR_LOD_GROUPS_FIRST
#define CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
//#define CROWD_DEBUG_COPY_SOURCE_NODE
#define CROWD_DELETE_SKINNED_NODES

namespace AftkCrowdReplaceSourceNodes
{
	static const bool g_CrowdDebugSpewValidRandomActors		= false;

	static const S32 kCrowdBaseMeshDatabaseSize				= 100;

	static const S32 kCrowdActorTotalLodLevelCount			= 3;
	static const S32 kCrowdActorMeshLodLevelCount			= 2;
	static const S32 kCrowdActorImposterLodLevelIndex		= kCrowdActorTotalLodLevelCount - 1;

	static const F32 kCrowdActorLodLevelThresholds[kCrowdActorTotalLodLevelCount]
															= { 20.f, 80.f, 384.f };
	//static const F32 kCrowdActorLodLevelThresholds[kCrowdActorTotalLodLevelCount]
	//														= { 16.f, 88.f /*MPH!!*/, 384.f };
	/*static const F32 kCrowdActorLodLevelThresholds[kCrowdActorTotalLodLevelCount]
															= { 16.f, 64.f, 384.f };*/

	static const char* kCrowdActorLodLevelNames[kCrowdActorTotalLodLevelCount]
															= { "_LOD0", "_LOD1", "_IMPOSTER" };

	static const std::string kCrowdConvention				= "crowd";
	static const std::string kCrowdPlaceholderConvention	= "Placeholder";
	static const std::string kCrowdBehaviourConvention		= "Behaviour";
	static const std::string kCrowdAppearanceConvention		= "Appearance";
	static const std::string kCrowdPropTypeConvention		= "Prop";
	static const std::string kCrowdHandPropFlagsConvention	= "PropUseHands";
	static const std::string kCrowdPropOptionalConvention	= "PropOptional";

	static const std::string kCrowdBaseSkeletonConvention	= "skeleton_base";
	static const std::string kCrowdExportConvention			= "export_";

	static const std::string kCrowdConfigGender				= "Gender";
	static const std::string kCrowdConfigStyle				= "Style";
	static const std::string kCrowdConfigConditions			= "Conditions";
	static const std::string kCrowdConfigAppearance			= "Appearance";
	static const std::string kCrowdConfigSleeves			= "Sleeves";
	static const std::string kCrowdConfigNeck				= "Neck";
	static const std::string kCrowdConfigHooded				= "Hooded";
	static const std::string kCrowdConfigCropped			= "Cropped";
	static const std::string kCrowdConfigSkinType			= "SkinType";
	static const std::string kCrowdConfigSkinColour			= "ColourName";
	static const std::string kCrowdConfigHairRequired		= "HairRequired";
	static const std::string kCrowdConfigHatRequired		= "HatRequired";
	static const std::string kCrowdConfigLegsRequired		= "RequiresLegs";
	static const std::string kCrowdConfigAccessoryType		= "Type";
	static const std::string kCrowdConfigPropType			= "Type";
	static const std::string kCrowdConfigHandPropFlags		= "Flags";

	static const std::string kCrowdImposterFront			= "imposterMatFront";
	static const std::string kCrowdImposterBack				= "imposterMatBack";
	static const std::string kCrowdImposterSide				= "imposterMatSide";

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdRandomNumberGenerator::C_CrowdRandomNumberGenerator()
	{
	}

	void C_CrowdRandomNumberGenerator::Seed(const U32 seed)
	{
		m_Random.Seed(seed);
	}

	S32 C_CrowdRandomNumberGenerator::operator ()(S32 range)
	{
		return static_cast<S32>(m_Random.Rand(static_cast<U32>(range)));
	}

	///////////////////////////////////////////////////////////////////////////////////////

	/*AnimatedCrowd::CrowdBehaviourType GetCrowdBehaviourType(const Aftk::C_SceneNode &node)
	{
		std::string behaviourTypeString = AftkUt::Query::GetCrowdBehaviourTypeData(&node);
		return AnimatedCrowd::GetBehaviourTypeFromName(behaviourTypeString.c_str());
	}*/

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetFlagGroupName(E_CrowdFlagGroup flagGroup)
	{
		switch (flagGroup)
		{
			case E_FlagGroup_UpperClothing:		return "UpperClothing";
			case E_FlagGroup_LowerClothing:		return "LowerClothing";
			case E_FlagGroup_FullBodyClothing:	return "FullBodyClothing";
			case E_FlagGroup_Skin:				return "Skin";
			case E_FlagGroup_Head:				return "Head";
			case E_FlagGroup_Hair:				return "Hair";
			case E_FlagGroup_Hat:				return "Hat";
			case E_FlagGroup_Shoes:				return "Shoes";
			case E_FlagGroup_Accessory:			return "Accessory";
			case E_FlagGroup_Prop:				return "Prop";

			default: break;
		}

		return "Unknown";
	}

	E_CrowdFlagGroup GetFlagGroup(const std::string& flagGroupName)
	{
		for (S32 i = 0, count = E_FlagGroup_Count; i < count; ++i)
		{
			if (_stricmp(flagGroupName.c_str(), GetFlagGroupName(static_cast<E_CrowdFlagGroup>(i))) == 0)
				return static_cast<E_CrowdFlagGroup>(i);
		}
		return E_FlagGroup_Unknown;
	}

	bool HasValidCrowdFlagGroupName(const E_CrowdFlagGroup flagGroup)
	{
		return flagGroup != E_FlagGroup_Unknown;
	}

	bool HasValidCrowdFlagGroupName(const std::string& groupName)
	{
		return HasValidCrowdFlagGroupName(GetFlagGroup(groupName));
	}

	bool ClothingGroup(const E_CrowdFlagGroup flagGroup)
	{
		switch (flagGroup)
		{
		case E_FlagGroup_UpperClothing:
			return true;
		case E_FlagGroup_LowerClothing:
			return true;
		case E_FlagGroup_FullBodyClothing:
			return true;
		case E_FlagGroup_Hat:
			return true;
		case E_FlagGroup_Shoes:
			return true;
		case E_FlagGroup_Accessory:
			return true;
		default:
			return false;
		}
	}

	bool SkinGroup(const E_CrowdFlagGroup flagGroup)
	{
		switch (flagGroup)
		{
		case E_FlagGroup_Skin:
			return true;
		case E_FlagGroup_Head:
			return true;
		case E_FlagGroup_Hair:
			return true;
		default:
			return false;
		}
	}

	U32 GetLODLevelsByCrowdFlagGroup(const E_CrowdFlagGroup flagGroup)
	{
		switch (flagGroup)
		{
		case E_FlagGroup_UpperClothing:
		case E_FlagGroup_LowerClothing:
		case E_FlagGroup_FullBodyClothing:
			return 2;

		case E_FlagGroup_Skin:
		case E_FlagGroup_Head:
			return 2;

		case E_FlagGroup_Hair:
			return 2;

		case E_FlagGroup_Hat:
		case E_FlagGroup_Shoes:
			return 2;

		case E_FlagGroup_Accessory:
			return 0;

		case E_FlagGroup_Prop:
			return 2;

		default:
			return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdAppearanceName(E_CrowdAppearance appearance)
	{
		switch (appearance)
		{
			case E_CrowdAppearance_Generic:					return "Generic";

			case E_CrowdAppearance_Worker_Overalls:			return "Worker_Overalls";
			case E_CrowdAppearance_Worker_Overalls_J:		return "Worker_Overalls_J";
			case E_CrowdAppearance_Worker_HighVisVest:		return "Worker_HighVisVest";
			case E_CrowdAppearance_Worker_HighVisJacket:	return "Worker_HighVisJacket";

			default: break;
		}

		return "Generic";
	}

	E_CrowdAppearance GetCrowdAppearance(const std::string& appearance)
	{
		for (S32 i = 0, count = E_CrowdAppearance_Count; i < count; ++i)
		{
			if (_stricmp(appearance.c_str(), GetCrowdAppearanceName(static_cast<E_CrowdAppearance>(i))) == 0)
				return static_cast<E_CrowdAppearance>(i);
		}
		return E_CrowdAppearance_Generic;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdGenderName(E_CrowdGender gender)
	{
		switch (gender)
		{
			case E_CrowdGender_Female:	return "Female";
			case E_CrowdGender_Male:	return "Male";
			
			default: break;
		}

		return "Unisex";
	}

	E_CrowdGender GetCrowdGender(const std::string& gender)
	{
		for (S32 i = 0, count = E_CrowdGender_Count; i < count; ++i)
		{
			if (_stricmp(gender.c_str(), GetCrowdGenderName(static_cast<E_CrowdGender>(i))) == 0)
				return static_cast<E_CrowdGender>(i);
		}
		return E_CrowdGender_Unisex;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdClothingStyleName(E_CrowdClothingStyle style)
	{
		switch (style)
		{
			case E_CrowdClothingStyle_Baggy:	return "Baggy";
			case E_CrowdClothingStyle_Skinny:	return "Skinny";
			
			default: break;
		}

		return "All";
	}

	E_CrowdClothingStyle GetCrowdClothingStyle(const std::string& style)
	{
		for (S32 i = 0, count = E_CrowdClothingStyle_Count; i < count; ++i)
		{
			if (_stricmp(style.c_str(), GetCrowdClothingStyleName(static_cast<E_CrowdClothingStyle>(i))) == 0)
				return static_cast<E_CrowdClothingStyle>(i);
		}
		return E_CrowdClothingStyle_All;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdClothingConditionsName(E_CrowdClothingConditions conditions)
	{
		switch (conditions)
		{
			case E_CrowdClothingConditions_Hot:		return "Hot";
			case E_CrowdClothingConditions_Cold:	return "Cold";
			
			default: break;
		}

		return "All";
	}

	E_CrowdClothingConditions GetCrowdClothingConditions(const std::string& conditions)
	{
		for (S32 i = 0, count = E_CrowdClothingConditions_Count; i < count; ++i)
		{
			if (_stricmp(conditions.c_str(), GetCrowdClothingConditionsName(static_cast<E_CrowdClothingConditions>(i))) == 0)
				return static_cast<E_CrowdClothingConditions>(i);
		}
		return E_CrowdClothingConditions_All;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdClothingAppearanceName(E_CrowdClothingAppearance appearance)
	{
		switch (appearance)
		{
			case E_CrowdClothingAppearance_Worker_Any:				return "Worker_Any";
			case E_CrowdClothingAppearance_Worker_Overalls:			return "Worker_Overalls";
			case E_CrowdClothingAppearance_Worker_HighVisVest:		return "Worker_HighVisVest";
			case E_CrowdClothingAppearance_Worker_HighVisJacket:	return "Worker_HighVisJacket";
			
			default: break;
		}

		return "Generic";
	}

	E_CrowdClothingAppearance GetCrowdClothingAppearance(const std::string& appearance)
	{
		for (S32 i = 0, count = E_CrowdClothingAppearance_Count; i < count; ++i)
		{
			if (_stricmp(appearance.c_str(), GetCrowdClothingAppearanceName(static_cast<E_CrowdClothingAppearance>(i))) == 0)
				return static_cast<E_CrowdClothingAppearance>(i);
		}
		return E_CrowdClothingAppearance_Generic;
	}

	bool ValidClothingAppearance(const E_CrowdAppearance appearance, const E_CrowdClothingAppearance clothingAppearance)
	{
		bool valid = true;

		switch (appearance)
		{
		case E_CrowdAppearance_Generic:
			{
				switch (clothingAppearance)
				{
				case E_CrowdClothingAppearance_Generic:
					break;
				case E_CrowdClothingAppearance_Worker_Any:
				case E_CrowdClothingAppearance_Worker_Overalls:
				case E_CrowdClothingAppearance_Worker_HighVisVest:
				case E_CrowdClothingAppearance_Worker_HighVisJacket:
					{
						valid = false;
					}
					break;
				default:
					break;
				}
			}
			break;
		case E_CrowdAppearance_Worker_Overalls:
			{
				switch (clothingAppearance)
				{
				case E_CrowdClothingAppearance_Generic:
					{
						valid = false;
					}
					break;
				case E_CrowdClothingAppearance_Worker_Any:
				case E_CrowdClothingAppearance_Worker_Overalls:
					break;
				case E_CrowdClothingAppearance_Worker_HighVisVest:
				case E_CrowdClothingAppearance_Worker_HighVisJacket:
					{
						valid = false;
					}
					break;
				default:
					break;
				}
			}
			break;
		case E_CrowdAppearance_Worker_Overalls_J:
			{
				switch (clothingAppearance)
				{
				case E_CrowdClothingAppearance_Generic:
					{
						valid = false;
					}
					break;
				case E_CrowdClothingAppearance_Worker_Any:
				case E_CrowdClothingAppearance_Worker_Overalls:
					break;
				case E_CrowdClothingAppearance_Worker_HighVisVest:
				case E_CrowdClothingAppearance_Worker_HighVisJacket:
					{
						valid = false;
					}
					break;
				default:
					break;
				}
			}
			break;
		case E_CrowdAppearance_Worker_HighVisVest:
			{
				switch (clothingAppearance)
				{
				case E_CrowdClothingAppearance_Generic:
					{
						valid = false;
					}
					break;
				case E_CrowdClothingAppearance_Worker_Any:
					break;
				case E_CrowdClothingAppearance_Worker_Overalls:
					{
						valid = false;
					}
					break;
				case E_CrowdClothingAppearance_Worker_HighVisVest:
					break;
				case E_CrowdClothingAppearance_Worker_HighVisJacket:
					{
						valid = false;
					}
					break;
				default:
					break;
				}
			}
			break;
		case E_CrowdAppearance_Worker_HighVisJacket:
			{
				switch (clothingAppearance)
				{
				case E_CrowdClothingAppearance_Generic:
					{
						valid = false;
					}
					break;
				case E_CrowdClothingAppearance_Worker_Any:
					break;
				case E_CrowdClothingAppearance_Worker_Overalls:
				case E_CrowdClothingAppearance_Worker_HighVisVest:
					{
						valid = false;
					}
					break;
				case E_CrowdClothingAppearance_Worker_HighVisJacket:
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
		
		return valid;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdSleeveTypeName(E_CrowdSleeveType type)
	{
		switch (type)
		{
			case E_CrowdSleeveType_Long:
				{
					return "Long";
				}
			case E_CrowdSleeveType_Short:
			default:
				{
					return "Short";
				}
		}
	}

	E_CrowdSleeveType GetCrowdSleeveType(const std::string& type)
	{
		for (S32 i = 0, count = E_CrowdSleeveType_Count; i < count; ++i)
		{
			if (_stricmp(type.c_str(), GetCrowdSleeveTypeName(static_cast<E_CrowdSleeveType>(i))) == 0)
				return static_cast<E_CrowdSleeveType>(i);
		}
		return E_CrowdSleeveType_Short;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdNeckTypeName(E_CrowdNeckType type)
	{
		switch (type)
		{
			case E_CrowdNeckType_Open:
				{
					return "Open";
				}
			case E_CrowdNeckType_Strap:
				{
					return "Strap";
				}
			case E_CrowdNeckType_Closed:
			default:
				{
					return "Closed";
				}
		}
	}

	E_CrowdNeckType GetCrowdNeckType(const std::string& type)
	{
		for (S32 i = 0, count = E_CrowdNeckType_Count; i < count; ++i)
		{
			if (_stricmp(type.c_str(), GetCrowdNeckTypeName(static_cast<E_CrowdNeckType>(i))) == 0)
				return static_cast<E_CrowdNeckType>(i);
		}
		return E_CrowdNeckType_Closed;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdSkinTypeName(E_CrowdSkinType type)
	{
		switch (type)
		{
			case E_CrowdSkinType_ShortArms:
				{
					return "ShortArms";
				}
			case E_CrowdSkinType_LongArms:
				{
					return "LongArms";
				}
			case E_CrowdSkinType_Neck:
				{
					return "Neck";
				}
			case E_CrowdSkinType_Bust:
				{
					return "Bust";
				}
			case E_CrowdSkinType_Stomach:
				{
					return "Stomach";
				}
			case E_CrowdSkinType_Legs:
				{
					return "Legs";
				}
			default:
				{
					return "Unknown";
				}
		}
	}

	E_CrowdSkinType GetCrowdSkinType(const std::string& type)
	{
		if (_stricmp(type.c_str(), GetCrowdSkinTypeName(E_CrowdSkinType_ShortArms)) == 0)
			return E_CrowdSkinType_ShortArms;
		if (_stricmp(type.c_str(), GetCrowdSkinTypeName(E_CrowdSkinType_LongArms)) == 0)
			return E_CrowdSkinType_LongArms;
		if (_stricmp(type.c_str(), GetCrowdSkinTypeName(E_CrowdSkinType_Neck)) == 0)
			return E_CrowdSkinType_Neck;
		if (_stricmp(type.c_str(), GetCrowdSkinTypeName(E_CrowdSkinType_Bust)) == 0)
			return E_CrowdSkinType_Bust;
		if (_stricmp(type.c_str(), GetCrowdSkinTypeName(E_CrowdSkinType_Stomach)) == 0)
			return E_CrowdSkinType_Stomach;
		if (_stricmp(type.c_str(), GetCrowdSkinTypeName(E_CrowdSkinType_Legs)) == 0)
			return E_CrowdSkinType_Legs;

		return E_CrowdSkinType_Unknown;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdIncludeHairName(E_CrowdIncludeHair flag)
	{
		switch (flag)
		{
			case E_CrowdIncludeHair_NotFussed:
				{
					return "NotFussed";
				}
			case E_CrowdIncludeHair_No:
				{
					return "No";
				}
			default:
				{
					return "Yes";
				}
		}
	}

	E_CrowdIncludeHair GetCrowdIncludeHairFlag(const std::string& flag)
	{
		for (S32 i = 0, count = E_CrowdIncludeHair_Count; i < count; ++i)
		{
			if (_stricmp(flag.c_str(), GetCrowdIncludeHairName(static_cast<E_CrowdIncludeHair>(i))) == 0)
				return static_cast<E_CrowdIncludeHair>(i);
		}
		return E_CrowdIncludeHair_Yes;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdAccessoryTypeName(E_CrowdAccessoryType type)
	{
		switch (type)
		{
			case E_CrowdAccessoryType_Glasses:
				{
					return "Glasses";
				}
			case E_CrowdAccessoryType_Scarf:
				{
					return "Scarf";
				}
			case E_CrowdAccessoryType_Watch:
				{
					return "Watch";
				}
			case E_CrowdAccessoryType_Visor:
				{
					return "Visor";
				}
			default:
				{
					return "Unknown";
				}
		}
	}

	E_CrowdAccessoryType GetCrowdAccessoryType(const std::string& type)
	{
		for (S32 i = 0, count = E_CrowdAccessoryType_Count; i < count; ++i)
		{
			if (_stricmp(type.c_str(), GetCrowdAccessoryTypeName(static_cast<E_CrowdAccessoryType>(i))) == 0)
				return static_cast<E_CrowdAccessoryType>(i);
		}
		return E_CrowdAccessoryType_Unknown;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdPropTypeName(E_CrowdPropType type)
	{
		switch (type)
		{
			case E_CrowdPropType_None:
				{
					return "None";
				}
			case E_CrowdPropType_SprayCan:
				{
					return "SprayCan";
				}
			case E_CrowdPropType_Skateboard:
				{
					return "Skateboard";
				}
			case E_CrowdPropType_BMX:
				{
					return "BMX";
				}
			case E_CrowdPropType_GlowStick:
				{
					return "GlowStick";
				}
			case E_CrowdPropType_CellPhone:
				{
					return "CellPhone";
				}
			case E_CrowdPropType_FlagPole:
				{
					return "FlagPole";
				}
			default:
				{
					return "None";
				}
		}
	}

	E_CrowdPropType GetCrowdPropType(const std::string& type)
	{
		for (S32 i = 0, count = E_CrowdPropType_Count; i < count; ++i)
		{
			if (_stricmp(type.c_str(), GetCrowdPropTypeName(static_cast<E_CrowdPropType>(i))) == 0)
				return static_cast<E_CrowdPropType>(i);
		}
		return E_CrowdPropType_None;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	inline const char* GetCrowdHandPropFlagName(E_CrowdHandPropFlags flag)
	{
		switch (flag)
		{
			case E_CrowdHandPropFlag_None:
				{
					return "None";
				}
			case E_CrowdHandPropFlag_Left:
				{
					return "Left";
				}
			case E_CrowdHandPropFlag_Right:
				{
					return "Right";
				}
			case E_CrowdHandPropFlag_Both:
				{
					return "Both";
				}
			default:
				{
					return "None";
				}
		}
	}

	E_CrowdHandPropFlags GetCrowdHandPropFlags(const std::string& flag)
	{
		for (S32 i = 0, count = E_CrowdHandPropFlag_Count; i < count; ++i)
		{
			if (_stricmp(flag.c_str(), GetCrowdHandPropFlagName(static_cast<E_CrowdHandPropFlags>(i))) == 0)
				return static_cast<E_CrowdHandPropFlags>(i);
		}
		return E_CrowdHandPropFlag_None;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	template <class T>
	bool Contains(const Ut::Vector<T> & list, T item, U32* pIndex)
	{
		for (U32 i = 0, count = static_cast<U32>(list.Size()); i < count; ++i)
		{
			if (list[i] == item)
			{
				if (pIndex)
					*pIndex = i;
				return true;
			}
		}

		return false;
	}

	void SetIgnoredInstanceNodeFlags(Aftk::C_SceneInstanceNode* pNode)
	{
		AftkUt::C_NodeFlagParser instNodeFlags(*pNode);
		instNodeFlags.SetParameterValue("UseSHLighting", (bool)true);
		instNodeFlags.DeleteParameter("UseLightmapLighting");
		instNodeFlags.DeleteParameter("UseVertexLighting");
	}

	static S32 iCrowdInstanceIndex = 0;

	void LocateCrowdNodes(Ut::Vector<Aftk::C_SceneNode*> & placeholderNodes, /*Ut::Vector<Aftk::C_SceneNode*> & placeholderDummyNodes,*/
						  Aftk::C_SceneNode* pSceneNode)
	{
		AftkUt::Query::C_Flags flags = AftkUt::Query::GetConventionFlags(pSceneNode);

		if (flags.asCrowd)
		{
			if (!Contains(placeholderNodes, pSceneNode, NULL))
			{
				//LOGINFO( Tools::g_log, "[Crowd] %s - %d", pSceneNode->GetName().c_str(), pSceneNode->GetNumInstancesOfThisNode() );
				placeholderNodes.PushBack(pSceneNode);

				/*Aftk::C_SceneNode* pDummyNode = new Aftk::C_SceneNode(*pSceneNode);
				pDummyNode->SetName(pSceneNode->GetName() + "_empty");

				pSceneNode->GetParentPtr()->AddChildPtr(pDummyNode);

				placeholderDummyNodes.PushBack(pDummyNode);*/

				for (S32 i = 0, count = pSceneNode->GetNumInstancesOfThisNode(); i < count; ++i)
				{
					Aftk::C_SceneNode* pInstance = pSceneNode->GetInstanceOfThisNode(i);
					//AftkUt::CrowdHelper::SetCrowdInstanceIndex(pInstance, iCrowdInstanceIndex++);
					SetIgnoredInstanceNodeFlags((Aftk::C_SceneInstanceNode*)pInstance);
				}
			}
		}

		for (U32 i = 0, count = pSceneNode->GetNumChildren(); i < count; ++i)
			LocateCrowdNodes(placeholderNodes, /*placeholderDummyNodes,*/ pSceneNode->GetChildPtr(i));
	}

	void LocateSkinnedMeshNodes(Ut::Vector<Aftk::C_SceneNode*> & skinnedMeshNodes, Aftk::C_SceneNode* pSceneNode)
	{
		if (!pSceneNode)
			return;

		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
		{
			if (((Aftk::C_SceneMeshNode*)pSceneNode)->IsSkinned())
			{
				if (!Contains(skinnedMeshNodes, pSceneNode, NULL))
					skinnedMeshNodes.PushBack(pSceneNode);
				else
					LOGDEBUG( Tools::g_log, "[Crowd] Already processed scene node '%s'", pSceneNode->GetName().c_str() );
			}
		}

		if (pSceneNode->GetNumChildren() > 0)
		{
			for (U32 i = 0, count = pSceneNode->GetNumChildren(); i < count; ++i)
				LocateSkinnedMeshNodes(skinnedMeshNodes, pSceneNode->GetChildPtr(i));
		}
	}

	void LocateSkinnedNodes(Ut::Vector<Aftk::C_SceneNode*> & skinnedNodes, Aftk::C_SceneNode* pSceneNode,
		const bool locateMeshNodes)
	{
		if (!pSceneNode)
			return;

		bool status;

		AftkUt::C_NodeFlagParser nodeParser(*pSceneNode, &status);

		if (status)
		{
			if (!AftkUt::Query::IsInstance(pSceneNode) && nodeParser.GetFlagGroupCategory() == "Crowd")
			{
				if (HasValidCrowdFlagGroupName(nodeParser.GetFlagGroupName()))
				{
					if (locateMeshNodes)
						LocateSkinnedMeshNodes(skinnedNodes, pSceneNode);
					else
						skinnedNodes.PushBack(pSceneNode);
				}
			}
		}

		for (U32 i = 0, count = pSceneNode->GetNumChildren(); i < count; ++i)
			LocateSkinnedNodes(skinnedNodes, pSceneNode->GetChildPtr(i), locateMeshNodes);
	}

	void TagSkinnedNodeAsBaseInstance(Aftk::C_SceneNode* pSceneNode)
	{
		bool status;

		AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &status);

		if (!status)
			return;

		nodeFlags.SetFlagGroupCategory("System");
		nodeFlags.SetFlagGroupName("BaseInstance");
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdImposterInfo::C_CrowdImposterInfo()
	{ }

	const bool C_CrowdImposterInfo::IsValid() const
	{
		bool valid = true;

		for (U32 i = 0, count = C_CrowdImposterInfo::E_ImposterInfo_Count; i < count; ++i)
		{
			if (!ImagePath[i].length())
				return false;
		}

		return valid;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdPlaceholderData::C_CrowdPlaceholderData()
		: ParentPtr(NULL),
		BehaviourHash(0),
		Appearance(E_CrowdAppearance_Generic),
		PropType(E_CrowdPropType_None),
		PropHandFlags(E_CrowdHandPropFlag_Both),
		PropOptional(false),
		SecondaryPropType(E_CrowdPropType_None),
		SecondaryPropHandFlags(E_CrowdHandPropFlag_Both),
		SecondaryPropOptional(false),
		SecondaryBehaviourHash(0),
		NumInstances(0)
	{ }

	void C_CrowdPlaceholderData::Create(Aftk::C_SceneNode* pSceneNode)
	{
		BIZ_ASSERT(pSceneNode);

		bool bStatus = false;
		AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &bStatus);

		if (bStatus)
		{
			if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), kCrowdConvention.c_str()) == 0)
			{
				std::string groupName = nodeFlags.GetFlagGroupName();

				if (_stricmp(groupName.c_str(), kCrowdPlaceholderConvention.c_str()) == 0)
				{
					ParentPtr = pSceneNode->GetParentPtr();
					
					Name = pSceneNode->GetName().c_str();
					
					// Behaviour
					{
						std::string behaviourString;
					
						if(nodeFlags.GetParameterValue(kCrowdBehaviourConvention.c_str(), behaviourString))
							BehaviourHash = Btl::Hash(behaviourString.c_str());
					}

					// Appearance
					{
						std::string appearanceString;
					
						if(nodeFlags.GetParameterValue(kCrowdAppearanceConvention.c_str(), appearanceString))
							Appearance = GetCrowdAppearance(appearanceString);
					}

					// PropType
					{
						std::string propTypeString;
					
						if(nodeFlags.GetParameterValue(kCrowdPropTypeConvention.c_str(), propTypeString))
							PropType = GetCrowdPropType(propTypeString);
					}

					// PropHandFlags
					{
						std::string propHandFlagsString;
					
						if(nodeFlags.GetParameterValue(kCrowdHandPropFlagsConvention.c_str(), propHandFlagsString))
							PropHandFlags = GetCrowdHandPropFlags(propHandFlagsString);
					}

					// PropOptional
					{
						bool propOptional = false;
					
						if(nodeFlags.GetParameterValue(kCrowdPropOptionalConvention.c_str(), propOptional))
							PropOptional = propOptional;
					}

					// SecondaryPropType
					{
						std::string propTypeString;
					
						if(nodeFlags.GetParameterValue(kCrowdPropTypeConvention.c_str(), propTypeString))
							SecondaryPropType = GetCrowdPropType(propTypeString);
					}
					
					// SecondaryPropHandFlags
					{
						std::string propHandFlagsString;
					
						if(nodeFlags.GetParameterValue(kCrowdHandPropFlagsConvention.c_str(), propHandFlagsString))
							SecondaryPropHandFlags = GetCrowdHandPropFlags(propHandFlagsString);
					}
					
					// SecondaryPropOptional
					{
						bool propOptional = false;
					
						if(nodeFlags.GetParameterValue(kCrowdPropOptionalConvention.c_str(), propOptional))
							SecondaryPropOptional = propOptional;
					}

					// SecondaryBehaviour
					{
						std::string behaviourString;
					
						if(nodeFlags.GetParameterValue(kCrowdBehaviourConvention.c_str(), behaviourString))
							SecondaryBehaviourHash = Btl::Hash(behaviourString.c_str());
					}

					Bounds = pSceneNode->GetEvaluatedWorldBounds();

					// Imposter image paths
					if (pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
					{
						Aftk::C_SceneMeshNode* pMeshNode = (Aftk::C_SceneMeshNode*)pSceneNode;

						U16 numMaterials = pMeshNode->GetNumPolygonMaterialMappings();

						for (U16 i = 0, count = numMaterials; i < count; ++i)
						{
							const Aftk::C_MaterialResource* pMaterialResource = pMeshNode->GetPolygonMaterialMappingPtr(static_cast<S16>(i));

							BIZ_ASSERT(pMaterialResource);

							const Aftk::C_MaterialNode* pMaterial = &pMaterialResource->GetContent();

							C_CrowdImposterInfo::E_ImposterInfo imposterInfo = C_CrowdImposterInfo::E_ImposterInfo_Unknown;

							if (pMaterial && pMaterial->GetNumShaderNodes())
							{
								if (pMaterial->GetName().find(kCrowdImposterFront) != std::string::npos)
									imposterInfo = C_CrowdImposterInfo::E_ImposterInfo_Front;
								else if (pMaterial->GetName().find(kCrowdImposterBack) != std::string::npos)
									imposterInfo = C_CrowdImposterInfo::E_ImposterInfo_Back;
								else if (pMaterial->GetName().find(kCrowdImposterSide) != std::string::npos)
									imposterInfo = C_CrowdImposterInfo::E_ImposterInfo_Side;

								if (pMaterial->GetNumShaderImageNodes() && imposterInfo != C_CrowdImposterInfo::E_ImposterInfo_Unknown)
								{
									const Aftk::C_ShaderImageNode* pShaderImageNode = pMaterial->GetShaderImageNodePtr(0);

									BIZ_ASSERT(pShaderImageNode);

									std::string imagePath;
									pShaderImageNode->GetFilePlug().GetValue(imagePath);
									ImposterInfo.ImagePath[imposterInfo] = imagePath;
								}
							}
						}
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdRandomActorDesc::C_CrowdRandomActorDesc()
		: m_Female(true),
		m_Cold(true),
		m_Style(E_CrowdClothingStyle_All),
		m_iFlags(0),
		m_SleeveType(E_CrowdSleeveType_Short),
		m_NeckType(E_CrowdNeckType_Closed),
		m_Hooded(false),
		m_Cropped(false),
		m_ColourDesc(""),
		m_IncludeHair(E_CrowdIncludeHair_Yes),
		m_RequiresLegs(false),
		m_RequiresHat(false),
		m_UseSecondaryBehaviour(false)
	{
		m_Random.Seed(0);
	}

	void C_CrowdRandomActorDesc::Seed(const U32 seed, const E_CrowdAppearance appearance)
	{
		m_Random.Seed(seed);

#if (!defined(CROWD_DEBUG_FORCE_RANDOMNESS) && (defined(CROWD_DEBUG_BODY_PARTS) || defined(CROWD_DEBUG_UPPER_CLOTHING_ONLY) || defined(CROWD_DEBUG_LOWER_CLOTHING_ONLY)))
		m_Female		= false;
		m_Cold			= true;
		m_Appearance	= appearance;
		m_Style			= E_CrowdClothingStyle_Baggy;
#else
		m_Female		= (m_Random.FRand() <= 0.5f);
		m_Cold			= (m_Random.FRand() <= 0.5f);
		m_Appearance	= appearance;
		m_Style			= (m_Random.FRand() <= 0.5f) ? E_CrowdClothingStyle_Baggy : E_CrowdClothingStyle_Skinny;

		switch (appearance)
		{
		case E_CrowdAppearance_Generic:
			break;
		case E_CrowdAppearance_Worker_Overalls:
		case E_CrowdAppearance_Worker_Overalls_J:
		case E_CrowdAppearance_Worker_HighVisVest:
		case E_CrowdAppearance_Worker_HighVisJacket:
			{
				m_Female = false;
			}
			break;
		default:
			break;
		}
#endif
	}

	Util::C_Random& C_CrowdRandomActorDesc::GetRandom()
	{
		return m_Random;
	}

	const bool C_CrowdRandomActorDesc::Female() const
	{
		return m_Female;
	}

	const bool C_CrowdRandomActorDesc::Cold() const
	{
		return m_Cold;
	}

	const E_CrowdAppearance C_CrowdRandomActorDesc::Appearance() const
	{
		return m_Appearance;
	}

	const E_CrowdClothingStyle C_CrowdRandomActorDesc::Style() const
	{
		return m_Style;
	}

	const bool C_CrowdRandomActorDesc::UpperClothing() const
	{
		return (m_UpperClothing > 0);
	}

	void C_CrowdRandomActorDesc::UpperClothing(const bool b)
	{
		m_UpperClothing = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Arms() const
	{
		return (m_Arms > 0);
	}

	void C_CrowdRandomActorDesc::Arms(const bool b)
	{
		m_Arms = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Torso() const
	{
		return (m_Torso > 0);
	}

	void C_CrowdRandomActorDesc::Torso(const bool b)
	{
		m_Torso = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Stomach() const
	{
		return (m_Stomach > 0);
	}

	void C_CrowdRandomActorDesc::Stomach(const bool b)
	{
		m_Stomach = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::LowerClothing() const
	{
		return (m_LowerClothing > 0);
	}

	void C_CrowdRandomActorDesc::LowerClothing(const bool b)
	{
		m_LowerClothing = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Legs() const
	{
		return (m_Legs > 0);
	}

	void C_CrowdRandomActorDesc::Legs(const bool b)
	{
		m_Legs = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Shoes() const
	{
		return (m_Shoes > 0);
	}

	void C_CrowdRandomActorDesc::Shoes(const bool b)
	{
		m_Shoes = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Head() const
	{
		return (m_Head > 0);
	}

	void C_CrowdRandomActorDesc::Head(const bool b)
	{
		m_Head = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Hair() const
	{
		return (m_Hair > 0);
	}

	void C_CrowdRandomActorDesc::Hair(const bool b)
	{
		m_Hair = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Hat() const
	{
		return (m_Hat > 0);
	}

	void C_CrowdRandomActorDesc::Hat(const bool b)
	{
		m_Hat = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Glasses() const
	{
		return (m_Glasses > 0);
	}

	void C_CrowdRandomActorDesc::Glasses(const bool b)
	{
		m_Glasses = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Scarf() const
	{
		return (m_Scarf > 0);
	}

	void C_CrowdRandomActorDesc::Scarf(const bool b)
	{
		m_Scarf = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Watch() const
	{
		return (m_Watch > 0);
	}

	void C_CrowdRandomActorDesc::Watch(const bool b)
	{
		m_Watch = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Visor() const
	{
		return (m_Visor > 0);
	}

	void C_CrowdRandomActorDesc::Visor(const bool b)
	{
		m_Visor = b ? 1 : 0;
	}

	const bool C_CrowdRandomActorDesc::Prop() const
	{
		return (m_Prop > 0);
	}

	void C_CrowdRandomActorDesc::Prop(const bool b)
	{
		m_Prop = b ? 1 : 0;
	}

	const E_CrowdSleeveType C_CrowdRandomActorDesc::SleeveType() const
	{
		return m_SleeveType;
	}

	void C_CrowdRandomActorDesc::SleeveType(const E_CrowdSleeveType type)
	{
		m_SleeveType = type;
	}

	const E_CrowdNeckType C_CrowdRandomActorDesc::NeckType() const
	{
		return m_NeckType;
	}

	void C_CrowdRandomActorDesc::NeckType(const E_CrowdNeckType type)
	{
		m_NeckType = type;
	}

	const bool C_CrowdRandomActorDesc::Hooded() const
	{
		return m_Hooded;
	}

	void C_CrowdRandomActorDesc::Hooded(const bool hooded)
	{
		m_Hooded = hooded;
	}

	const bool C_CrowdRandomActorDesc::Cropped() const
	{
		return m_Cropped;
	}

	void C_CrowdRandomActorDesc::Cropped(const bool cropped)
	{
		m_Cropped = cropped;
	}

	const Util::C_String& C_CrowdRandomActorDesc::ColourDesc() const
	{
		return m_ColourDesc;
	}

	void C_CrowdRandomActorDesc::ColourDesc(const Util::C_String& desc)
	{
		m_ColourDesc = desc;
	}

	const E_CrowdIncludeHair C_CrowdRandomActorDesc::IncludeHair() const
	{
		return m_IncludeHair;
	}

	void C_CrowdRandomActorDesc::IncludeHair(const E_CrowdIncludeHair includeHair)
	{
		m_IncludeHair = includeHair;
	}

	const bool C_CrowdRandomActorDesc::RequiresLegs() const
	{
		return m_RequiresLegs;
	}

	void C_CrowdRandomActorDesc::RequiresLegs(const bool requiresLegs)
	{
		m_RequiresLegs = requiresLegs;
	}

	const bool C_CrowdRandomActorDesc::RequiresHat()
	{
		if (m_RequiresHat)
			return true;
		else
		{
			bool requiresHat = false;
			
			switch (m_Appearance)
			{
			case E_CrowdAppearance_Generic:
				break;
			case E_CrowdAppearance_Worker_Overalls:
			case E_CrowdAppearance_Worker_Overalls_J:
			case E_CrowdAppearance_Worker_HighVisVest:
			case E_CrowdAppearance_Worker_HighVisJacket:
				{
					requiresHat = (m_Random.FRand() <= 0.5f);
				}
				break;
			default:
				break;
			}

			return requiresHat;
		}
	}

	void C_CrowdRandomActorDesc::RequiresHat(const bool requiresHat)
	{
		m_RequiresHat = requiresHat;
	}

	const E_CrowdPropType C_CrowdRandomActorDesc::PropType() const
	{
		return m_PropType;
	}

	void C_CrowdRandomActorDesc::PropType(const E_CrowdPropType type)
	{
		m_PropType = type;
	}

	const E_CrowdHandPropFlags C_CrowdRandomActorDesc::PropHandFlags() const
	{
		return m_PropHandFlag;
	}

	void C_CrowdRandomActorDesc::PropHandFlags(const E_CrowdHandPropFlags flag)
	{
		m_PropHandFlag = flag;
	}

	const bool C_CrowdRandomActorDesc::UseSecondaryBehaviour() const
	{
		return m_UseSecondaryBehaviour;
	}

	void C_CrowdRandomActorDesc::UseSecondaryBehaviour(const bool useSecondaryBehaviour)
	{
		m_UseSecondaryBehaviour = useSecondaryBehaviour;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdRandomActor::C_CrowdRandomActor(const U32 seed, const E_CrowdAppearance appearance)
	{
		m_ActorDesc.Seed(seed, appearance);
	}

	bool C_CrowdRandomActor::AddActorPart(Aftk::C_SceneNode* pSceneNode)
	{
		if (!pSceneNode)
			return false;

		bool status = false;

		AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &status);

		if (!status)
			return false;

		if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "Crowd") == 0)
		{
			E_CrowdFlagGroup flagGroup = GetFlagGroup(nodeFlags.GetFlagGroupName());
			
			switch (flagGroup)
			{
			case E_FlagGroup_UpperClothing:
				{
					if (!UpdateUpperClothingFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_LowerClothing:
				{
					if (!UpdateLowerClothingFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_FullBodyClothing:
				{
					if (!UpdateUpperClothingFlags(nodeFlags))
						return false;
					if (!UpdateLowerClothingFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Skin:
				{
					if (!UpdateSkinFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Head:
				{
					if (!UpdateHeadFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Hair:
				{
					if (!UpdateHairFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Hat:
				{
					if (!UpdateHatFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Shoes:
				{
					if (!UpdateShoeFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Accessory:
				{
					if (!UpdateAccessoryFlags(nodeFlags))
						return false;
				}
				break;
			case E_FlagGroup_Prop:
				{
					if (!UpdatePropFlags(nodeFlags))
						return false;
				}
				break;
			default:
				return false;
			}

			m_ActorParts.PushBack(pSceneNode);

			return true;
		}

		return false;
	}

	const S32 C_CrowdRandomActor::GetNumActorParts() const
	{
		return m_ActorParts.Size();
	}

	Aftk::C_SceneNode* C_CrowdRandomActor::GetActorPart(const S32 index) const
	{
		if (index >= 0 && index < m_ActorParts.Size())
			return m_ActorParts[index];
		return NULL;
	}

	C_CrowdRandomActorDesc& C_CrowdRandomActor::GetActorDesc()
	{
		return m_ActorDesc;
	}

	const C_CrowdRandomActorDesc& C_CrowdRandomActor::GetActorDesc() const
	{
		return m_ActorDesc;
	}

	void C_CrowdRandomActor::SetName(const Util::C_String& name)
	{
		m_Name = name;
	}

	const Util::C_String& C_CrowdRandomActor::GetName() const
	{
		return m_Name;
	}

	bool C_CrowdRandomActor::UpdateUpperClothingFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		std::string sleevesString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigSleeves, sleevesString))
			return false;

		m_ActorDesc.SleeveType(GetCrowdSleeveType(sleevesString));

		std::string neckString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigNeck, neckString))
			return false;

		m_ActorDesc.NeckType(GetCrowdNeckType(neckString));

		std::string hoodedString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigHooded, hoodedString))
			return false;

		if (_stricmp(hoodedString.c_str(), "0") == 0 || _stricmp(hoodedString.c_str(), "false") == 0)
			m_ActorDesc.Hooded(false);
		else
			m_ActorDesc.Hooded(true);

		std::string croppedString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigCropped, croppedString))
			return false;

		if (_stricmp(croppedString.c_str(), "0") == 0 || _stricmp(croppedString.c_str(), "false") == 0)
			m_ActorDesc.Cropped(false);
		else
			m_ActorDesc.Cropped(true);

		m_ActorDesc.UpperClothing(true);

		return true;
	}

	bool C_CrowdRandomActor::UpdateSkinFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		std::string skinColourString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigSkinColour, skinColourString))
			return false;

		if (m_ActorDesc.ColourDesc().Length() < 1)
			m_ActorDesc.ColourDesc(Util::C_String(skinColourString.c_str()));

		std::string skinTypeString;
		if (nodeFlags.GetParameterValue(kCrowdConfigSkinType, skinTypeString))
		{
			E_CrowdSkinType type = GetCrowdSkinType(skinTypeString);

			if (type == E_CrowdSkinType_ShortArms)
				m_ActorDesc.Arms(true);
			if (type == E_CrowdSkinType_LongArms)
				m_ActorDesc.Arms(true);
			if (type == E_CrowdSkinType_Neck)
				m_ActorDesc.Torso(true);
			if (type == E_CrowdSkinType_Bust)
				m_ActorDesc.Torso(true);
			if (type == E_CrowdSkinType_Stomach)
				m_ActorDesc.Stomach(true);
			if (type == E_CrowdSkinType_Legs)
				m_ActorDesc.Legs(true);
		}

		return true;
	}

	bool C_CrowdRandomActor::UpdateLowerClothingFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		std::string requiresLegsString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigLegsRequired, requiresLegsString))
		{
			m_ActorDesc.RequiresLegs(false);
		}
		else
		{
			if (_stricmp(requiresLegsString.c_str(), "0") == 0 || _stricmp(requiresLegsString.c_str(), "false") == 0)
				m_ActorDesc.RequiresLegs(false);
			else
				m_ActorDesc.RequiresLegs(true);
		}

		m_ActorDesc.LowerClothing(true);

		return true;
	}

	bool C_CrowdRandomActor::UpdateHeadFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		std::string requiresHairString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigHairRequired, requiresHairString))
		{
			m_ActorDesc.IncludeHair(E_CrowdIncludeHair_No);
		}
		else
		{
			m_ActorDesc.IncludeHair(GetCrowdIncludeHairFlag(requiresHairString));
		}

		m_ActorDesc.Head(true);

		return true;
	}

	bool C_CrowdRandomActor::UpdateHairFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		std::string requiresHatString;
		if (!nodeFlags.GetParameterValue(kCrowdConfigHatRequired, requiresHatString))
		{
			m_ActorDesc.RequiresHat(false);
			m_ActorDesc.Hat(true);
		}
		else
		{
			if (_stricmp(requiresHatString.c_str(), "0") == 0 || _stricmp(requiresHatString.c_str(), "false") == 0)
			{
				m_ActorDesc.RequiresHat(false);
				m_ActorDesc.Hat(true);
			}
			else
			{
				m_ActorDesc.RequiresHat(true);
			}
		}

		m_ActorDesc.Hair(true);

		return true;
	}

	bool C_CrowdRandomActor::UpdateHatFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		m_ActorDesc.Hat(true);

		return true;
	}

	bool C_CrowdRandomActor::UpdateShoeFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		m_ActorDesc.Shoes(true);

		return true;
	}

	bool C_CrowdRandomActor::UpdateAccessoryFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		std::string accessoryTypeString;
		if (nodeFlags.GetParameterValue(kCrowdConfigAccessoryType, accessoryTypeString))
		{
			E_CrowdAccessoryType type = GetCrowdAccessoryType(accessoryTypeString);

			if (type == E_CrowdAccessoryType_Glasses)
				m_ActorDesc.Glasses(true);
			if (type == E_CrowdAccessoryType_Scarf)
				m_ActorDesc.Scarf(true);
			if (type == E_CrowdAccessoryType_Watch)
				m_ActorDesc.Watch(true);
			if (type == E_CrowdAccessoryType_Visor)
				m_ActorDesc.Visor(true);
		}

		return true;
	}

	bool C_CrowdRandomActor::UpdatePropFlags(AftkUt::C_NodeFlagParser& nodeFlags)
	{
		m_ActorDesc.Prop(true);

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	void LocateChildMeshNodes(Aftk::C_SceneNode* pSceneNode, Btl::Vector<Aftk::C_SceneMeshNode*> & meshNodes)
	{
		if (!pSceneNode)
			return;

		if (!pSceneNode->GetNumChildren())
		{
			if (!pSceneNode->HasType(Aftk::E_ObjType_SceneLodGroup))
			{
				if (pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
				{
					if (((Aftk::C_SceneMeshNode*)pSceneNode)->IsSkinned())
					{
						meshNodes.PushBack((Aftk::C_SceneMeshNode*)pSceneNode);
					}
				}
			}
		}
		else
		{
			for (S32 i = 0, count = pSceneNode->GetNumChildren(); i < count; ++i)
			{
				Aftk::C_SceneNode* pChildNode = pSceneNode->GetChildPtr(i);

				if (!pChildNode)
					continue;

				if (!pChildNode->HasType(Aftk::E_ObjType_SceneLodGroup))
				{
					if (pChildNode->HasType(Aftk::E_ObjType_SceneMesh))
					{
						if (((Aftk::C_SceneMeshNode*)pChildNode)->IsSkinned())
						{
							meshNodes.PushBack((Aftk::C_SceneMeshNode*)pChildNode);
						}
					}
					else
					{
						for (S32 j = 0, childCount = pChildNode->GetNumChildren(); j < childCount; ++j)
						{
							LocateChildMeshNodes(pChildNode->GetChildPtr(j), meshNodes);
						}
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////

	void LocateChildLODGroupNodes(Aftk::C_SceneNode* pSceneNode, Btl::Vector<Aftk::C_SceneLodGroupNode*> & LODGroupNodes)
	{
		if (!pSceneNode)
			return;

		if (!pSceneNode->GetNumChildren())
		{
			if (pSceneNode->HasType(Aftk::E_ObjType_SceneLodGroup))
			{
				LODGroupNodes.PushBack((Aftk::C_SceneLodGroupNode*)pSceneNode);
			}
		}
		else
		{
			for (S32 i = 0, count = pSceneNode->GetNumChildren(); i < count; ++i)
			{
				Aftk::C_SceneNode* pChildNode = pSceneNode->GetChildPtr(i);

				if (!pChildNode)
					continue;

				if (pChildNode->HasType(Aftk::E_ObjType_SceneLodGroup))
				{
					LODGroupNodes.PushBack((Aftk::C_SceneLodGroupNode*)pChildNode);
				}
				else
				{
					for (S32 j = 0, childCount = pChildNode->GetNumChildren(); j < childCount; ++j)
					{
						LocateChildLODGroupNodes(pChildNode->GetChildPtr(j), LODGroupNodes);
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdSkinnedSceneDescription::C_CrowdSkinnedSceneDescription()
	{
		m_Random.Seed(0);
	}

	C_CrowdSkinnedSceneDescription::~C_CrowdSkinnedSceneDescription()
	{
	}

	void C_CrowdSkinnedSceneDescription::Build(Aftk::C_SceneNode* pSceneNode, const U32 seed)
	{
		m_Random.Seed(seed);

		char seedString[9];
		LOGDEBUG( Tools::g_log, "[Crowd] Seed: 0x%s", itoa(seed, seedString, 16) );

		if (!pSceneNode)
			return;

		LocateSkinnedNodes(m_SkinnedNodes, pSceneNode, false);

		if (m_SkinnedNodes.Size())
		{
			m_SkinnedNodeDataList.Resize(m_SkinnedNodes.Size());

			for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
			{
				LocateChildMeshNodes(m_SkinnedNodes[i], m_SkinnedNodeDataList[i].m_MeshNodes);
			}

			m_SkinnedLODGroupNodeDataList.Resize(m_SkinnedNodes.Size());

			for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
			{
				LocateChildLODGroupNodes(m_SkinnedNodes[i], m_SkinnedLODGroupNodeDataList[i].m_LODGroupNodes);
			}
		}
	}

	void C_CrowdSkinnedSceneDescription::End()
	{
		for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
		{
#ifdef CROWD_DELETE_SKINNED_NODES
			m_SkinnedNodes[i]->GetScenePtr()->DeleteSceneNodeAndChildren(m_SkinnedNodes[i]);
#else
			TagSkinnedNodeAsBaseInstance(m_SkinnedNodes[i]);
#endif

			//[owent] tag nodes so we can identify them later for cropsheet generation
			//AftkUt::C_NodeFlagParser nodeFlags(*m_SkinnedNodes[i]);
			//nodeFlags.SetParameterValue("CrowdCropSheet", true);
		}
	}

	const U32 C_CrowdSkinnedSceneDescription::GetRandomSeed() const
	{
#if (!defined(CROWD_DEBUG_FORCE_RANDOMNESS) && (defined(CROWD_DEBUG_BODY_PARTS) || defined(CROWD_DEBUG_UPPER_CLOTHING_ONLY) || defined(CROWD_DEBUG_LOWER_CLOTHING_ONLY)))
		return 0;
#else
		return m_Random.Rand();
#endif
	}

	const bool C_CrowdSkinnedSceneDescription::HasSkinnedNodes() const
	{
		return (m_SkinnedNodes.Size() > 0);
	}

	const S32 C_CrowdSkinnedSceneDescription::GetSkinnedNodeIndex(const Aftk::C_SceneNode* pSceneNode) const
	{
		if (pSceneNode)
		{
			for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
			{
				if (m_SkinnedNodes[i])
				{
					if (_stricmp(pSceneNode->GetName().c_str(), m_SkinnedNodes[i]->GetName().c_str()) == 0)
						return i;
				}
			}
		}

		return -1;
	}

	Btl::Vector<Aftk::C_SceneMeshNode*>* C_CrowdSkinnedSceneDescription::GetMeshNodeList(const S32 index) const
	{
		if (index > -1 && index < m_SkinnedNodeDataList.Size())
			return &(m_SkinnedNodeDataList[index].m_MeshNodes);
		return NULL;
	}

	Btl::Vector<Aftk::C_SceneLodGroupNode*>* C_CrowdSkinnedSceneDescription::GetLODGroupNodeList(const S32 index) const
	{
		if (index > -1 && index < m_SkinnedLODGroupNodeDataList.Size())
			return &(m_SkinnedLODGroupNodeDataList[index].m_LODGroupNodes);
		return NULL;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomActor(C_CrowdRandomActor& actor) const
	{
#if (defined(CROWD_DEBUG_UPPER_CLOTHING_ONLY) || defined(CROWD_DEBUG_LOWER_CLOTHING_ONLY))
#ifdef CROWD_DEBUG_UPPER_CLOTHING_ONLY
		if (!GenerateRandomUpper(actor))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper." );
#endif
			return false;
		}
#endif
#ifdef CROWD_DEBUG_LOWER_CLOTHING_ONLY
		if (!GenerateRandomLower(actor))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor lower." );
#endif
			return false;
		}
#endif
#else
		if (!GenerateRandomUpper(actor))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper." );
#endif
			return false;
		}

		if (!GenerateRandomLower(actor))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor lower." );
#endif
			return false;
		}

		if (!GenerateRandomHead(actor))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor head." );
#endif
			return false;
		}

		if (!GenerateRandomAccessories(actor))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor accessories." );
#endif
			return false;
		}

		if (!GenerateRandomProp(actor))
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor prop." );
#endif

		return true;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomUpper(C_CrowdRandomActor& actor) const
	{
#ifdef CROWD_DEBUG_UPPER_CLOTHING_ONLY
		if (!GenerateRandomClothing(actor, E_FlagGroup_UpperClothing))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper clothing." );
#endif
			return false;
		}
#else
		if (!GenerateRandomClothing(actor, E_FlagGroup_UpperClothing))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper clothing." );
#endif
			return false;
		}

		if (!GenerateRandomSkin(actor, E_FlagGroup_Skin, E_CrowdSkinType_ShortArms | E_CrowdSkinType_LongArms))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper arms." );
#endif
			return false;
		}

		if (!GenerateRandomSkin(actor, E_FlagGroup_Skin, E_CrowdSkinType_Neck | E_CrowdSkinType_Bust))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper neck|bust." );
#endif
			return false;
		}

		if (!GenerateRandomSkin(actor, E_FlagGroup_Skin, E_CrowdSkinType_Stomach))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper stomach." );
#endif
			return false;
		}
#endif

		return true;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomLower(C_CrowdRandomActor& actor) const
	{
#ifdef CROWD_DEBUG_LOWER_CLOTHING_ONLY
		if (!GenerateRandomClothing(actor, E_FlagGroup_LowerClothing))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor lower clothing." );
#endif
			return false;
		}
#else
		if (!GenerateRandomClothing(actor, E_FlagGroup_LowerClothing))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor lower clothing." );
#endif
			return false;
		}

		if (!GenerateRandomSkin(actor, E_FlagGroup_Skin, E_CrowdSkinType_Legs))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor lower legs." );
#endif
			return false;
		}

		if (!GenerateRandomClothing(actor, E_FlagGroup_Shoes))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor upper." );
#endif
			return false;
		}
#endif

		return true;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomHead(C_CrowdRandomActor& actor) const
	{
		if (!GenerateRandomSkin(actor, E_FlagGroup_Head))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor head ...head." );
#endif
			return false;
		}

		if (!GenerateRandomSkin(actor, E_FlagGroup_Hair))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor head hair." );
#endif
			return false;
		}

		if (!GenerateRandomClothing(actor, E_FlagGroup_Hat))
		{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
			LOGDEBUG( Tools::g_log, "[Crowd] Failed to generate actor head hat." );
#endif
			return false;
		}

		return true;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomAccessories(C_CrowdRandomActor& actor) const
	{
		if (actor.GetActorDesc().Appearance() == E_CrowdClothingAppearance_Generic)
		{
			for (S32 i = 0, count = static_cast<S32>(E_CrowdAccessoryType_Count); i < count; ++i)
			{
				if (!GenerateRandomClothing(actor, E_FlagGroup_Accessory, static_cast<E_CrowdAccessoryType>(i)))
					return false;
			}
		}
		
		return true;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomProp(C_CrowdRandomActor& actor) const
	{
		if (actor.GetActorDesc().Prop())
			return true; // Already got one

		if (actor.GetActorDesc().PropType() == E_CrowdPropType_None)
			return true;

/*#ifndef CROWD_DEBUG_FORCE_PROPS
		if (actor.GetActorDesc().PropOptional())
		{
			if (actor.GetActorDesc().GetRandom().FRand() > 0.5f)
				return true;
		}
#endif*/

		Btl::Vector<S32> itemIndexesLeft;
		Btl::Vector<S32> itemIndexesRight;

		for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
		{
			Aftk::C_SceneNode* pSceneNode = m_SkinnedNodes[i];

			if (!pSceneNode)
				continue;

			bool status = false;

			AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &status);

			if (!status)
				continue;

			if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "Crowd") == 0)
			{
				E_CrowdFlagGroup nodeFlagGroup = GetFlagGroup(nodeFlags.GetFlagGroupName());

				if (nodeFlagGroup == E_FlagGroup_Prop)
				{
					std::string genderString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigGender, genderString))
						continue;
					
					if (_stricmp(genderString.c_str(), GetCrowdGenderName(actor.GetActorDesc().Female() ? E_CrowdGender_Female : E_CrowdGender_Male)) != 0
						&& _stricmp(genderString.c_str(), GetCrowdGenderName(E_CrowdGender_Unisex)) != 0)
						continue;

					std::string propTypeString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigPropType, propTypeString))
						continue;

					E_CrowdPropType type = GetCrowdPropType(propTypeString);

					if (type != actor.GetActorDesc().PropType())
						continue;

					std::string handPropFlagsString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigHandPropFlags, handPropFlagsString))
						continue;

					E_CrowdHandPropFlags handFlags = GetCrowdHandPropFlags(handPropFlagsString);

					if (handFlags != E_CrowdHandPropFlag_None)
					{
						switch (actor.GetActorDesc().PropHandFlags())
						{
						case E_CrowdHandPropFlag_Left:
							{
								if (handFlags == E_CrowdHandPropFlag_Right)
									continue;
							}
							break;
						case E_CrowdHandPropFlag_Right:
							{
								if (handFlags == E_CrowdHandPropFlag_Left)
									continue;
							}
							break;
						case E_CrowdHandPropFlag_Both:
						case E_CrowdHandPropFlag_None:
						default:
							break;
						}
					}

					if (handFlags == E_CrowdHandPropFlag_Right)
						itemIndexesRight.PushBack(i);
					else
						itemIndexesLeft.PushBack(i);
				}
			}
		}

		if (!itemIndexesLeft.Size() && !itemIndexesRight.Size())
		{
			/*if (actor.GetActorDesc().PropOptional())
				return true;*/

			LOGDEBUG( Tools::g_log, "[Crowd] Unable to find any skinned 'Prop' nodes" );
			return true;
		}


		// left/default first
		if (itemIndexesLeft.Size())
		{
			U32 randomIndex = actor.GetActorDesc().GetRandom().Rand(itemIndexesLeft.Size());

			assert(static_cast<S32>(randomIndex) < itemIndexesLeft.Size());
			assert(itemIndexesLeft[randomIndex] < m_SkinnedNodes.Size());

			if (!actor.AddActorPart(m_SkinnedNodes[itemIndexesLeft[randomIndex]]))
				return false;
		}

		// then right
		if (itemIndexesRight.Size())
		{
			U32 randomIndex = actor.GetActorDesc().GetRandom().Rand(itemIndexesRight.Size());

			assert(static_cast<S32>(randomIndex) < itemIndexesRight.Size());
			assert(itemIndexesRight[randomIndex] < m_SkinnedNodes.Size());

			if (!actor.AddActorPart(m_SkinnedNodes[itemIndexesRight[randomIndex]]))
				return false;
		}

		return true;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomClothing(C_CrowdRandomActor& actor,
		const E_CrowdFlagGroup flagGroup, const E_CrowdAccessoryType accessoryType) const
	{
		if (!ClothingGroup(flagGroup))
			return false;

		if (flagGroup == E_FlagGroup_UpperClothing)
		{
			if (actor.GetActorDesc().UpperClothing())
				return true;
		}

		if (flagGroup == E_FlagGroup_LowerClothing)
		{
			if (actor.GetActorDesc().LowerClothing())
				return true;
		}

		if (flagGroup == E_FlagGroup_Hat)
		{
			if (actor.GetActorDesc().Hooded())
				return true;
			if (actor.GetActorDesc().Hat())
				return true;
			if (!actor.GetActorDesc().RequiresHat())
				return true;
		}

		if (flagGroup == E_FlagGroup_Shoes)
		{
			if (actor.GetActorDesc().Shoes())
				return true;
		}

		if (flagGroup == E_FlagGroup_Accessory)
		{
			if (actor.GetActorDesc().GetRandom().FRand() > 0.5f)
				return true;

			if (accessoryType == E_CrowdAccessoryType_Glasses)
			{
				if (actor.GetActorDesc().Glasses())
					return true;
			}
			if (accessoryType == E_CrowdAccessoryType_Scarf)
			{
				if (actor.GetActorDesc().Scarf())
					return true;
				if (actor.GetActorDesc().Hooded())
					return true;
			}
			if (accessoryType == E_CrowdAccessoryType_Watch)
			{
				if (actor.GetActorDesc().Watch())
					return true;
				// [rthomas]	I refuse to support a piece of geometry that is never bigger than a single pixel on-screen
				//				and has a 128x64 texture :)
				return true;
			}
			if (accessoryType == E_CrowdAccessoryType_Visor)
			{
				if (actor.GetActorDesc().Visor())
					return true;
				if (actor.GetActorDesc().Hat())
					return true;
			}
		}

		Btl::Vector<S32> itemIndexes;

		for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
		{
			Aftk::C_SceneNode* pSceneNode = m_SkinnedNodes[i];

			if (!pSceneNode)
				continue;

			bool status = false;

			AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &status);

			if (!status)
				continue;

			if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "Crowd") == 0)
			{
				E_CrowdFlagGroup nodeFlagGroup = GetFlagGroup(nodeFlags.GetFlagGroupName());

				if ((flagGroup == E_FlagGroup_UpperClothing && nodeFlagGroup == E_FlagGroup_FullBodyClothing) || (nodeFlagGroup == flagGroup))
				{
					std::string genderString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigGender, genderString))
						continue;
					
					if (_stricmp(genderString.c_str(), GetCrowdGenderName(actor.GetActorDesc().Female() ? E_CrowdGender_Female : E_CrowdGender_Male)) != 0
						&& _stricmp(genderString.c_str(), GetCrowdGenderName(E_CrowdGender_Unisex)) != 0)
						continue;

					std::string styleString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigStyle, styleString))
						continue;

					if (_stricmp(styleString.c_str(), GetCrowdClothingStyleName(actor.GetActorDesc().Style())) != 0
						&& _stricmp(styleString.c_str(), GetCrowdClothingStyleName(E_CrowdClothingStyle_All)) != 0)
						continue;

					std::string conditionsString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigConditions, conditionsString))
						continue;

					if (_stricmp(conditionsString.c_str(), GetCrowdClothingConditionsName(actor.GetActorDesc().Cold() ? E_CrowdClothingConditions_Cold : E_CrowdClothingConditions_Hot)) != 0
						&& _stricmp(conditionsString.c_str(), GetCrowdClothingConditionsName(E_CrowdClothingConditions_All)) != 0)
						continue;

					if (flagGroup != E_FlagGroup_LowerClothing)
					{
						std::string appearanceString;
						if (!nodeFlags.GetParameterValue(kCrowdConfigAppearance, appearanceString) && actor.GetActorDesc().Appearance() != E_CrowdAppearance_Generic)
							continue;

						if (!ValidClothingAppearance(actor.GetActorDesc().Appearance(), GetCrowdClothingAppearance(appearanceString)))
							continue;
					}

					if (flagGroup == E_FlagGroup_Accessory)
					{
						std::string accessoryTypeString;
						if (!nodeFlags.GetParameterValue(kCrowdConfigAccessoryType, accessoryTypeString))
							continue;

						E_CrowdAccessoryType type = GetCrowdAccessoryType(accessoryTypeString);

						if (type != accessoryType)
							continue;
					}

					itemIndexes.PushBack(i);
				}
			}
		}

		if (!itemIndexes.Size())
		{
			if (flagGroup == E_FlagGroup_Accessory)
				return true;

			LOGDEBUG( Tools::g_log, "[Crowd] Unable to find any skinned '%s' nodes", GetFlagGroupName(flagGroup) );
			return false;
		}

#ifdef CROWD_DEBUG_FORCE_LOOK_FOR_LOD_GROUPS_FIRST
		for (S32 i = 0, count = itemIndexes.Size(); i < count; ++i)
		{
			const Aftk::C_SceneNode* pSceneNode = m_SkinnedNodes[itemIndexes[i]];

			if (pSceneNode->GetNumChildren())
			{
				for (U32 iChild = 0, childCount = pSceneNode->GetNumChildren(); iChild < childCount; ++iChild)
				{
					const Aftk::C_SceneNode* pChildNode = pSceneNode->GetChildPtr(iChild);

					if (pChildNode && pChildNode->HasType(Aftk::E_ObjType_SceneLodGroup))
					{
						if (actor.AddActorPart(m_SkinnedNodes[itemIndexes[i]]))
							return true;
					}
				}
			}
		}
#endif

		U32 randomIndex = actor.GetActorDesc().GetRandom().Rand(itemIndexes.Size());

		assert(static_cast<S32>(randomIndex) < itemIndexes.Size());
		assert(itemIndexes[randomIndex] < m_SkinnedNodes.Size());

		if (actor.AddActorPart(m_SkinnedNodes[itemIndexes[randomIndex]]))
			return true;

		return false;
	}

	const bool C_CrowdSkinnedSceneDescription::GenerateRandomSkin(C_CrowdRandomActor& actor,
		const E_CrowdFlagGroup flagGroup, const U32 skinType) const
	{
		if (!SkinGroup(flagGroup))
			return false;

		if (flagGroup == E_FlagGroup_Head)
		{
			if (actor.GetActorDesc().Head())
				return true;
		}

		// Check if optional skin types are required
		if (flagGroup == E_FlagGroup_Skin)
		{
			if (skinType & E_CrowdSkinType_Neck || skinType & E_CrowdSkinType_Bust)
			{
				if (actor.GetActorDesc().Torso())
					return true;
				if (actor.GetActorDesc().NeckType() == E_CrowdNeckType_Closed)
					return true;
			}
			if (skinType & E_CrowdSkinType_Stomach)
			{
				if (actor.GetActorDesc().Stomach())
					return true;
				if (!actor.GetActorDesc().Cropped())
					return true;
			}
			if (skinType & E_CrowdSkinType_Legs)
			{
				if (actor.GetActorDesc().Legs())
					return true;
				if (!actor.GetActorDesc().RequiresLegs())
					return true;
			}
		}

		// Check if optional hair is required
		if (flagGroup == E_FlagGroup_Hair)
		{
			if (actor.GetActorDesc().Hair())
				return true;

			if (actor.GetActorDesc().Hooded())
				return true;

			if (actor.GetActorDesc().IncludeHair() == E_CrowdIncludeHair_No)
				return true;
			if (actor.GetActorDesc().IncludeHair() == E_CrowdIncludeHair_NotFussed)
			{
				if (m_Random.FRand() <= 0.5f)
					return true;
			}
		}

		// Looks like it is required, let's do this!
		Btl::Vector<S32> itemIndexes;

		for (S32 i = 0, count = m_SkinnedNodes.Size(); i < count; ++i)
		{
			Aftk::C_SceneNode* pSceneNode = m_SkinnedNodes[i];

			if (!pSceneNode)
				continue;

			bool status = false;

			AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &status);

			if (!status)
				continue;

			if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "Crowd") == 0)
			{
				E_CrowdFlagGroup nodeFlagGroup = GetFlagGroup(nodeFlags.GetFlagGroupName());

				if (nodeFlagGroup == flagGroup)
				{
					std::string genderString;
					if (!nodeFlags.GetParameterValue(kCrowdConfigGender, genderString))
						continue;
					
					if (_stricmp(genderString.c_str(), GetCrowdGenderName(actor.GetActorDesc().Female() ? E_CrowdGender_Female : E_CrowdGender_Male)) != 0
						&& _stricmp(genderString.c_str(), GetCrowdGenderName(E_CrowdGender_Unisex)) != 0)
						continue;

					if (flagGroup == E_FlagGroup_Skin)
					{
						std::string skinTypeString;
						if (!nodeFlags.GetParameterValue(kCrowdConfigSkinType, skinTypeString))
							continue;

						E_CrowdSkinType type = GetCrowdSkinType(skinTypeString);

						bool success = false;

						if (skinType & type)
						{
							if (type == E_CrowdSkinType_ShortArms)
							{
								if (actor.GetActorDesc().SleeveType() == E_CrowdSleeveType_Long)
									success = true;
							}
							if (type == E_CrowdSkinType_LongArms)
							{
								if (actor.GetActorDesc().SleeveType() == E_CrowdSleeveType_Short)
									success = true;
							}
							if (type == E_CrowdSkinType_Neck)
							{
								if (actor.GetActorDesc().NeckType() == E_CrowdNeckType_Open)
									success = true;
							}
							if (type == E_CrowdSkinType_Bust)
							{
								if (actor.GetActorDesc().NeckType() == E_CrowdNeckType_Strap)
									success = true;
							}
							if (type == E_CrowdSkinType_Stomach)
							{
								if (actor.GetActorDesc().Cropped())
									success = true;
							}
							if (type == E_CrowdSkinType_Legs)
							{
								success = true;
							}
						}

						if (!success)
							continue;
					}

					if (flagGroup != E_FlagGroup_Hair)
					{
						std::string skinColourString;
						if (!nodeFlags.GetParameterValue(kCrowdConfigSkinColour, skinColourString))
							continue;

						if (actor.GetActorDesc().ColourDesc().Length() > 0)
						{
							if (_stricmp(actor.GetActorDesc().ColourDesc().Chars(), skinColourString.c_str()) != 0)
								continue;
						}
					}

					itemIndexes.PushBack(i);
				}
			}
		}

		if (!itemIndexes.Size())
		{
			LOGDEBUG( Tools::g_log, "[Crowd] Unable to find any skinned 'Skin' nodes" );
			return false;
		}

		U32 randomIndex = actor.GetActorDesc().GetRandom().Rand(itemIndexes.Size());

		assert(static_cast<S32>(randomIndex) < itemIndexes.Size());
		assert(itemIndexes[randomIndex] < m_SkinnedNodes.Size());

		if (actor.AddActorPart(m_SkinnedNodes[itemIndexes[randomIndex]]))
			return true;

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	void SetLightingFlags(Aftk::C_SceneNode* pNode, bool useVertexLighting)
	{
		if (!pNode)
			return;

		AftkUt::C_NodeFlagParser nodeFlags(*pNode);
		if (useVertexLighting)
		{
			//nodeFlags.SetParameterValue("UseVertexLighting", (bool)true);
			nodeFlags.DeleteParameter("UseVertexLighting");
			nodeFlags.DeleteParameter("UseLightmapLighting");
			nodeFlags.DeleteParameter("UseSHLighting");
		}
		else
		{
			nodeFlags.SetParameterValue("UseSHLighting", (bool)true);
			nodeFlags.DeleteParameter("UseLightmapLighting");
			nodeFlags.DeleteParameter("UseVertexLighting");
		}
	}

	void SetSkinInstanceNodeFlags(Aftk::C_SceneInstanceNode* pNode, const Maths::HVec3& translate, const S32 uniqueId,
		const Aftk::C_SceneNode* pPlaceholderNode)
	{
		AftkUt::C_NodeFlagParser instNodeFlags(*pNode);
		//instNodeFlags.SetParameterValue("UseSHLighting", (bool)true);
		instNodeFlags.DeleteParameter("UseSHLighting");
		instNodeFlags.DeleteParameter("UseLightmapLighting");
		instNodeFlags.DeleteParameter("UseVertexLighting");
		instNodeFlags.SetParameterValue("PaletteIndex", uniqueId);
		instNodeFlags.SetParameterValue("PaletteSortId", -1);

		// TODO: [rthomas]	Use extents from behaviour somehow :|
		
		Maths::HVec4 extents(-2.f, 2.f, -2.f, 2.f); // default
		if (pPlaceholderNode)
		{
			Aftk::C_Bounds bounds = pPlaceholderNode->GetEvaluatedLocalBounds();

			/*extents.X(Maths::Min<F32>(bounds.GetMin().X(), bounds.GetMin().Z()));
			extents.Y(Maths::Max<F32>(bounds.GetMax().X(), bounds.GetMax().Z()));
			extents.Z(bounds.GetMin().Y());
			extents.W(bounds.GetMax().Y());*/
		}

		instNodeFlags.SetParameterValue("BoundsCentre", translate.X(), translate.Y(), translate.Z(), 0.f);
		instNodeFlags.SetParameterValue("BoundsExtents", extents.X(), extents.Y(), extents.Z(), extents.W());
	}

	void UseSecondaryBehaviour(Aftk::C_SceneNode* pNode, const bool useSecondaryBehaviour)
	{
		if (!pNode)
			return;

		AftkUt::C_NodeFlagParser nodeFlags(*pNode);
		nodeFlags.SetParameterValue("UseSecondaryBehaviour", useSecondaryBehaviour);
	}

	bool UseSecondaryBehaviour(const Aftk::C_SceneNode* pNode)
	{
		if (!pNode)
			return false;

		AftkUt::C_NodeFlagParser nodeFlags(*pNode);

		bool useSecondaryBehaviour = false;
		nodeFlags.GetParameterValue("UseSecondaryBehaviour", useSecondaryBehaviour);

		return useSecondaryBehaviour;
	}

	void DumpRandomActor(const C_CrowdRandomActor& actor, bool failed)
	{
#ifdef CROWD_DEBUG_SPEW_ACTOR_PART_NAMES
		if (failed)
		{
			LOGDEBUG( Tools::g_log, "[Crowd] Failed crowd actor had '%d' parts", actor.GetNumActorParts() );
		}
		else
		{
			if (!g_CrowdDebugSpewValidRandomActors)
				return;

			LOGDEBUG( Tools::g_log, "[Crowd] Crowd actor has '%d' parts", actor.GetNumActorParts() );
		}

		for (S32 i = 0, count = actor.GetNumActorParts(); i < count; ++i)
		{
			Aftk::C_SceneNode* pSceneNode = actor.GetActorPart(i);
			if (pSceneNode)
			{
				LOGDEBUG( Tools::g_log, "[Crowd] - Actor part '%d' - '%s'", i, pSceneNode->GetName().c_str() );
			}
		}
#endif
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdSpectatorData::C_CrowdSpectatorData()
		: m_LastRandomIndex(-1)
	{}

	C_CrowdSpectatorData::~C_CrowdSpectatorData()
	{}

	U32 C_CrowdSpectatorData::Build(Aftk::C_Scene* pAftkScene, const C_CrowdPlaceholderData& placeholderData, const U32 numNodes,
		const C_CrowdSkinnedSceneDescription& desc, const U32 seed, const bool rotateY180)
	{
		if (!pAftkScene || !numNodes)
			return 0;

		Util::C_Random random;
		random.Seed(seed);

		if (!placeholderData.BehaviourHash)
			return 0;

		U32 numMeshes = 0;

		for (U32 i = 0, count = numNodes; i < count; ++i)
		{
			C_CrowdRandomActor randomActor(random.Rand(), placeholderData.Appearance);

			randomActor.GetActorDesc().PropType(E_CrowdPropType_None);
			randomActor.GetActorDesc().PropHandFlags(E_CrowdHandPropFlag_None);

			// defaults
			bool useSecondaryBehaviour = false;
			E_CrowdPropType propType = E_CrowdPropType_None;
			E_CrowdHandPropFlags propHandFlags = E_CrowdHandPropFlag_None;

			if (placeholderData.SecondaryBehaviourHash /*&& (placeholderData.SecondaryBehaviourHash != placeholderData.BehaviourHash)*/)
			{
				// randomly pick either primary or secondary
				if (random.FRand() < 0.7f)
				{
					// Primary
					useSecondaryBehaviour = false;
					if (placeholderData.PropOptional)
					{
						if (random.FRand() >= 0.5f)
						{
							propType = placeholderData.PropType;
							propHandFlags = placeholderData.PropHandFlags;
						}
					}
					else
					{
						propType = placeholderData.PropType;
						propHandFlags = placeholderData.PropHandFlags;
					}
				}
				else
				{
					// Secondary
					useSecondaryBehaviour = true;
					if (placeholderData.SecondaryPropOptional)
					{
						if (random.FRand() >= 0.5f)
						{
							propType = placeholderData.SecondaryPropType;
							propHandFlags = placeholderData.SecondaryPropHandFlags;
						}
					}
					else
					{
						propType = placeholderData.SecondaryPropType;
						propHandFlags = placeholderData.SecondaryPropHandFlags;
					}
				}
			}
			else
			{
				// Use primary
				useSecondaryBehaviour = false;
				if (placeholderData.PropOptional)
				{
					if (random.FRand() >= 0.5f)
					{
						propType = placeholderData.PropType;
						propHandFlags = placeholderData.PropHandFlags;
					}
				}
				else
				{
					propType = placeholderData.PropType;
					propHandFlags = placeholderData.PropHandFlags;
				}
			}

			randomActor.GetActorDesc().UseSecondaryBehaviour(useSecondaryBehaviour);
			randomActor.GetActorDesc().PropType(propType);
			randomActor.GetActorDesc().PropHandFlags(propHandFlags);

			std::stringstream nameStream;
			nameStream << placeholderData.Name.c_str() << "_" << i;

			std::string nodeName;
			nameStream >> nodeName;

			randomActor.SetName(nodeName.c_str());

			if (desc.GenerateRandomActor(randomActor))
			{
				DumpRandomActor(randomActor, false);
				numMeshes += AddMesh(randomActor, desc, pAftkScene, placeholderData, rotateY180);
			}
			else
			{
				DumpRandomActor(randomActor, true);
			}
		}

		return numMeshes;
	}

	const U32 C_CrowdSpectatorData::GetNumGroupNodes() const
	{
		return static_cast<U32>(m_LodGroupNodes.Size());
	}

	const Aftk::C_SceneLodGroupNode* C_CrowdSpectatorData::GetRandomGroupNode(Util::C_Random& random)
	{
		if (GetNumGroupNodes())
		{
			U32 randomIndex = random.Rand(GetNumGroupNodes());

			if (m_LastRandomIndex >= 0 && randomIndex == static_cast<U32>(m_LastRandomIndex))
				randomIndex = ((randomIndex + 1) % GetNumGroupNodes());

			m_LastRandomIndex = static_cast<S32>(randomIndex);
			
			return m_LodGroupNodes[randomIndex];
		}

		return NULL;
	}

	U32 C_CrowdSpectatorData::AddMesh(const C_CrowdRandomActor& actor, const C_CrowdSkinnedSceneDescription& desc,
		Aftk::C_Scene* pScene, const C_CrowdPlaceholderData& placeholderData, const bool rotateY180)
	{
		assert(pScene && placeholderData.ParentPtr);

		Aftk::C_SceneLodGroupNode* pGroupNode = new Aftk::C_SceneLodGroupNode(pScene);
		pGroupNode->SetName(actor.GetName().Chars());
		placeholderData.ParentPtr->AddChildPtr(pGroupNode);

		/*for (S32 i = 0, count = kCrowdActorMeshLodLevelCount; i < count; ++i)
		{
			GenerateLodLevelMesh(*pGroupNode, kCrowdActorLodLevelThresholds[i],
				kCrowdActorLodLevelNames[i], *pScene, placeholderData, rotateY180);
		}*/

		for (S32 i = 0, count = kCrowdActorTotalLodLevelCount; i < count; ++i)
		{
			GenerateLodLevelMesh(*pGroupNode, kCrowdActorLodLevelThresholds[i],
				kCrowdActorLodLevelNames[i], *pScene, placeholderData, rotateY180, (i == kCrowdActorImposterLodLevelIndex));
		}

		for (S32 i = 0, count = actor.GetNumActorParts(); i < count; ++i)
		{
			Aftk::C_SceneNode* pSceneNode = actor.GetActorPart(i);

			bool bStatus = false;
			AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &bStatus);
			if (!bStatus)
				continue;

			E_CrowdFlagGroup flagGroup = GetFlagGroup(nodeFlags.GetFlagGroupName());
			if (!HasValidCrowdFlagGroupName(flagGroup))
				continue;

			U32 lodLevels = Maths::Min<U32>(kCrowdActorMeshLodLevelCount, GetLODLevelsByCrowdFlagGroup(flagGroup));

			if (pSceneNode)
			{
				Btl::Vector<Aftk::C_SceneLodGroupNode*>* pLODGroupNodes = desc.GetLODGroupNodeList(desc.GetSkinnedNodeIndex(pSceneNode));

				for (S32 j = 0, LODGroupCount = pLODGroupNodes->Size(); j < LODGroupCount; ++j)
				{
					Aftk::C_SceneLodGroupNode* pLODGroupNode = (*pLODGroupNodes)[j];

					if (pLODGroupNode)
						AddLodGroupToLodGroupNode(pGroupNode, pLODGroupNode, lodLevels);
				}
			}
		}

		UseSecondaryBehaviour(pGroupNode, actor.GetActorDesc().UseSecondaryBehaviour());

		m_LodGroupNodes.PushBack(pGroupNode);

		return pGroupNode->GetNumLodLevels();
	}

	void C_CrowdSpectatorData::GenerateLodLevelMesh(Aftk::C_SceneLodGroupNode& groupNode, const F32 fThreshold,
		const char* pName, Aftk::C_Scene& aftkScene, const C_CrowdPlaceholderData& placeholderData, const bool rotateY180, bool bImposterMesh)
	{
		Aftk::C_SceneMeshNode* pMeshNode = new Aftk::C_SceneMeshNode(&aftkScene);
		pMeshNode->SetName(groupNode.GetName() + pName);

		if (bImposterMesh)
		{
			if (placeholderData.ImposterInfo.IsValid())
			{
				Aftk::C_SceneMeshNode* pNode = (Aftk::C_SceneMeshNode*)groupNode.GetLodLevelInfo(0).GetSceneNodePtr();
				GenerateImposterLodMesh(*pMeshNode, aftkScene, placeholderData, rotateY180);
			}
			else
			{
				aftkScene.DeleteSceneNodeAndChildren(pMeshNode);
				return;
			}
		}

		groupNode.AddChildPtr(pMeshNode);

		SetLightingFlags(pMeshNode, bImposterMesh);

		Aftk::C_GroupLodLevelInfo lodLevelInfo;
		lodLevelInfo.SetSceneNodePtr(pMeshNode);
		lodLevelInfo.SetThreshold(fThreshold);

		groupNode.AddLodLevelInfo(lodLevelInfo);
	}

	void C_CrowdSpectatorData::GenerateImposterLodMesh(Aftk::C_SceneMeshNode& meshNode, Aftk::C_Scene& aftkScene,
		const C_CrowdPlaceholderData& placeholderData, const bool rotateY180)
	{
		// TODO: [rthomas]	Use bounding box of relevant placeholder node to work out size of billboard
		const Aftk::C_BlindDataResource* pPolyFlagsBlindData = 0;
		{
			Aftk::C_BlindDataNode blindData;

			Aftk::C_GenericAttribute attr;

			attr.SetName("polySet_doNotCastShadows");
			blindData.AddAttribute(attr);
			attr.Clear();

			attr.SetName("polySet_doNotLight");
			blindData.AddAttribute(attr);
			attr.Clear();

			attr.SetName("polySet_excludeFromCubemap");
			blindData.AddAttribute(attr);
			attr.Clear();

			attr.SetName("polySet_excludeFromPrepass");
			blindData.AddAttribute(attr);
			attr.Clear();

			pPolyFlagsBlindData = aftkScene.AddBlindDataNode(blindData);
		}

		Aftk::C_Bounds bounds = placeholderData.Bounds;

		F32 min_XY_X = bounds.GetMin().X();
		F32 max_XY_X = bounds.GetMax().X();

		F32 min_XY_Y = bounds.GetMin().Y();
		F32 max_XY_Y = bounds.GetMax().Y();

		F32 min_ZY_Z = bounds.GetMin().Z();
		F32 max_ZY_Z = bounds.GetMax().Z();

		F32 min_ZY_Y = bounds.GetMin().Y();
		F32 max_ZY_Y = bounds.GetMax().Y();

		bool bScaleBoundsToMaxDimension = true;

		if (bScaleBoundsToMaxDimension)
		{
			Maths::HVec3 dimensions = Maths::HVec3(bounds.GetMax().X(), bounds.GetMax().Y(), bounds.GetMax().Z())
				- Maths::HVec3(bounds.GetMin().X(), bounds.GetMin().Y(), bounds.GetMin().Z());

			if (dimensions.X() > dimensions.Y())
			{
				min_XY_Y = bounds.GetMin().Y() + (dimensions.Y() * 0.5f) - (dimensions.X() * 0.5f);
				max_XY_Y = bounds.GetMax().Y() - (dimensions.Y() * 0.5f) + (dimensions.X() * 0.5f);
			}
			else
			{
				min_XY_X = bounds.GetMin().X() + (dimensions.X() * 0.5f) - (dimensions.Y() * 0.5f);
				max_XY_X = bounds.GetMax().X() - (dimensions.X() * 0.5f) + (dimensions.Y() * 0.5f);
			}

			if (dimensions.Z() > dimensions.Y())
			{
				min_ZY_Y = bounds.GetMin().Y() + (dimensions.Y() * 0.5f) - (dimensions.Z() * 0.5f);
				max_ZY_Y = bounds.GetMax().Y() - (dimensions.Y() * 0.5f) + (dimensions.Z() * 0.5f);
			}
			else
			{
				min_ZY_Z = bounds.GetMin().Z() + (dimensions.Z() * 0.5f) - (dimensions.Y() * 0.5f);
				max_ZY_Z = bounds.GetMax().Z() - (dimensions.Z() * 0.5f) + (dimensions.Y() * 0.5f);
			}
		}

		{
			bool bAntiClockwise = true;
			bool bCentreOffset = true;
			bool bZYPlane = true;
			bool bMakeDoubleSided = true;

			S16 blindDataMapping = meshNode.AddPolygonBlindDataMapping(pPolyFlagsBlindData);

			S32 posIdx[8]	= {-1, -1, -1, -1, -1, -1, -1, -1}; // position indexes
			S32 uvIdx[4]	= {-1, -1, -1, -1};					// uv indexes
			S32 pvIdx[8]	= {-1, -1, -1, -1, -1, -1, -1 ,-1};	// polygon vertex indexes
			S32 nIdx[2]		= {-1, -1};							// normal indexes

			// vertex positions
			{
				Aftk::Position3 p[8];
				{
					S32 c = 0;

					// XY Plane
					p[c].x = min_XY_X;
					p[c].y = max_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					p[c].x = min_XY_X;
					p[c].y = min_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					p[c].x = max_XY_X;
					p[c].y = min_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					p[c].x = max_XY_X;
					p[c].y = max_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					// ZY Plane
					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = max_ZY_Y;
					p[c].z = min_ZY_Z;
					c++;

					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = min_ZY_Y;
					p[c].z = min_ZY_Z;
					c++;

					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = min_ZY_Y;
					p[c].z = max_ZY_Z;
					c++;

					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = max_ZY_Y;
					p[c].z = max_ZY_Z;
					c++;
				}

				Aftk::Matrix44 rotation;
				UtMaths::SetIdentity(&rotation);
				rotation[0][0] = -1.f;
				rotation[2][2] = -1.f;

				for (U32 i = 0; i < static_cast<U32>(bZYPlane ? 8 : 4); ++i)
				{
					if (!rotateY180)
						p[i] = Aftk::ApplyMatrix(rotation, p[i]);

					p[i] = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverse(), p[i]);
					posIdx[i] = meshNode.AddVertexPosition(p[i]);
				}
			}

			// vertex normals
			{
				{
					Aftk::Vector3F32 n(0.f, 0.f, -1.f);
					if (!rotateY180)
						n *= -1.f;
					if (bAntiClockwise)
						n *= -1.f;
					n = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverseTranspose(), n);
					UtMaths::SafeNormalise(&n);
					nIdx[0] = meshNode.AddVertexNormal((const Aftk::Normal3&)n);
				}
				if (bZYPlane)
				{
					Aftk::Vector3F32 n(1.f, 0.f, 0.f);
					if (!rotateY180)
						n *= -1.f;
					if (bAntiClockwise)
						n *= -1.f;
					n = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverseTranspose(), n);
					UtMaths::SafeNormalise(&n);
					nIdx[1] = meshNode.AddVertexNormal((const Aftk::Normal3&)n);
				}
			}

			// texxture coordinates
			{
				if (bAntiClockwise)
				{
					uvIdx[0] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 0.f, 0.f, 0.f));
					uvIdx[1] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 1.f, 0.f, 0.f));
					uvIdx[2] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 1.f, 0.f, 0.f));
					uvIdx[3] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 0.f, 0.f, 0.f));
				}
				else
				{
					uvIdx[0] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 0.f, 0.f, 0.f));
					uvIdx[1] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 1.f, 0.f, 0.f));
					uvIdx[2] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 1.f, 0.f, 0.f));
					uvIdx[3] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 0.f, 0.f, 0.f));
				}
			}

			// polygon vertices
			{
				for (S32 i = 0; i < (bZYPlane ? 8 : 4); ++i)
				{
					Aftk::C_PolygonVertex pv;
					pv.SetPositionIndex(posIdx[i]);
					pv.SetNormalIndex(nIdx[(S32)::floor(i / 4.f)]);
					if (i < 4)
						pv.SetTextureCoordinateIndex(0, uvIdx[i]);
					else
						pv.SetTextureCoordinateIndex(0, uvIdx[3 - (i % 4)]);

					pvIdx[i] = meshNode.AddPolygonVertex(pv);
				}
			}

			// material
			S16 materialMapping[3] = {-1, -1, -1};

			for (U32 i = 0; i < 3; ++i)
			{
				Aftk::C_MaterialNode material;
				material.SetOrigin(Aftk::E_MaterialOrigin_Maya);

				// add shader node
				Aftk::C_ShaderNode* pLodShader = new Aftk::C_ShaderNode;
				material.AddShaderNodePtr(pLodShader);
				pLodShader->SetName("LodShader");

				{
					Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
					material.AddPlugPtr(pNewPlug);
					pNewPlug->SetName("mayaSurfaceShader");
					pNewPlug->SetSourceConnectionPtr(pLodShader);
				}

				std::string colourPlugName = "color";

				// set billboard fx
				{
					// emulate maya hlsl shaders
					material.SetOrigin(Aftk::E_MaterialOrigin_Maya_Hlsl_2008);
					Aftk::C_DynamicPlug* pFXPlug = new Aftk::C_DynamicPlug();
					pLodShader->AddPlugPtr(pFXPlug);
					pFXPlug->SetValue(std::string("Default.fx"));
					pFXPlug->SetName("shader");

					colourPlugName = "Colour";
				}

				// add shader image node
				{
					Aftk::C_ShaderImageNode* pImageNode = new Aftk::C_ShaderImageNode;
					material.AddShaderNodePtr(pImageNode);
					pImageNode->SetName("file1");

					std::string imagePath = placeholderData.ImposterInfo.ImagePath[i];
					pImageNode->GetFilePlug().SetValue(imagePath);

					pImageNode->GetImageTypePlug().SetValue("");

					Aftk::C_DynamicPlug* pOutColourPlug = new Aftk::C_DynamicPlug();
					pImageNode->AddPlugPtr(pOutColourPlug);
					pOutColourPlug->SetName("outColour");

					Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
					pLodShader->AddPlugPtr(pPlug);
					pPlug->SetName(colourPlugName);
					pPlug->SetSourceConnectionPtr(pOutColourPlug);
				}

				const Aftk::C_MaterialResource *pMaterialResource = aftkScene.AddMaterialNode(material);
				materialMapping[i] = meshNode.AddPolygonMaterialMapping(pMaterialResource);
			}

			// now the actual polygons
			// XZ Plane
			{
				S32 polyIdx = meshNode.AddPolygon();
				Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

				if (bAntiClockwise)
				{
					polygon.AddPolygonVertexIndex(pvIdx[0]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
				}
				else
				{
					polygon.AddPolygonVertexIndex(pvIdx[3]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
				}
				polygon.SetMaterialMappingIndex(materialMapping[0]);
				polygon.SetBlindDataMappingIndex(blindDataMapping);
			}
			{
				S32 polyIdx = meshNode.AddPolygon();
				Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

				if (bAntiClockwise)
				{
					polygon.AddPolygonVertexIndex(pvIdx[0]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
					polygon.AddPolygonVertexIndex(pvIdx[3]);
				}
				else
				{
					polygon.AddPolygonVertexIndex(pvIdx[3]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
					polygon.AddPolygonVertexIndex(pvIdx[0]);
				}
				polygon.SetMaterialMappingIndex(materialMapping[0]);
				polygon.SetBlindDataMappingIndex(blindDataMapping);
			}
			// ZY Plane
			if (bZYPlane)
			{
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdx[4]);
						polygon.AddPolygonVertexIndex(pvIdx[5]);
						polygon.AddPolygonVertexIndex(pvIdx[6]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdx[7]);
						polygon.AddPolygonVertexIndex(pvIdx[6]);
						polygon.AddPolygonVertexIndex(pvIdx[5]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[2]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdx[4]);
						polygon.AddPolygonVertexIndex(pvIdx[6]);
						polygon.AddPolygonVertexIndex(pvIdx[7]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdx[7]);
						polygon.AddPolygonVertexIndex(pvIdx[5]);
						polygon.AddPolygonVertexIndex(pvIdx[4]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[2]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
			}

			if (bMakeDoubleSided)
			{
				S32 nIdxBack[2] = {-1, -1};
				{
					Aftk::Normal3 n = meshNode.GetVertexNormal(nIdx[0]);
					n *= -1.f;
					nIdxBack[0] = meshNode.AddVertexNormal(n);
				}
				if (bZYPlane)
				{
					Aftk::Normal3 n = meshNode.GetVertexNormal(nIdx[1]);
					n *= -1.f;
					nIdxBack[1] = meshNode.AddVertexNormal(n);
				}

				S32 pvIdxBack[8] = {-1, -1, -1, -1, -1, -1, -1 ,-1};

				for (S32 i = 0; i < (bZYPlane ? 8 : 4); ++i)
				{
					Aftk::C_PolygonVertex pv = meshNode.GetPolygonVertex(pvIdx[i]);
					pv.SetNormalIndex(nIdxBack[(S32)::floor(i / 4.f)]);
					pv.SetTextureCoordinateIndex(0, uvIdx[3 - (i % 4)]);
					pvIdxBack[i] = meshNode.AddPolygonVertex(pv);
				}

				// XY Plane
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[1]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[1]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				// ZY Plane
				if (bZYPlane)
				{
					{
						S32 polyIdx = meshNode.AddPolygon();
						Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

						if (bAntiClockwise)
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[7]);
							polygon.AddPolygonVertexIndex(pvIdxBack[6]);
							polygon.AddPolygonVertexIndex(pvIdxBack[5]);
						}
						else
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[4]);
							polygon.AddPolygonVertexIndex(pvIdxBack[5]);
							polygon.AddPolygonVertexIndex(pvIdxBack[6]);
						}
						polygon.SetMaterialMappingIndex(materialMapping[2]);
						polygon.SetBlindDataMappingIndex(blindDataMapping);
					}
					{
						S32 polyIdx = meshNode.AddPolygon();
						Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

						if (bAntiClockwise)
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[7]);
							polygon.AddPolygonVertexIndex(pvIdxBack[5]);
							polygon.AddPolygonVertexIndex(pvIdxBack[4]);
						}
						else
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[4]);
							polygon.AddPolygonVertexIndex(pvIdxBack[6]);
							polygon.AddPolygonVertexIndex(pvIdxBack[7]);
						}
						polygon.SetMaterialMappingIndex(materialMapping[2]);
						polygon.SetBlindDataMappingIndex(blindDataMapping);
					}
				}
			}
		}
	}

	bool C_CrowdSpectatorData::AddMeshToLodGroupNode(Aftk::C_SceneLodGroupNode* pGroupNode, Aftk::C_SceneMeshNode* pMeshNode, const U32 lodLevel)
	{
		assert(pGroupNode);

		if (!pMeshNode)
			return false;

		if (!((Aftk::C_SceneMeshNode*)pGroupNode->GetLodLevelInfo(lodLevel).GetSceneNodePtr())->AddMesh(pMeshNode) == Aftk::C_Status::E_Status_Success)
			return false;

		return true;
	}

	bool C_CrowdSpectatorData::AddLodGroupToLodGroupNode(Aftk::C_SceneLodGroupNode* pGroupNode, Aftk::C_SceneLodGroupNode* pLodGroupNode, const U32 maxLevelsOfDetail)
	{
		assert(pGroupNode);

		if (!pLodGroupNode)
			return false;

		U32 destNumLodLevels = pGroupNode->GetNumLodLevels();
		U32 srcNumLodLevels = pLodGroupNode->GetNumLodLevels();

		if (!destNumLodLevels || !srcNumLodLevels)
			return true;

		for (U32 i = 0, count = maxLevelsOfDetail; i < count; ++i)
		{
			U32 lodLevelIndex = i;
			if (lodLevelIndex >= srcNumLodLevels)
				lodLevelIndex = srcNumLodLevels - 1;
			
			if (lodLevelIndex < destNumLodLevels)
			{
				if (pLodGroupNode->GetLodLevelInfo(lodLevelIndex).GetSceneNodePtr())
				{
					Aftk::C_SceneMeshNode* pMeshNode = (Aftk::C_SceneMeshNode*)pLodGroupNode->GetLodLevelInfo(lodLevelIndex).GetSceneNodePtr();

					if (!((Aftk::C_SceneMeshNode*)pGroupNode->GetLodLevelInfo(i).GetSceneNodePtr())->AddMesh(pMeshNode) == Aftk::C_Status::E_Status_Success)
						return false;
				}
			}
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	C_CrowdSpectatorDatabase::C_CrowdSpectatorDatabase()
	{}

	C_CrowdSpectatorDatabase::~C_CrowdSpectatorDatabase()
	{}

	U32 C_CrowdSpectatorDatabase::Build(Ut::Vector<Aftk::C_SceneNode*>& placeholderNodes, Ut::Vector<C_CrowdPlaceholderData> & placeholderDataList,
		Btl::Map<U32, U32> & placeholderDataMap, const C_CrowdSkinnedSceneDescription& desc, Aftk::C_Scene* pScene, const U32 seed, const bool rotateY180)
	{
		if (!placeholderNodes.Size() || !placeholderDataList.Size())
			return 0;

		U32 numMeshesCreated = 0;

		m_Random.Seed(seed);

		m_Spectators.Resize(placeholderDataList.Size());
		
		U32 totalInstanceNodes = 0;

		for (U32 i = 0, count = static_cast<U32>(placeholderNodes.Size()); i < count; ++i)
		{
			if (placeholderNodes[i])
			{
				totalInstanceNodes += placeholderNodes[i]->GetNumInstancesOfThisNode();

				Aftk::C_SceneNode* pPlaceholder = placeholderNodes[i];
				
				Btl::Map<U32, U32>::ConstIterator dataMapIter = placeholderDataMap.Find(i);

				U32 dataIndex = 0;

				if (dataMapIter != placeholderDataMap.End())
					dataIndex = (*dataMapIter).GetValue();
				else
					dataIndex = 0;

				placeholderDataList[dataIndex].NumInstances += placeholderNodes[i]->GetNumInstancesOfThisNode();
			}
		}

		for (U32 i = 0, count = static_cast<U32>(placeholderDataList.Size()); i < count; ++i)
		{
			U32 numMeshes = (U32)::ceil(kCrowdBaseMeshDatabaseSize * (placeholderDataList[i].NumInstances / (F32)totalInstanceNodes));

			if (numMeshes > placeholderDataList[i].NumInstances)
				numMeshes = placeholderDataList[i].NumInstances;
				
			numMeshesCreated += m_Spectators[i].Build(pScene, placeholderDataList[i],
				numMeshes, desc, m_Random.Rand(), rotateY180);
		}

		return numMeshesCreated;
	}

	void C_CrowdSpectatorDatabase::End()
	{
	}

	C_CrowdSpectatorData* C_CrowdSpectatorDatabase::GetSpectator(const U32 index)
	{
		if (index < static_cast<U32>(m_Spectators.Size()))
			return &m_Spectators[index];
		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	Btl::Map<std::string, S32> g_NodeNameMap;
	Btl::Vector<S32> g_NodeNameCount;

	std::string GetUniqueNodeName(std::string& s)
	{
		std::stringstream nameStream;

		nameStream << s;

		Btl::Map<std::string, S32>::Iterator it = g_NodeNameMap.Find(s);

		if (it != g_NodeNameMap.End())
		{
			S32 index = it->GetValue();

			nameStream << "_" << g_NodeNameCount[index]++;
		}
		else
		{
			S32 index = g_NodeNameCount.Size();
			
			g_NodeNameMap.Insert(s, index);

			g_NodeNameCount.PushBack(0);

			nameStream << "_" << g_NodeNameCount[index]++;
		}

		std::string nodeName;

		nameStream >> nodeName;

		return nodeName;
	}

	static U32 iNumMeshesTaggedForCropSheet = 0;

	void SetCrowdCropSheetFlags(Aftk::C_SceneNode* pSceneNode)
	{
		if (pSceneNode)
		{
			bool bStatus = false;
			bool bAlreadyAdded = false;

			AftkUt::C_NodeFlagParser nodeFlags(*pSceneNode, &bStatus);
			if (bStatus)
			{
				bool bCropSheet = false;
				if (nodeFlags.GetParameterValue("CrowdCropSheet", bCropSheet))
				{
					if (bCropSheet)
						bAlreadyAdded = true;
				}
			}
			
			nodeFlags.SetParameterValue("CrowdCropSheet", true);

			if (!bAlreadyAdded)
				iNumMeshesTaggedForCropSheet++;
		}
	}

	/*Aftk::C_Status GenerateImposterLodMesh(Aftk::C_SceneMeshNode& meshNode, Aftk::C_Scene& aftkScene,
		const C_CrowdPlaceholderData& placeholderData, const bool rotateY180)
	{
		// TODO: [rthomas]	Use bounding box of relevant placeholder node to work out size of billboard
		const Aftk::C_BlindDataResource* pPolyFlagsBlindData = 0;
		{
			Aftk::C_BlindDataNode blindData;

			Aftk::C_GenericAttribute attr;

			attr.SetName("polySet_doNotCastShadows");
			blindData.AddAttribute(attr);
			attr.Clear();

			attr.SetName("polySet_doNotLight");
			blindData.AddAttribute(attr);
			attr.Clear();

			attr.SetName("polySet_excludeFromCubemap");
			blindData.AddAttribute(attr);
			attr.Clear();

			attr.SetName("polySet_excludeFromPrepass");
			blindData.AddAttribute(attr);
			attr.Clear();

			pPolyFlagsBlindData = aftkScene.AddBlindDataNode(blindData);
		}

		Aftk::C_Bounds bounds = placeholderData.Bounds;

		F32 min_XY_X = bounds.GetMin().X();
		F32 max_XY_X = bounds.GetMax().X();

		F32 min_XY_Y = bounds.GetMin().Y();
		F32 max_XY_Y = bounds.GetMax().Y();

		F32 min_ZY_Z = bounds.GetMin().Z();
		F32 max_ZY_Z = bounds.GetMax().Z();

		F32 min_ZY_Y = bounds.GetMin().Y();
		F32 max_ZY_Y = bounds.GetMax().Y();

		bool bScaleBoundsToMaxDimension = true;

		if (bScaleBoundsToMaxDimension)
		{
			Maths::HVec3 dimensions = Maths::HVec3(bounds.GetMax().X(), bounds.GetMax().Y(), bounds.GetMax().Z())
				- Maths::HVec3(bounds.GetMin().X(), bounds.GetMin().Y(), bounds.GetMin().Z());

			if (dimensions.X() > dimensions.Y())
			{
				min_XY_Y = bounds.GetMin().Y() + (dimensions.Y() * 0.5f) - (dimensions.X() * 0.5f);
				max_XY_Y = bounds.GetMax().Y() - (dimensions.Y() * 0.5f) + (dimensions.X() * 0.5f);
			}
			else
			{
				min_XY_X = bounds.GetMin().X() + (dimensions.X() * 0.5f) - (dimensions.Y() * 0.5f);
				max_XY_X = bounds.GetMax().X() - (dimensions.X() * 0.5f) + (dimensions.Y() * 0.5f);
			}

			if (dimensions.Z() > dimensions.Y())
			{
				min_ZY_Y = bounds.GetMin().Y() + (dimensions.Y() * 0.5f) - (dimensions.Z() * 0.5f);
				max_ZY_Y = bounds.GetMax().Y() - (dimensions.Y() * 0.5f) + (dimensions.Z() * 0.5f);
			}
			else
			{
				min_ZY_Z = bounds.GetMin().Z() + (dimensions.Z() * 0.5f) - (dimensions.Y() * 0.5f);
				max_ZY_Z = bounds.GetMax().Z() - (dimensions.Z() * 0.5f) + (dimensions.Y() * 0.5f);
			}
		}

		{
			bool bAntiClockwise = true;
			bool bCentreOffset = true;
			bool bZYPlane = true;
			bool bMakeDoubleSided = true;

			S16 blindDataMapping = meshNode.AddPolygonBlindDataMapping(pPolyFlagsBlindData);

			S32 posIdx[8]	= {-1, -1, -1, -1, -1, -1, -1, -1}; // position indexes
			S32 uvIdx[4]	= {-1, -1, -1, -1};					// uv indexes
			S32 pvIdx[8]	= {-1, -1, -1, -1, -1, -1, -1 ,-1};	// polygon vertex indexes
			S32 nIdx[2]		= {-1, -1};							// normal indexes

			// vertex positions
			{
				Aftk::Position3 p[8];
				{
					S32 c = 0;

					// XY Plane
					p[c].x = min_XY_X;
					p[c].y = max_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					p[c].x = min_XY_X;
					p[c].y = min_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					p[c].x = max_XY_X;
					p[c].y = min_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					p[c].x = max_XY_X;
					p[c].y = max_XY_Y;
					p[c].z = (bCentreOffset ? bounds.GetCentre().Z() : 0.f);
					c++;

					// ZY Plane
					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = max_ZY_Y;
					p[c].z = min_ZY_Z;
					c++;

					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = min_ZY_Y;
					p[c].z = min_ZY_Z;
					c++;

					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = min_ZY_Y;
					p[c].z = max_ZY_Z;
					c++;

					p[c].x = (bCentreOffset ? bounds.GetCentre().X() : 0.f);
					p[c].y = max_ZY_Y;
					p[c].z = max_ZY_Z;
					c++;
				}

				Aftk::Matrix44 rotation;
				UtMaths::SetIdentity(&rotation);
				rotation[0][0] = -1.f;
				rotation[2][2] = -1.f;

				for (U32 i = 0; i < static_cast<U32>(bZYPlane ? 8 : 4); ++i)
				{
					if (!rotateY180)
						p[i] = Aftk::ApplyMatrix(rotation, p[i]);

					p[i] = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverse(), p[i]);
					posIdx[i] = meshNode.AddVertexPosition(p[i]);
				}
			}

			// vertex normals
			{
				{
					Aftk::Vector3F32 n(0.f, 0.f, -1.f);
					if (!rotateY180)
						n *= -1.f;
					if (bAntiClockwise)
						n *= -1.f;
					n = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverseTranspose(), n);
					UtMaths::SafeNormalise(&n);
					nIdx[0] = meshNode.AddVertexNormal((const Aftk::Normal3&)n);
				}
				if (bZYPlane)
				{
					Aftk::Vector3F32 n(1.f, 0.f, 0.f);
					if (!rotateY180)
						n *= -1.f;
					if (bAntiClockwise)
						n *= -1.f;
					n = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverseTranspose(), n);
					UtMaths::SafeNormalise(&n);
					nIdx[1] = meshNode.AddVertexNormal((const Aftk::Normal3&)n);
				}
			}

			// texxture coordinates
			{
				if (bAntiClockwise)
				{
					uvIdx[0] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 0.f, 0.f, 0.f));
					uvIdx[1] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 1.f, 0.f, 0.f));
					uvIdx[2] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 1.f, 0.f, 0.f));
					uvIdx[3] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 0.f, 0.f, 0.f));
				}
				else
				{
					uvIdx[0] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 0.f, 0.f, 0.f));
					uvIdx[1] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(0.f, 1.f, 0.f, 0.f));
					uvIdx[2] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 1.f, 0.f, 0.f));
					uvIdx[3] = meshNode.AddTextureCoordinate(0, Aftk::TextureCoordinate4(1.f, 0.f, 0.f, 0.f));
				}
			}

			// polygon vertices
			{
				for (S32 i = 0; i < (bZYPlane ? 8 : 4); ++i)
				{
					Aftk::C_PolygonVertex pv;
					pv.SetPositionIndex(posIdx[i]);
					pv.SetNormalIndex(nIdx[(S32)::floor(i / 4.f)]);
					if (i < 4)
						pv.SetTextureCoordinateIndex(0, uvIdx[i]);
					else
						pv.SetTextureCoordinateIndex(0, uvIdx[3 - (i % 4)]);

					pvIdx[i] = meshNode.AddPolygonVertex(pv);
				}
			}

			// material
			S16 materialMapping[3] = {-1, -1, -1};

			for (U32 i = 0; i < 3; ++i)
			{
				Aftk::C_MaterialNode material;
				material.SetOrigin(Aftk::E_MaterialOrigin_Maya);

				// add shader node
				Aftk::C_ShaderNode* pLodShader = new Aftk::C_ShaderNode;
				material.AddShaderNodePtr(pLodShader);
				pLodShader->SetName("LodShader");

				{
					Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
					material.AddPlugPtr(pNewPlug);
					pNewPlug->SetName("mayaSurfaceShader");
					pNewPlug->SetSourceConnectionPtr(pLodShader);
				}

				std::string colourPlugName = "color";

				// set billboard fx
				{
					// emulate maya hlsl shaders
					material.SetOrigin(Aftk::E_MaterialOrigin_Maya_Hlsl_2008);
					Aftk::C_DynamicPlug* pFXPlug = new Aftk::C_DynamicPlug();
					pLodShader->AddPlugPtr(pFXPlug);
					pFXPlug->SetValue(std::string("Default.fx"));
					pFXPlug->SetName("shader");

					colourPlugName = "Colour";
				}

				// add shader image node
				{
					Aftk::C_ShaderImageNode* pImageNode = new Aftk::C_ShaderImageNode;
					material.AddShaderNodePtr(pImageNode);
					pImageNode->SetName("file1");

					std::string imagePath = placeholderData.ImposterInfo.ImagePath[i];
					pImageNode->GetFilePlug().SetValue(imagePath);

					pImageNode->GetImageTypePlug().SetValue("");

					Aftk::C_DynamicPlug* pOutColourPlug = new Aftk::C_DynamicPlug();
					pImageNode->AddPlugPtr(pOutColourPlug);
					pOutColourPlug->SetName("outColour");

					Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
					pLodShader->AddPlugPtr(pPlug);
					pPlug->SetName(colourPlugName);
					pPlug->SetSourceConnectionPtr(pOutColourPlug);
				}

				const Aftk::C_MaterialResource *pMaterialResource = aftkScene.AddMaterialNode(material);
				materialMapping[i] = meshNode.AddPolygonMaterialMapping(pMaterialResource);
			}

			// now the actual polygons
			// XZ Plane
			{
				S32 polyIdx = meshNode.AddPolygon();
				Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

				if (bAntiClockwise)
				{
					polygon.AddPolygonVertexIndex(pvIdx[0]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
				}
				else
				{
					polygon.AddPolygonVertexIndex(pvIdx[3]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
				}
				polygon.SetMaterialMappingIndex(materialMapping[0]);
				polygon.SetBlindDataMappingIndex(blindDataMapping);
			}
			{
				S32 polyIdx = meshNode.AddPolygon();
				Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

				if (bAntiClockwise)
				{
					polygon.AddPolygonVertexIndex(pvIdx[0]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
					polygon.AddPolygonVertexIndex(pvIdx[3]);
				}
				else
				{
					polygon.AddPolygonVertexIndex(pvIdx[3]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
					polygon.AddPolygonVertexIndex(pvIdx[0]);
				}
				polygon.SetMaterialMappingIndex(materialMapping[0]);
				polygon.SetBlindDataMappingIndex(blindDataMapping);
			}
			// ZY Plane
			if (bZYPlane)
			{
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdx[4]);
						polygon.AddPolygonVertexIndex(pvIdx[5]);
						polygon.AddPolygonVertexIndex(pvIdx[6]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdx[7]);
						polygon.AddPolygonVertexIndex(pvIdx[6]);
						polygon.AddPolygonVertexIndex(pvIdx[5]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[2]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdx[4]);
						polygon.AddPolygonVertexIndex(pvIdx[6]);
						polygon.AddPolygonVertexIndex(pvIdx[7]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdx[7]);
						polygon.AddPolygonVertexIndex(pvIdx[5]);
						polygon.AddPolygonVertexIndex(pvIdx[4]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[2]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
			}

			if (bMakeDoubleSided)
			{
				S32 nIdxBack[2] = {-1, -1};
				{
					Aftk::Normal3 n = meshNode.GetVertexNormal(nIdx[0]);
					n *= -1.f;
					nIdxBack[0] = meshNode.AddVertexNormal(n);
				}
				if (bZYPlane)
				{
					Aftk::Normal3 n = meshNode.GetVertexNormal(nIdx[1]);
					n *= -1.f;
					nIdxBack[1] = meshNode.AddVertexNormal(n);
				}

				S32 pvIdxBack[8] = {-1, -1, -1, -1, -1, -1, -1 ,-1};

				for (S32 i = 0; i < (bZYPlane ? 8 : 4); ++i)
				{
					Aftk::C_PolygonVertex pv = meshNode.GetPolygonVertex(pvIdx[i]);
					pv.SetNormalIndex(nIdxBack[(S32)::floor(i / 4.f)]);
					pv.SetTextureCoordinateIndex(0, uvIdx[3 - (i % 4)]);
					pvIdxBack[i] = meshNode.AddPolygonVertex(pv);
				}

				// XY Plane
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[1]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				{
					S32 polyIdx = meshNode.AddPolygon();
					Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

					if (bAntiClockwise)
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
					}
					else
					{
						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
					}
					polygon.SetMaterialMappingIndex(materialMapping[1]);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				// ZY Plane
				if (bZYPlane)
				{
					{
						S32 polyIdx = meshNode.AddPolygon();
						Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

						if (bAntiClockwise)
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[7]);
							polygon.AddPolygonVertexIndex(pvIdxBack[6]);
							polygon.AddPolygonVertexIndex(pvIdxBack[5]);
						}
						else
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[4]);
							polygon.AddPolygonVertexIndex(pvIdxBack[5]);
							polygon.AddPolygonVertexIndex(pvIdxBack[6]);
						}
						polygon.SetMaterialMappingIndex(materialMapping[2]);
						polygon.SetBlindDataMappingIndex(blindDataMapping);
					}
					{
						S32 polyIdx = meshNode.AddPolygon();
						Aftk::C_Polygon& polygon = meshNode.GetPolygon(polyIdx);

						if (bAntiClockwise)
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[7]);
							polygon.AddPolygonVertexIndex(pvIdxBack[5]);
							polygon.AddPolygonVertexIndex(pvIdxBack[4]);
						}
						else
						{
							polygon.AddPolygonVertexIndex(pvIdxBack[4]);
							polygon.AddPolygonVertexIndex(pvIdxBack[6]);
							polygon.AddPolygonVertexIndex(pvIdxBack[7]);
						}
						polygon.SetMaterialMappingIndex(materialMapping[2]);
						polygon.SetBlindDataMappingIndex(blindDataMapping);
					}
				}
			}
		}

		return Aftk::C_Status::E_Status_Success;
	}*/

	Aftk::C_Status CreateCrowdLodGroupInstanceNode(Aftk::C_SceneNode* pPlaceholderInstance,
		const Aftk::C_SceneLodGroupNode* pLodMeshGroupNode, const C_CrowdPlaceholderData placeholderData,
		const bool rotateY180)
	{
		if (!pPlaceholderInstance || !pLodMeshGroupNode || !pPlaceholderInstance->GetParentPtr())
			return Aftk::C_Status::E_Status_Failure;

		if (pPlaceholderInstance->GetScenePtr() != pLodMeshGroupNode->GetScenePtr())
			return Aftk::C_Status::E_Status_Failure;

		Aftk::C_Scene* pScene = pPlaceholderInstance->GetScenePtr();

		// Create new lod group
		Aftk::C_SceneLodGroupNode* pNewLodInstanceGroupNode = new Aftk::C_SceneLodGroupNode(pScene);
		pPlaceholderInstance->GetParentPtr()->AddChildPtr(pNewLodInstanceGroupNode);

		pNewLodInstanceGroupNode->SetName(pPlaceholderInstance->GetName() + "_lod");

		pNewLodInstanceGroupNode->SetWorldMatrix(pPlaceholderInstance->GetWorldMatrix(), true);

		Maths::HMat44 inverseRoot;
		inverseRoot.SetIdentity();
		inverseRoot[2][2] = -1.f;

		Maths::HMat44 sceneTransform = AftkHelpers::GetHMat44(pNewLodInstanceGroupNode->GetLocalMatrix());
		sceneTransform = inverseRoot * sceneTransform;

		if (!rotateY180)
		{
			Maths::HMat44 rotation;

			Maths::HVec4 translation = sceneTransform.AW();

			rotation.AX(Maths::HVec4(-1.f, 0.f,  0.f, 0.f));
			rotation.AY(Maths::HVec4( 0.f, 1.f,  0.f, 0.f));
			rotation.AZ(Maths::HVec4( 0.f, 0.f, -1.f, 0.f));
			rotation.AW(Maths::HVec4( 0.f, 0.f,  0.f, 1.f));

			sceneTransform = rotation * sceneTransform;

			sceneTransform.AW(translation);
		}

		pNewLodInstanceGroupNode->SetLocalMatrix(AftkHelpers::GetMatrix44(sceneTransform));

		for (U32 i = 0; i < pPlaceholderInstance->GetNumLayers(); ++i)
		{
			Aftk::C_LayerNode* pLayerNode = const_cast<Aftk::C_LayerNode*>(pPlaceholderInstance->GetLayerPtr(i));

			if (pLayerNode)
				pLayerNode->AddMember(pNewLodInstanceGroupNode);
		}

		Aftk::Matrix44 instanceWorldMatrix = pPlaceholderInstance->GetWorldMatrix();

		Maths::HMat43 instanceWorldTransform;
		Aftk::TemplateToNightmareMatrix43(instanceWorldTransform, instanceWorldMatrix);

		Maths::HVec3 instanceScale;
		Maths::HVec3 instanceShear;
		Maths::HVec3 instanceRotate;
		Maths::HVec3 instanceTranslate;

		Aftk::GetSShRT(instanceScale, instanceShear, instanceRotate, instanceTranslate, instanceWorldTransform, pPlaceholderInstance->GetRotationOrder());

		for (U32 i = 0; i < pLodMeshGroupNode->GetNumLodLevels(); i++)
		{
			Aftk::C_GroupLodLevelInfo info = pLodMeshGroupNode->GetLodLevelInfo(i);

			Aftk::C_SceneNode* pLodMeshNode = info.GetSceneNodePtr();

			SetCrowdCropSheetFlags(pLodMeshNode);

			//Resolve any indirect instancing.
			if (pLodMeshNode->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				Aftk::C_SceneInstanceNode* pLodInstNode = (Aftk::C_SceneInstanceNode*)pLodMeshNode;
				if (pLodInstNode->GetSourceNodePtr() && 
					pLodInstNode->GetSourceNodePtr()->GetType() == Aftk::E_ObjType_SceneInstance)
				{
					pLodInstNode->SetSourceNodePtr(pLodInstNode->GetTopLevelSourceNodePtr());
				}
			}

			Aftk::C_SceneInstanceNode* pLodInstanceNode = new Aftk::C_SceneInstanceNode(pScene);

			//inherit layer membership
			for (U32 i = 0; i < pNewLodInstanceGroupNode->GetNumLayers(); ++i)
			{
				Aftk::C_LayerNode *pLayerNode = pNewLodInstanceGroupNode->GetLayerPtr(i);
				pLayerNode->AddMember(pLodInstanceNode);
			}

			pLodInstanceNode->SetName(pLodMeshNode->GetName() + "_inst");
			pLodInstanceNode->SetSourceNodePtr(pLodMeshNode);
			Aftk::Matrix44 id;
			UtMaths::SetIdentity(&id);
			
			pLodInstanceNode->SetLocalMatrix(id);

			pNewLodInstanceGroupNode->AddChildPtr(pLodInstanceNode);

			U32 behaviourHash = placeholderData.BehaviourHash;
			if (UseSecondaryBehaviour(pLodMeshGroupNode) == true)
				behaviourHash = placeholderData.SecondaryBehaviourHash;

			SetSkinInstanceNodeFlags(pLodInstanceNode, instanceTranslate, iCrowdInstanceIndex,
				((Aftk::C_SceneInstanceNode*)pPlaceholderInstance)->GetSourceNodePtr());

			info.SetSceneNodePtr(pLodInstanceNode);
			pNewLodInstanceGroupNode->AddLodLevelInfo(info);
		}

		/*if (pNewLodInstanceGroupNode->GetNumLodLevels() < kCrowdActorTotalLodLevelCount)
		{
			if (placeholderData.ImposterInfo.IsValid())
			{
				Aftk::C_SceneMeshNode* pMeshNode = new Aftk::C_SceneMeshNode(pScene);
				pMeshNode->SetName(placeholderData.Name + kCrowdActorLodLevelNames[kCrowdActorImposterLodLevelIndex]);

				if (GenerateImposterLodMesh(*pMeshNode, *pScene, placeholderData, rotateY180))
				{
					pNewLodInstanceGroupNode->AddChildPtr(pMeshNode);

					SetCrowdCropSheetFlags(pMeshNode);
					SetLightingFlags(pMeshNode, true);

					Aftk::C_SceneNode* pLodInstance0 = pNewLodInstanceGroupNode->GetLodLevelInfo(0).GetSceneNodePtr();
					Aftk::C_SceneNode* pLod0Mesh = ((Aftk::C_SceneInstanceNode*)pLodInstance0)->GetSourceNodePtr();

					Aftk::C_GroupLodLevelInfo lodLevelInfo;
					lodLevelInfo.SetSceneNodePtr(pMeshNode);
					lodLevelInfo.SetThreshold(kCrowdActorLodLevelThresholds[kCrowdActorImposterLodLevelIndex]);

					pNewLodInstanceGroupNode->AddLodLevelInfo(lodLevelInfo);
				}
				else
				{
					pScene->DeleteSceneNodeAndChildren(pMeshNode);
				}
			}
		}*/

		// MASSIVE HACK [rthomas]	Finally got fed up trying to fix weirdness in imposter positions when
		//							the rotation is within certain ranges ((89 < x < 91) and (-91 < x < -89)).
		//							So screw it, I'll just fudge the rotation and take it out of those ranges.
		//							It shouldn't make the resulting rotation different enough from the artist
		//							set value in maya to be noticable (I hope) :)
		//
		//							For future reference I think there might be a bug in GetSShRT...
		//							but not completely sure. Also it occurs when AftkUtSnapVertices freezes transforms
		//							by applying the matrix to the scene mesh node and then setting world matrix to identity.

		// Follow Up				Spoke to Ken and he has seen this occur before and has fixed it in the trunk.
		//							Fix is a change to an epsilon value which would affect too many things so
		//							is too risky to commit at the moment.
		//							So for this reason I am enabling this hack until Euler.cpp gets changed in blur release
#if 1
		{
			Maths::HMat43 temp;
			Aftk::TemplateToNightmareMatrix43(temp, pNewLodInstanceGroupNode->GetWorldMatrix());

			Maths::HVec3 scale;
			Maths::HVec3 shear;
			Maths::HVec3 rotate;
			Maths::HVec3 translate;

			Aftk::GetSShRT(
				scale,
				shear,
				rotate,
				translate, 
				temp, 
				pNewLodInstanceGroupNode->GetRotationOrder());

			F32 rotYDeg = rotate.Y() * (180.f / Maths::Pi);
			if (rotYDeg > 89.f && rotYDeg < 91.f)
			{
				if (rotYDeg <= 90.f)
					rotYDeg = 89.f;
				else
					rotYDeg = 91.f;
			}
			else if (rotYDeg < -89.f && rotYDeg > -91.f)
			{
				if (rotYDeg >= -90.f)
					rotYDeg = -89.f;
				else
					rotYDeg = -91.f;
			}
			rotate.Y(rotYDeg * (Maths::Pi / 180.f));

			Maths::HMat33 id;
			id.SetIdentity();

			Aftk::BuildMatrix(
				temp,
				Maths::HVec3(0.f,0.f,0.f),	//no scale pivot
				scale,
				shear,
				Maths::HVec3(0.f,0.f,0.f),	//no scale pivot translation
				Maths::HVec3(0.f,0.f,0.f),	//no rotate pivot
				id,							//no rotate orientation matrix
				rotate,
				Maths::HVec3(0.f,0.f,0.f),	//no rotate pivot translation
				translate,
				pNewLodInstanceGroupNode->GetRotationOrder());

			pNewLodInstanceGroupNode->SetWorldMatrix(AftkHelpers::GetMatrix44(Maths::HMat44(temp)));
		}
#endif

		// id of another scene node as blind data :(
		if (pNewLodInstanceGroupNode && pNewLodInstanceGroupNode->GetNumLodLevels())
		{
			const Aftk::C_SceneNode* pMeshNodeInst = pNewLodInstanceGroupNode->GetLodLevelInfo(0).GetSceneNodePtr();
			
			if (pMeshNodeInst && AftkHelpers::IsInstance(pMeshNodeInst))
			{
				//const Aftk::C_SceneNode* pMeshNode = AftkHelpers::InstanceGetSourceNode(pMeshNodeInst);

				Aftk::C_GenericAttribute sceneNodeID;

				sceneNodeID.SetName("SceneNodeID");
				//sceneNodeID.SetValue(pMeshNode->GetUniqueId());
				sceneNodeID.SetValue(pMeshNodeInst->GetUniqueId());

				Aftk::C_BlindDataNode blindData;
				blindData.AddAttribute(sceneNodeID);

				const Aftk::C_BlindDataResource *pResourcePtr = pScene->AddBlindDataNode(blindData);
				pPlaceholderInstance->SetBlindDataResourcePtr(pResourcePtr);
			}
		}

		AftkUt::CrowdHelper::SetCrowdInstanceIndex(pPlaceholderInstance, iCrowdInstanceIndex++);

		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_Status ReplaceNode(Aftk::C_Scene* pAftkScene, Aftk::C_SceneNode* pSourceNode,
		const C_CrowdSkinnedSceneDescription& desc, C_CrowdSpectatorData* pSpectator,
		const C_CrowdPlaceholderData placeholderData, Util::C_Random& random, const bool rotateY180, U32 &failureCount)
	{
		if (!pSourceNode || !AftkUt::CrowdHelper::IsBaseCrowdInstance(pSourceNode))
		{
			if (pSourceNode)
				failureCount += pSourceNode->GetNumInstancesOfThisNode();
			return Aftk::C_Status::E_Status_Failure;
		}

		if (!pSpectator || !pSpectator->GetNumGroupNodes())
		{
			failureCount += pSourceNode->GetNumInstancesOfThisNode();
			return Aftk::C_Status::E_Status_Failure;
		}

		// Make a list of the source node's instances
		S32 invalidInstanceNames = 0;

		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > nodeInstances;
		for (U32 i = 0, count = pSourceNode->GetNumInstancesOfThisNode(); i < count; i++)
		{
			Aftk::C_SceneNode* pNodeInstance = pSourceNode->GetInstanceOfThisNode(i);

			if (pNodeInstance)
			{
				if (_stricmp(pNodeInstance->GetName().substr(0, 2).c_str(), "M_") == 0)
				{
					invalidInstanceNames++;
					pNodeInstance->SetName("IRM_" + pNodeInstance->GetName());
				}

				nodeInstances.PushBack(pSourceNode->GetInstanceOfThisNode(i));
			}
		}

#ifdef CROWD_DEBUG_COPY_SOURCE_NODE
		Aftk::C_SceneMeshNode* pDummyNode = new Aftk::C_SceneMeshNode(*((Aftk::C_SceneMeshNode*)pSourceNode));
		pDummyNode->SetName(pSourceNode->GetName() + "_dummy");
#else
		Aftk::C_SceneNode* pDummyNode = new Aftk::C_SceneNode(pAftkScene);
		*pDummyNode = *pSourceNode;
		pDummyNode->SetName(pSourceNode->GetName() + "_dummy");
#endif

		bool completeSuccess = true;

		U32 iInstanceIndex = 0;
		while (iInstanceIndex < pSourceNode->GetNumInstancesOfThisNode())
		{
			if (!CreateCrowdLodGroupInstanceNode(pSourceNode->GetInstanceOfThisNode(iInstanceIndex),
				pSpectator->GetRandomGroupNode(random), placeholderData, rotateY180))
			{
				completeSuccess = false;
				iInstanceIndex++;
				failureCount++;
			}
			else
			{
				AftkUt::C_Utility::ReparentInstance(pSourceNode->GetInstanceOfThisNode(iInstanceIndex), pDummyNode);
			}
		}

		if (completeSuccess)
		{
			if (!pDummyNode->Replace(pSourceNode, true))
			{
				LOGERROR( Tools::g_log, "[Crowd] Unable to replace scene node '%s'", pSourceNode->GetName().c_str() );
				return Aftk::C_Status::E_Status_Failure;
			}
		}
		else
		{
			if (!pDummyNode->GetNumInstancesOfThisNode())
				pAftkScene->DeleteSceneNodeAndChildren(pDummyNode);
		}

		return Aftk::C_Status::E_Status_Success;
	}

	U32 Do(Aftk::C_Scene* pAftkScene, Aftk::C_SceneNode* pSceneNode, const U32 seed, const bool rotateY180)
	{
		Aftk::C_Scene* pScene = pSceneNode->GetScenePtr();
		assert(pScene);

		g_NodeNameMap.Clear();
		g_NodeNameCount.Clear();

		Ut::Vector<Aftk::C_SceneNode*> placeholderNodes;
		LocateCrowdNodes(placeholderNodes, pSceneNode);

		Ut::Vector<C_CrowdPlaceholderData> placeholderDataList;
		Btl::Map<U32, U32> placeholderDataMap;
		
		for (S32 i = 0, count = placeholderNodes.Size(); i < count; ++i)
		{
			if (placeholderNodes[i])
			{
				C_CrowdPlaceholderData data;
				data.Create(placeholderNodes[i]);

				U32 index = 0;
				
				if (Contains(placeholderDataList, data, &index))
				{
					// Already in list at index specified
					placeholderDataMap.Insert(i, index);
				}
				else
				{
					// Not in list so add it
					placeholderDataMap.Insert(i, static_cast<U32>(placeholderDataList.Size()));
					placeholderDataList.PushBack(data);
				}
			}
		}

		C_CrowdSkinnedSceneDescription desc;
		desc.Build(pSceneNode, seed);

		Util::C_Random random;
		random.Seed(seed);

		U32 replacementCount = 0;
		U32 totalPotentialReplacements = static_cast<U32>(placeholderNodes.Size());

		U32 successCount = 0;
		U32 failCount = 0;

		U32 meshDatabaseSize = 0;

		if (placeholderNodes.Size())
		{
			C_CrowdSpectatorDatabase database;
			meshDatabaseSize = database.Build(placeholderNodes, placeholderDataList, placeholderDataMap, desc, pAftkScene, seed, rotateY180);

			S32 spectatorIndex = 0;
			while (placeholderNodes.Size() != 0)
			{
				Ut::Vector<Aftk::C_SceneNode*>::Iterator it = placeholderNodes.Begin();

				U32 numInstancesToBeReplaced = (*it)->GetNumInstancesOfThisNode();
				U32 numFailed = 0;

				Btl::Map<U32, U32>::ConstIterator dataIter = placeholderDataMap.Find(spectatorIndex++);

				C_CrowdPlaceholderData placeholderData;
				C_CrowdSpectatorData* pSpectator = database.GetSpectator(0);

				if (dataIter != placeholderDataMap.End())
				{
					pSpectator = database.GetSpectator(dataIter->GetValue());
					placeholderData = placeholderDataList[dataIter->GetValue()];
				}

				if (ReplaceNode(pAftkScene, *it, desc, pSpectator, placeholderData, random, rotateY180, numFailed))
					replacementCount++;
				else
					AftkUt::CrowdHelper::StripCrowdFlags(*it);

				failCount += numFailed;
				successCount += (numInstancesToBeReplaced - numFailed);

				placeholderNodes.Erase(it);
			}

			database.End();
		}

		desc.End();

		g_NodeNameMap.Clear();
		g_NodeNameCount.Clear();

		LOGDEBUG( Tools::g_log, "[Crowd] Instances Replaced: %d/%d/%d", successCount, failCount, (successCount + failCount) );
		LOGDEBUG( Tools::g_log, "[Crowd] Bases Replaced: %d/%d/%d", replacementCount, (totalPotentialReplacements - replacementCount), totalPotentialReplacements );
		LOGDEBUG( Tools::g_log, "[Crowd] Meshes Tagged for Crop Sheet: %d/%d/%d", iNumMeshesTaggedForCropSheet, (meshDatabaseSize - iNumMeshesTaggedForCropSheet), meshDatabaseSize);

		return replacementCount;
	}
}; // namespace AftkCrowdReplaceSourceNodes
