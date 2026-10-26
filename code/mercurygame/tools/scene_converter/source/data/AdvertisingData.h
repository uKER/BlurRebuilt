#ifndef APP_ADVERTISING_DATA_H
#define APP_ADVERTISING_DATA_H


#ifdef _MSC_VER
#pragma once
#endif

#include <Advertising/AdvertResource.h>

class C_DataAdvertising : public Tools::ProcessGraph::C_Data
{
public:

	C_DataAdvertising();
	~C_DataAdvertising();

	// inherited from C_Data
	const Util::C_String GetName() const;

	void AddAdvertQuad(const Advertising::AdvertResource::AdvertQuad& quad);

	const Advertising::AdvertResource& GetAdvertResource() const;

	void SortAdvertQuads();

	bool Export( Sys::I_File* pFile) const;

private:

	Advertising::AdvertResource m_AdvertResource;
};



#endif