#include "stdafx.h"

#include <tex_atlas/TexAtlas.h>

#include <converter_shared/ProcessGraph.h>
#include <data/DataTypes.h>
#include <data/CropSheetData.h>


C_DataCropSheet::C_DataCropSheet()
{
}

C_DataCropSheet::~C_DataCropSheet()
{
}

const Util::C_String C_DataCropSheet::GetName() const
{
	return Util::C_String("C_DataCropSheet");
}

TexAtlas::Database& C_DataCropSheet::GetDatabase(E_CropSheetType type)
{
	return m_databases[type];
}

const TexAtlas::Database& C_DataCropSheet::GetDatabase(E_CropSheetType type) const
{
	return m_databases[type];
}
