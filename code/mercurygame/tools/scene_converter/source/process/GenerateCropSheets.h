#ifndef APP_GENERATE_CROPSHEETS_H
#define APP_GENERATE_CROPSHEETS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessGenerateCropSheets : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process (Progress::I_ProgressBar *pProgress);

	// inherited from C_Process
	const Util::C_String GetName() const;

private:

	struct TextureData
	{
		union Flags
		{
			struct
			{
				bool hasIPR0	: 1;
				bool hasNEON	: 1;
				bool isHighVis	: 1;
				bool isImposter	: 1;
			};
			U32 value;
		};

		Util::C_String path;
		Flags flags;
	};

	typedef Ut::Vector<TextureData>	TextureDataArray;

	void							GetCrowdNodes(C_AftkNodeList& nodes);
	void							BuildCrowdTextureList(TextureDataArray& textureList);
	bool							BuildCrowdCropSheet();
	
	const Aftk::C_MaterialResource*	CreateAtlasMaterialResource(Aftk::C_Scene *pScene, const std::string& fileName, const Aftk::C_MaterialNode* pMaterialNode = NULL);
	bool							RemapCrowdMaterialAndUVs(const TexAtlas::Database& database);
};

#endif