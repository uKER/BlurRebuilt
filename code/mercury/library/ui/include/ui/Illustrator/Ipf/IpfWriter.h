
#ifndef IPF_IPF_WRITER_H
#define IPF_IPF_WRITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/BinarySerialiseAutoEndian.h>

#include "ui/Illustrator/UIAdobeDefines.h"

namespace UI
{
class C_AdobeGroup;
class C_AdobePolygon;
}

namespace Ipf
{
typedef Sys::BinarySerialiser<Ut::NoEndianSwap> IpfSerialiser;

class C_IpfData;

class C_IpfWriter
{
public:
	E_Result						ExportScene( const char* pFilepath, const C_IpfData* pData );

									/// Returns the current version of the file reader.
	static const U32				GetFileVersion()
									{ 
										return m_VersionNumber; 
									}

	static const Util::C_String		GetFileVersionString()
									{
										char buff[64];
										sprintf_s( buff, 64, "%d.%d", IPF_VersionMajor, IPF_VersionMinor );

										return buff;
									}

private:
	const bool						WriteLayer( const UI::C_AdobeGroup* pGroup, IpfSerialiser& stream );

	const bool						WriteGroup( const UI::C_AdobeGroup* pGroup, IpfSerialiser& stream );

	const bool						WriteShape( const UI::C_AdobePolygon* pShape, IpfSerialiser& stream );

private:
	static U32						m_VersionNumber;	///< combined version number.
};

}; // namespace Ipf

#endif // IPF_IPF_WRITER_H