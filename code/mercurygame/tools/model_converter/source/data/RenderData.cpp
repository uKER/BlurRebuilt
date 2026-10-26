#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/RenderData.h>

#include <converter_shared/RenderDataConverter.h>

C_DataRenderData::C_DataRenderData( const Util::C_String textureCachePath, const Util::C_String fXBPath )
{
	m_pConverter = new RenderingData::C_Converter( textureCachePath, fXBPath);
}

C_DataRenderData::~C_DataRenderData()
{
	delete m_pConverter;
}

const Util::C_String C_DataRenderData::GetName() const
{
	return Util::C_String("C_DataRenderData");
}

enum E_ModelRenderFileVersion
{
	E_ModelRenderFileVersion_Initial = 0,

	E_ModelRenderFileVersion_Current = E_ModelRenderFileVersion_Initial
};

bool C_DataRenderData::Export( Sys::I_File* pFile, const Util::E_Platform platform ) const
{
	Sys::C_FileBlock rootBlock(pFile, "Render");

	// Header Block
	{
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");

		Sys::BinarySerialiserAutoEndian ar( headerBlock );
		ar << (S32)E_ModelRenderFileVersion_Current;
	}

	// Scene Block
	{
		Sys::C_FileBlock sceneBlock(&rootBlock, "Scene");

		switch( platform )
		{
			case Util::E_Platform_360:
				m_pConverter->ExportBinary(&sceneBlock, 0, RenderingData::PLATFORM_TYPE_360);
			break;
			case Util::E_Platform_PS3:
				m_pConverter->ExportBinary(&sceneBlock, 0, RenderingData::PLATFORM_TYPE_PS3);
			break;
			case Util::E_Platform_PC:
				m_pConverter->ExportBinary(&sceneBlock, 0, RenderingData::PLATFORM_TYPE_PC);
			break;
			default:
				assert( 0 && "Platform not supported" );
			break;
		}
	}

	return true;
}
