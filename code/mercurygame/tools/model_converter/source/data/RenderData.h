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

	bool Export( Sys::I_File* pFile, const Util::E_Platform platform ) const;

	friend class C_ProcessBuildRenderData;

private:

	RenderingData::C_Converter*	m_pConverter;
};

#endif