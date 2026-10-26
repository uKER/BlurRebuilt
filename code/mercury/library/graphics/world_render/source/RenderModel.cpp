
#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/RenderModel.h>
#include <renderingdata/RenderingData.h>

namespace WorldRender
{

enum E_ModelRenderFileVersion
{
	E_ModelRenderFileVersion_Initial = 0,

	E_ModelRenderFileVersion_Current = E_ModelRenderFileVersion_Initial
};

// C_RenderModel

C_RenderModel::C_RenderModel()
{
}

C_RenderModel::~C_RenderModel()
{
}

void C_RenderModel::OnLoad(RenderingData::Target::EffectVariationCallbackFunc pCallback)
{
	GetScene().OnLoad(pCallback);
}
	
bool C_RenderModel::Read( Sys::I_File* pFile, const Util::C_String& paramType )
{
	bool success = true;

	RESOURCE_MEMORY_ID("ModelData");

	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_ModelRenderFileVersion_Initial;

	Sys::C_FileBlock rootBlock( pFile );
	if( rootBlock.HeaderId() != "Render" )
	{
		LOGWARN( g_Log, "Expected a 'Render' Block" );
		return false;
	}

	// keep reading child blocks
	while ( rootBlock.GetPosition() < rootBlock.Length() )
	{
		Sys::C_FileBlock block( &rootBlock );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> fileVersion;
			pFile->SetVersionNumber( fileVersion );
		}
		else			
		if( block.HeaderId() == "Scene" )
		{
			ReadScene( &block, m_pScene );
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	pFile->SetVersionNumber( prevFileVersion );

	return true;
}



} // namespace WorldRender
