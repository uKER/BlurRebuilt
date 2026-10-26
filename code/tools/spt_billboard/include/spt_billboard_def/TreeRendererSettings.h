#ifndef SPEEDTREE_BILLBOARD_TREE_RENDERER_SETTINGS_INCLUDE
#define SPEEDTREE_BILLBOARD_TREE_RENDERER_SETTINGS_INCLUDE


#ifdef _MSC_VER
#pragma once
#endif

namespace SpeedtreeBillboard
{

	extern Debug::Logger& g_Log;

	enum E_Lighting
	{
		E_Lighting_None,
		E_Lighting_VertexColours
	};

	enum E_Conditions
	{
		E_Condition_Sunny = 0,
		E_Condition_Night = 0,
		E_Condition_Overcast = 0,
		E_Condition_Snow
	};

	class C_TreeRendererSettings
	{
	public:

		// ViewPos = k * ( RotateY(azimuth)*RotateZ(elevation)*(1,0,0))	+ TreePosition where k = distance from tree
		struct View
		{
			S32 m_Azimuth;
			S32 m_Elevation;
		};

		C_TreeRendererSettings();
		~C_TreeRendererSettings();

		// Width & height of final texture atlas - must be a power of 2
		U32 m_AtlasWidth;
		U32 m_AtlasHeight;
		Util::C_String m_Filename;
		Util::C_String m_FileFormat;
		Util::C_String m_DefinitionFilename;

		U32 m_MinBillboardWidth;
		U32 m_MinBillboardHeight;

		E_Lighting m_Lighting;

		Ut::UtVector< Ut::DynamicVector<U32> > m_Conditions;
		Ut::UtVector< Ut::DynamicVector<View> > m_Views;

		static bool ReadSettingsFromXml(
			C_TreeRendererSettings& settings,
			const Util::C_String& filename);

		static bool WriteSettingsToXml(
			const C_TreeRendererSettings& settings,
			const Util::C_String& filename);
	};
}

#endif

