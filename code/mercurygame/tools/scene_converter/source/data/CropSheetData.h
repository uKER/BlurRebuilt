#ifndef APP_CROPSHEET_DATA_H
#define APP_CROPSHEET_DATA_H

#ifdef _MSC_VER
#pragma once
#endif


class C_DataCropSheet : public Tools::ProcessGraph::C_Data
{
public:
	enum E_CropSheetType
	{
		E_CropSheetType_Crowd,			//normal diffuse only cropsheet
		E_CropSheetType_CrowdNeon,		//DIFF, IPR0, NEON cropsheet
		E_CropSheetType_CrowdHighVis,	//DIFF, IPR0 cropsheet (HighVis)
		E_CropSheetType_CrowdImposters,	//_BACK, _FRONT, _SIDE cropsheet (for imposters, consider revising crappy naming convention [rthomas])
		E_CropSheetType_Count
	};

	C_DataCropSheet();
	~C_DataCropSheet();

	const Util::C_String GetName() const;

	TexAtlas::Database& GetDatabase(E_CropSheetType type);
	const TexAtlas::Database& GetDatabase(E_CropSheetType type) const;

private:
	TexAtlas::Database	m_databases[E_CropSheetType_Count];
};

#endif