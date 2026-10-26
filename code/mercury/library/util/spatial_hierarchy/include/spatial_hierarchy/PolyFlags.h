#ifndef SD_POLYFLAGS_H
#define SD_POLYFLAGS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace SD
{
	struct PolyFlags
	{
		PolyFlags()
		{
			flags = 0;
		}

		union
		{
			U32 flags;

			struct
			{
				U32 doubleSided : 1;
				U32 noCollision : 1;
			};
		};
	};

	inline PolyFlags GetPolyFlags(
		const Aftk::C_Polygon *pPoly,
		int iDoubleSidedPolyFlagMask,
		int iNoCollisionPolyflag)
	{
		const Aftk::C_BlindDataResource* pBlindData = GetBlindDataResourcePtr(pPoly);

		if (!pBlindData) 
			return PolyFlags();

		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindData->GetContent();

		//Create a new poly-flags structure in which we'll store our results.
		PolyFlags polyFlags;

		//Process legacy poly-flags as used in PGR3
		for (U32 j = 0; j < aftkBlindDataNode.GetNumAttributes(); j++) 
		{
			const Aftk::C_GenericAttribute&	aftkAttribute = aftkBlindDataNode.GetAttribute(j);
			if (!stricmp(aftkAttribute.GetName().c_str(), "polyflags"))
			{
				U32 bits = 0;
				aftkAttribute.GetValue(bits);
				if (bits & iDoubleSidedPolyFlagMask)
					polyFlags.doubleSided = 1;
				if (bits & iNoCollisionPolyflag)
					polyFlags.noCollision = 1;
			}
		}

		//Process new poly-flag system. This will potentially override legacy flags if both legacy
		//and poly-set flags are present.
		for (U32 j = 0; j < aftkBlindDataNode.GetNumAttributes(); j++) 
		{
			const Aftk::C_GenericAttribute&	aftkAttribute = aftkBlindDataNode.GetAttribute(j);
			if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_doubleSided"))
				polyFlags.doubleSided = 1;

			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_doNotCastShadows"))
				polyFlags.noCollision = 1;
		}

		return polyFlags;
	}

	inline void SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef)
	{
		Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

		if (pRef)
		{
			S16 bdLocalIndex = pMesh->AddPolygonBlindDataMapping(pRef);
			assert(bdLocalIndex != -1);
			pPoly->SetBlindDataMappingIndex(bdLocalIndex);
		}
		else
		{
			pPoly->SetBlindDataMappingIndex(-1);
		}
	}

	inline void SetPolyFlags(
		Aftk::C_Polygon* pPoly,
		const Aftk::C_BlindDataNode* pOldBlindData,
		const PolyFlags& newPolyFlags,
		int iDoubleSidedPolyFlagMask,
		int iNoCollisionPolyflag)
	{
		Aftk::C_BlindDataNode newBlindData;
		if (pOldBlindData)
		{
			newBlindData.SetName(pOldBlindData->GetName());
			for (U32 i = 0; i < pOldBlindData->GetNumAttributes(); i++)
			{
				const Aftk::C_GenericAttribute& attr = pOldBlindData->GetAttribute(i);

				if (!stricmp(attr.GetName().c_str(), "polySet_doubleSided") && newPolyFlags.doubleSided)
				{
					newBlindData.AddAttribute(attr);
				}
				else if (!stricmp(attr.GetName().c_str(), "polySet_dontCastShadows") && newPolyFlags.noCollision)
				{
					newBlindData.AddAttribute(attr);
				}
				else
				{
					//We're not concerned with this attribute, add it.
					newBlindData.AddAttribute(attr);
				}
			}
		}
		//Else we didn't previously have any blind data.
		else
		{
			Aftk::C_GenericAttribute attr;
			if (newPolyFlags.doubleSided)
			{
				attr.SetName("polySet_doubleSided");
				newBlindData.AddAttribute(attr);
			}
			if (newPolyFlags.noCollision)
			{
				attr.SetName("polySet_dontCastShadows");
				newBlindData.AddAttribute(attr);
			}
		}

		//Add new blind data node to our scene.
		const Aftk::C_BlindDataResource* pNewBlindDataResource = 
			pPoly->GetMeshNodePtr()->GetScenePtr()->AddBlindDataNode(newBlindData);

		SetBlindDataResourcePtr(pPoly, pNewBlindDataResource);
	}
}

#endif
