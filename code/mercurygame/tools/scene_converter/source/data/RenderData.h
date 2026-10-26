#ifndef APP_RENDER_DATA_H
#define APP_RENDER_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/config/PolyflagConfig.h>

namespace RenderingData
{
	class C_Converter;
}

class C_DataRenderData : public Tools::ProcessGraph::C_Data
{
public:
	C_DataRenderData( const Util::C_String textureCachePath, const Util::C_String fXBPath );
	~C_DataRenderData();

	// inherited from C_Data
	const Util::C_String GetName() const;

	bool Export( Sys::I_File* pFile, Sys::I_File* pStreamFile, const Util::E_Platform platform ) const;

	friend class C_ProcessBuildRenderData;

private:

	Util::C_String m_EffectVariation;
	RenderingData::C_Converter*	m_pConverter;
};

#endif