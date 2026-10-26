#pragma once

#include "Btl/containers/vector.h"
#include "Btl/utility/string.h"
//#include "core/containers.h"
//#include "system/serialise.h"

struct WaveBankList
{
	WaveBankList() {}

	void SetDatName(const char* pchName) { m_strDatName = pchName; }
	bool Add(const Btl::String& strBankFname);
	bool WriteToDisk(const Btl::String& strFname);

private:

	Btl::String m_strDatName;
	Btl::Vector<Btl::String> m_vFilenames;
};