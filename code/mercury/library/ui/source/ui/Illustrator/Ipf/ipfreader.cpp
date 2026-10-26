
#include "stdafx.h"
#include "Ui/Illustrator/Ipf/IpfReader.h"

#include <system/misc/stringutil.h>

#include "Ui/Illustrator/UIAdobeScene.h"
#include "Ui/Illustrator/UIAdobeGroup.h"
#include "Ui/Illustrator/UIAdobeShape.h"

// TODO : This shape vertex declaration should go in its own file and not share polygon font one
#include "Ui/Graphics/Simple2D.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

using namespace UI;

namespace Ipf
{

U32 C_IpfReader::m_VersionNumber = ( (IPF_VersionMajor<<16) | IPF_VersionMinor );

E_Result C_IpfReader::ImportScene( Sys::I_FileSystem& fs, const char* pFilepath, UI::C_AdobeScene* pScene )
{
	// Check that the file extension is correct.
	if( Sys::ExtractFileExtension( pFilepath ) != "ipf" )
	{
		LOGWARN( g_Log, DBSTR "Failed to read '%s', \".ipf\" file type expected!!", pFilepath );
		return E_Result_FileNotSupported;
	}

	// Write the contents of the object class to a binary file.
	Sys::I_File *pFile = fs.Open( pFilepath, Sys::E_FileAccess_Read);         

	if ( !pFile )
		return E_Result_FileNotFound;

	IpfDeserialiser serial( *pFile );

	//----------------------------------------------------------
	// Check the version number on the file.
	//----------------------------------------------------------
	U32	sVersionNumber = 0;	
	serial >> sVersionNumber;

	// Check major version number (top 16)
	if( (sVersionNumber >> 16) != IPF_VersionMajor )
	{
		LOGWARN( g_Log, DBSTR "File version (%d.%d) has been deprecated - current version is : (%d.%d)\n", (sVersionNumber >> 16), sVersionNumber&0xffff, IPF_VersionMajor, IPF_VersionMinor );

		// Clean up and bail
		delete pFile;
		return E_Result_VersionDeprecated;
	}
	// Major version (bottom 16)
	S32 sMinorVersionNumber = (sVersionNumber & 0xffff);

#ifndef CFG_GOLD
	if( sMinorVersionNumber < IPF_VersionMinor )
	{
		LOGWARN( g_Log, DBSTR "File version (%d.%d) is old not all features will be supported\n", (sVersionNumber >> 16), sVersionNumber&0xffff );
	}
#endif

	//----------------------------------------------------------
	// Start to read in the ipf data
	//----------------------------------------------------------
	U32 numLayers = 0;
	serial >> numLayers;

	for ( U32 i = 0; i < numLayers; i++ )
	{
		// Check its a layer
		S32 type = 0; 
		serial >> type;

		if ( type == E_ElementType_Group )
		{
			ReadLayer( pScene, serial );
		}
	}

	delete pFile;
	return E_Result_Success;
}


const bool C_IpfReader::ReadLayer( UI::C_AdobeScene* pScene, IpfDeserialiser& stream )
{
	Util::C_String name;
	U32 level = 0;
	S32 size = 0;
	S32 type = 0; 

	stream	>> level
			>> name
			>> size;

	pScene->AddLayer( name, size );

	for ( S32 i = 0; i < size; i++ )
	{
		stream >> type;

		if ( type == E_ElementType_Group )
		{
			ReadGroup( pScene, stream ); 
		}
		else
		if ( type == E_ElementType_Polygon )
		{
			ReadShape( pScene, stream ); 
		}
	}
	return true;
}


const bool C_IpfReader::ReadGroup( UI::C_AdobeScene* pScene, IpfDeserialiser& stream )
{
	Util::C_String name;
	U32 level = 0;
	S32 size = 0;
	S32 type = 0; 

	stream	>> level 
			>> name 
			>> size;

	pScene->AddGroup( level, name );

	for ( S32 i = 0; i < size; i++ )
	{
		stream >> type;

		if ( type == E_ElementType_Group )
		{
			ReadGroup( pScene, stream ); 
		}
		else
		if ( type == E_ElementType_Polygon )
		{
			ReadShape( pScene, stream ); 
		}
	}
	pScene->CloseGroup();
	return true;
}


const bool C_IpfReader::ReadShape( UI::C_AdobeScene* pScene, IpfDeserialiser& stream )
{
	Util::C_String name;
	U32 level = 0;
	S32 paths = 0;
	S32 size = 0;
	U16 index = 0;
	Maths::HVec2 pos, uv;
	Maths::HVec4 normal;

	// Create a new shape and add it to the scene
	C_AdobeShape* pShape = new C_AdobeShape();

	pShape->Deserialise( stream );
	pScene->AddShape( pShape );

	// Read verticies and indicies for fill
	stream >> paths;

	UI::TVertexList fillVertexList;
	fillVertexList.Reserve( paths );

	for ( S32 path = 0; path < paths; path++ )
	{
		stream >> size;

		UI::TVerticies verticies;
		verticies.Reserve( size );

		for ( S32 i = 0; i < size; i++ )
		{
			stream >> pos;
			stream >> normal;

			verticies.PushBack( C_AdobeVertex( pos, normal ) );
		}
		fillVertexList.PushBack( verticies );
	}

	// Read fill indicies
	stream >> paths;

	UI::TIndexList fillIndexList;
	fillIndexList.Reserve( paths );

	for ( S32 path = 0; path < paths; path++ )
	{
		stream >> size;

		UI::TIndicies indicies;
		indicies.Reserve( size );

		for ( S32 i = 0; i < size; i++ )
		{
			stream >> index;

			indicies.PushBack( index );
		}
		fillIndexList.PushBack( indicies );
	}

	// Read verticies for stroke
	stream >> paths;

	TVertexList strokeVertexList;
	strokeVertexList.Reserve( paths );

	for ( S32 path = 0; path < paths; path++ )
	{
		stream >> size;

		TVerticies verticies;
		verticies.Reserve( size );

		for ( S32 i = 0; i < size; i++ )
		{
			stream	>> pos
					>> normal
					>> uv;

			verticies.PushBack( C_AdobeVertex( pos, normal, uv ) );
		}
		strokeVertexList.PushBack( verticies );
	}

	// Build vertex buffers
	if ( fillVertexList.Size() && fillIndexList.Size() )
		CreateFillData( pShape, fillVertexList, fillIndexList );

	if ( strokeVertexList.Size() )
		CreateStrokeData( pShape, strokeVertexList );

	return true;
}


const bool C_IpfReader::CreateFillData( UI::C_AdobeShape* pShape, const UI::TVertexList& vVertexList, const UI::TIndexList& vIndexList )
{
	// vertex buffer
	U32 uVertexCount = 0;
	U32 uIndexCountCorrect = 0;

	for ( S32 i = 0; i < vVertexList.Size(); i++ )
	{
		uVertexCount += vVertexList[ i ].Size();
		uIndexCountCorrect += ( vIndexList[ i ].Size() - 2 ) * 3;
	}

	C_UITriListRenderInfo* pFillData = new C_UITriListRenderInfo();
	assert( pFillData );

	// Create the vertex buffer
	Display::C_VertexBuffer& vertexBuffer = pFillData->m_VertexBuffer;
	if ( Display::Device()->CreateVertexBuffer( sizeof( C_UICharacterOutlineVertex3d ) * uVertexCount, 0, &vertexBuffer ) )
	{
		U32 uCurrentVertex = 0;

		C_UICharacterOutlineVertex3d* pVertex = (C_UICharacterOutlineVertex3d*)vertexBuffer.Lock();
		BIZ_ASSERT(pVertex);

		for ( S32 i = 0; i < vVertexList.Size(); i++ )
		{
			const UI::TVerticies& verticies = vVertexList[i];

			for ( S32 j = 0; j < verticies.Size(); j++ )
			{
				BIZ_ASSERT(uCurrentVertex < uVertexCount);
				pVertex->Set( verticies[j].m_Pos, verticies[j].m_Normal );
				pVertex++;
				uCurrentVertex++;
			}
		}
		vertexBuffer.Unlock();
	}

	// index buffer
	U32 uIndexCount = 0;

	for ( S32 i = 0; i < vIndexList.Size(); i++ )
		uIndexCount += vIndexList[ i ].Size();

	//assert( uIndexCount == uIndexCountCorrect );

	if(uIndexCount == 0)
		return false;

	Display::C_IndexBuffer& indexBuffer = pFillData->m_IndexBuffer;
	if ( Display::Device()->CreateIndexBuffer( sizeof( U16 ) * uIndexCount, 0, Display::E_Format_Index16, &indexBuffer ) )
	{
		U32 uCurrentIndex = 0;

		U16* pIndex = (U16*)indexBuffer.Lock();
		BIZ_ASSERT(pIndex);

		U32 uStartOffset = 0;

		for ( S32 i = 0; i < vVertexList.Size(); i++ )
		{
			for ( S32 j = 0; j < vIndexList[ i ].Size(); j++ )
			{
				BIZ_ASSERT(uCurrentIndex < uIndexCount);
				*pIndex++ = vIndexList[ i ][ j ] + uStartOffset;
				uCurrentIndex++;
			}
			uStartOffset += vVertexList[ i ].Size();
		}

		// add in any extra indices that we've pre-calculated

		for ( S32 i = vVertexList.Size(); i < vIndexList.Size(); i++ )
		{
			for ( S32 j = 0; j < vIndexList[ i ].Size(); j++ )
			{
				BIZ_ASSERT(uCurrentIndex < uIndexCount);
				*pIndex++ = vIndexList[ i ][ j ];
				uCurrentIndex++;
			}
		}

		indexBuffer.Unlock();
	}

	// Add render info
	pFillData->m_uVertexCount = uVertexCount;
	pFillData->m_uIndexCount = uIndexCount;
	pShape->AddFill( pFillData );

	return false;
}


const bool C_IpfReader::CreateStrokeData( UI::C_AdobeShape* pShape, const UI::TVertexList& vVertexList )
{
	for ( S32 path = 0; path < vVertexList.Size(); path++ )
	{
		const TVerticies& verticies = vVertexList[path];

		// Create a new buffer to store the stroke in
		UI::C_UITriListRenderInfo* pOutlineData = new C_UITriListRenderInfo();
		pOutlineData->m_uVertexCount = verticies.Size();

		if ( Display::Device()->CreateVertexBuffer( sizeof( C_UIStrokeVertex3d ) * verticies.Size(), 0, &pOutlineData->m_VertexBuffer ) )
		{
			C_UIStrokeVertex3d* pVertex = (C_UIStrokeVertex3d*)pOutlineData->m_VertexBuffer.Lock();
			BIZ_ASSERT(pVertex);

			for ( S32 j = 0; j < verticies.Size(); j++ )
			{
				pVertex->SetPos( verticies[ j ].m_Pos );
				pVertex->SetUV( verticies[ j ].m_UV );
				pVertex++;
			}
			pOutlineData->m_VertexBuffer.Unlock();
		}
		pShape->AddStroke( pOutlineData );
	}
	return true;
}


}; // namespace Ipf

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

