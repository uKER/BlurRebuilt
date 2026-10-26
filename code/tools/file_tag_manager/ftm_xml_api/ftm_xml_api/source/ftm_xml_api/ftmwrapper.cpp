#include "stdafx.h"

#include <ftm_xml_api/Headers.h>
#include <tools_common/StringProc.h>

#define TEX_DATA_XML_FILE_NAME "TextureData.xml"

namespace FTM
{

// get the index of a database from it's path
int FTMWrapper::GetDatabase(const std::string &path)
{
	std::map<std::string, int>::iterator iter;

	// find this database
	iter = m_map.find( path );
	if (iter != m_map.end())
		return (*iter).second;

	// hasn't been loaded yet, so do it now
	int iIdx = m_entries.Size();
	m_entries.PushBack();
	Tu::Status status = m_entries[iIdx].m_db.Read(path.c_str());
	if (!status)
	{
		// couldn't load it, so delete it
		m_entries.PopBack();
		iIdx = -1;
	}

	return m_map[ path ] = iIdx;
}

// lookup a texture in it's database
const FTMTexInfo& FTMWrapper::LookupTexture(const std::string &texturePath)
{
	// get path to databse
	std::string dir(Tu::DirFromString(texturePath));
	std::string xmlPath = dir + std::string("/") + TEX_DATA_XML_FILE_NAME;
	// get database
	int i = GetDatabase( xmlPath );
	if (i == -1)
		return m_DummyRecord;	// not found

	// get entry from database
	S32 recordIndex = m_entries[i].m_db.RecordIndex(Tu::FileNameFromString(texturePath).c_str());
	if (recordIndex == -1)
		return m_DummyRecord;

	const FTM::TexDatabase::DBRecord& record = m_entries[i].m_db.Record(recordIndex);
	return *(FTMTexInfo*)&record;
}

}
