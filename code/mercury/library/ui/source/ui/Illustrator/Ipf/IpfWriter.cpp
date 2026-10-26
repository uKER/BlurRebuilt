
#include "stdafx.h"
#include "Ui/Illustrator/Ipf/IpfWriter.h"
#include "Ui/Illustrator/Ipf/IpfData.h"

#include <system/misc/stringutil.h>

#include "Ui/Illustrator/UIAdobeScene.h"
#include "Ui/Illustrator/UIAdobeGroup.h"
#include "Ui/Illustrator/UIAdobeShape.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

using namespace UI;

namespace Ipf
{

U32 C_IpfWriter::m_VersionNumber = ( (IPF_VersionMajor<<16) | IPF_VersionMinor );

E_Result C_IpfWriter::ExportScene( const char* pFilepath, const C_IpfData* pData )
{
	// Interagate the file incase its read only. Open returns null and doesnt imform you why??
	Sys::C_FileInfo fileInfo;
	Sys::GetDefaultFileSystem()->FileInfo( pFilepath, &fileInfo );	

	// Write the contents of the object class to a binary file.
	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open( pFilepath, Sys::E_FileAccess_Write);         

	if ( !pFile )
	{
		if ( fileInfo.m_FileAccess |= Sys::E_FileAccess_Read )
		{
			return E_Result_ReadOnly;
		}
		else
			return E_Result_Failed;
	}

	// Actual writing of files.
	IpfSerialiser serial(*pFile);

	// Write out the version number
	serial << m_VersionNumber;

	// Write each layer out containing the groups and data.
	serial << pData->GetNumberOfLayers();
	for ( S32 i = 0; i < pData->GetNumberOfLayers(); i++ )
	{
		WriteLayer( pData->m_LayerList[i], serial );
	}

	delete pFile;
	return E_Result_Success;
}


const bool C_IpfWriter::WriteLayer( const UI::C_AdobeGroup* pGroup, IpfSerialiser& stream )
{
	WriteGroup( pGroup, stream ); 

	return true;
}


const bool C_IpfWriter::WriteGroup( const UI::C_AdobeGroup* pGroup, IpfSerialiser& stream )
{
	stream  << (S32)pGroup->GetElementType()
			<< pGroup->GetLevel() 
			<< pGroup->GetName()
			<< pGroup->GetSize();

	for ( S32 i = 0; i < pGroup->GetSize(); i++ )
	{
		const C_AdobeElement* pElement = pGroup->GetElementPtr( i );

		if ( pElement->GetElementType() == E_ElementType_Group )
		{
			WriteGroup( static_cast< const C_AdobeGroup* >( pElement ), stream ); 
		}
		else
		if ( pElement->GetElementType() == E_ElementType_Polygon )
		{
			WriteShape( static_cast< const C_AdobePolygon* >( pElement ), stream ); 
		}
	}
	return true;
}


const bool C_IpfWriter::WriteShape( const UI::C_AdobePolygon* pShape, IpfSerialiser& stream )
{
	stream  << (S32)pShape->GetElementType()
			<< pShape->GetLevel() 
			<< pShape->GetName();

	stream	<< pShape->m_FillStyle
			<< pShape->m_StrokeStyle
			<< pShape->m_BlendStyle;

	stream	<< pShape->m_BoundingBox
			<< pShape->m_ScalarOffset
			<< pShape->m_ScalarValue;

	// Write fill verticies
	stream << pShape->m_FillVertexList.Size();
	for ( S32 path = 0; path < pShape->m_FillVertexList.Size(); path++ )
	{
		const UI::TVerticies& verticies = pShape->m_FillVertexList[path];
		stream << verticies.Size();

		for ( S32 i = 0; i < verticies.Size(); i++ )
		{
			stream	<< verticies[i].m_Pos
					<< verticies[i].m_Normal;
		}
	}

	// Write fill indicies
	stream << pShape->m_FillIndexList.Size();
	for ( S32 path = 0; path < pShape->m_FillIndexList.Size(); path++ )
	{
		const UI::TIndicies& indicies = pShape->m_FillIndexList[path];
		stream << indicies.Size();

		for ( S32 i = 0; i < indicies.Size(); i++ )
			stream	<< indicies[i];
	}

	// Write stroke verticies
	stream << pShape->m_StrokeVertexList.Size();
	for ( S32 path = 0; path < pShape->m_StrokeVertexList.Size(); path++ )
	{
		const UI::TVerticies& verticies = pShape->m_StrokeVertexList[path];
		stream << verticies.Size();

		for ( S32 i = 0; i < verticies.Size(); i++ )
		{
			stream	<< verticies[i].m_Pos
					<< verticies[i].m_Normal
					<< verticies[i].m_UV;
		}
	}

	return true;
}

}; // namespace Ipf

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif