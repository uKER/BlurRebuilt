#ifndef LUX_POLYFLAGS_H
#define LUX_POLYFLAGS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace LUX
{
	struct PolyFlags
	{
		PolyFlags()
		{
			flags = 0;
			lightmapRes = 1;
		}

		union
		{
			U32 flags;

			struct
			{
				U32 doubleSided : 1;
				U32 emitLight : 1;
				U32 doNotReceiveShadows : 1;
				U32 doNotCastShadows : 1;
				U32 lightmapStop : 1;
				U32 noCollision : 1;
				U32 groundCollision : 1;
				U32 lightmapRes : 3;
			};
		};
	};

	inline U32 GetDensity( const Aftk::C_GenericAttribute& aftkAttribute )
	{
		switch(aftkAttribute.GetDataType())
		{
			default:
			break;

			case Aftk::E_GenericAttributeType_String:
			{
				std::string value;
				aftkAttribute.GetValue(value);

				const char *densityNames[] = { "High", "Normal", "Low", "VeryLow", "UltraLow" };

				for(U32 i=0; i<5; i++)
				{
					if(_stricmp( value.c_str(), densityNames[i] ) == 0 )
					{
						return i;
					}
				}		
			}
			break;
		}

		return 1;
	}

	inline PolyFlags GetPolyFlags(
		const Aftk::C_Polygon *pPoly,
		int iDoubleSidedPolyFlagMask,
		int iDoNotLightPolyFlagMask,
		int iLightmapEndFlagMask,
		int iEmitLightPolyFlagMask,
		int iLowResLightmapPolyFlagMask,
		int iLowResLightmapBits)
	{
		const Aftk::C_BlindDataResource* pBlindData = GetBlindDataResourcePtr(pPoly);

		if (!pBlindData) 
			return PolyFlags();

		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindData->GetContent();

		//Create a new poly-flags structure in which we'll store our results.
		PolyFlags polyFlags;

		//Process new poly-flag system. This will potentially override legacy flags if both legacy
		//and poly-set flags are present.
		for (U32 j = 0; j < aftkBlindDataNode.GetNumAttributes(); j++) 
		{
			const Aftk::C_GenericAttribute&	aftkAttribute = aftkBlindDataNode.GetAttribute(j);

			if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_doNotCastShadows"))
				polyFlags.doNotCastShadows = 1;
			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_doNotLight"))
				polyFlags.doNotReceiveShadows = 1;
			// this needs to be changed to use the new name-value pair stuff in the new polyflag system
			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySetDropDown_lightmapResolution"))
				polyFlags.lightmapRes = GetDensity( aftkAttribute );
			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_emitLight"))
				polyFlags.emitLight = 1;
			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_lightmapStop"))
				polyFlags.lightmapStop = 1;
			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_doubleSided"))
				polyFlags.doubleSided = 1;
			else if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_collisionGround"))
				polyFlags.groundCollision = 1;
		}

		return polyFlags;
	}
}

#endif
