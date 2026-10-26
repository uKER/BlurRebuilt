
#ifndef IPF_IPF_READER_H
#define IPF_IPF_READER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/BinarySerialiseAutoEndian.h>

#include "ui/Illustrator/UIAdobeDefines.h"
#include "ui/Illustrator/UIAdobePolygon.h"

namespace UI
{
class C_AdobeScene;
class C_AdobeGroup;
class C_AdobeShape;
}

namespace Ipf
{
typedef Sys::BinaryDeserialiser<Ut::LittleEndianToNative>	IpfDeserialiser;

class C_IpfReader
{
public:
								/// Read in the contents of a .ipf
								/// @param pFilepath - location of the .ipf file.
								/// @param pScene - buffer to fill out.
	E_Result					ImportScene( Sys::I_FileSystem& fs, const char* pFilepath, UI::C_AdobeScene* pScene );

private:
	const bool					ReadLayer( UI::C_AdobeScene* pScene, IpfDeserialiser& stream );

	const bool					ReadGroup( UI::C_AdobeScene* pScene, IpfDeserialiser& stream );

	const bool					ReadShape( UI::C_AdobeScene* pScene, IpfDeserialiser& stream );

	const bool					CreateFillData( UI::C_AdobeShape* pShape, const UI::TVertexList& vVertexList, const UI::TIndexList& vIndexList );

	const bool					CreateStrokeData( UI::C_AdobeShape* pShape, const UI::TVertexList& vVertexList );

private:
	static U32					m_VersionNumber;	///< combined version number.
};

}; // namespace Ipf

#endif // IPF_IPF_READER_H
