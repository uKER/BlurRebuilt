#include "stdafx.h"
#include <system/Xml/XmlParser.h>
#include <file_tag_db/GenericAttribute.h>
#include <file_tag_db/Tag.h>
#include <file_tag_db/TagDB.h>
#include <file_tag_db/TagDBManager.h>
#include <system/misc/StringUtil.h>

namespace FileTag
{
	const char* g_pFilename = "file_tag_manager.data";
}

using namespace FileTag;


C_TagDBManager::~C_TagDBManager()
{
	for (S32 i = 0; i < m_databases.Size(); i++)
		delete m_databases[i];
}

C_Tag* C_TagDBManager::AddTag(const char* filePath)
{
	Util::C_String lcase = Util::FormatUnixFileString(filePath).Lower();

	// get path to databse
	Util::C_String dbPath = 
		Util::ExtractDirectory(lcase) + g_pFilename;

	// get database
	C_TagDB* pDB = GetDatabasePtr(dbPath);
	if (!pDB)
	{
		//Create new database.
		pDB = AddDatabase(dbPath);
	}

	return pDB->AddTag(Util::ExtractFilename(lcase));
}

// lookup a file in it's database
C_Tag* C_TagDBManager::GetTagPtr(const char* filePath)
{
	//Look in our cache first...
	Ut::OpenHashMap<Util::C_String, C_Tag*, Ut::HashKeyOpString>::Iterator it = 
		m_tagQueryCache.Find(filePath);
	if (it != m_tagQueryCache.End())
		return it->GetValue();

	//No cache hit, slow search.
	Util::C_String lcase = Util::FormatUnixFileString(filePath).Lower();
	// get path to databse
	Util::C_String dbPath = 
		Util::ExtractDirectory(lcase) + g_pFilename;

	// get database
	C_Tag* pTagResult;
	C_TagDB* pDB = GetDatabasePtr(dbPath);
	if (!pDB)
		pTagResult = 0;
	else 
		pTagResult = pDB->GetTagPtr(filePath);

	//Insert into cache.
	if (pTagResult)
		m_tagQueryCache[filePath] = pTagResult;

	return pTagResult;
}

C_TagDB* C_TagDBManager::AddDatabase(const char* path)
{
	Util::C_String lcase = Util::FormatUnixFileString(path).Lower();

	//Check that we've not got an existing database.
	C_TagDB* pTagDB = GetDatabasePtr(lcase);
	if (pTagDB)
	{
		assert("A database already exists for the given path!");
		return pTagDB;
	}

	C_TagDB* pNewTagDB = new C_TagDB(*this);
	m_databases.PushBack(pNewTagDB);
	m_map[lcase] = m_databases.Size()-1;
	return m_databases.Back();
}

// get the index of a database from it's path
C_TagDB* C_TagDBManager::GetDatabasePtr(const char* path)
{
	Util::C_String lcase = Util::FormatUnixFileString(path).Lower();
	Ut::UtMap<Util::C_String, S32>::Iterator iter;

	// find this database
	iter = m_map.Find( lcase );
	
	S32 dbIndex;
	if (iter != m_map.End())
	{
		dbIndex = iter->GetValue();
		if (dbIndex != -1)
			return m_databases[dbIndex];

		return 0;
	}

	// hasn't been loaded yet, so do it now
	dbIndex = m_databases.Size();
	{
		C_TagDB* pNewTagDB = new C_TagDB(*this);
		assert(pNewTagDB);
		m_databases.PushBack(pNewTagDB);
	}
	m_map[lcase] = dbIndex;
	if (!m_databases[dbIndex]->Read(lcase))
	{
		// couldn't load it, so delete it
		delete m_databases.Back();
		m_databases.PopBack();
		assert(m_map.Find(lcase) != m_map.End());
		m_map.Erase(m_map.Find(lcase));
		//Failed to load database.
		dbIndex = -1;
		return 0;
	}

	return m_databases[dbIndex];
}

bool C_TagDBManager::Write() const
{
	for (Ut::UtMap<Util::C_String, S32>::ConstIterator it = m_map.Begin(); it != m_map.End(); ++it)
		if (it->GetValue() != -1 && !m_databases[it->GetValue()]->Write(it->GetKey()))
			return false;

	return true;
}

void C_TagDBManager::Clear()
{
	for (S32 i = 0; i < m_databases.Size(); i++)
		delete m_databases[i];

	m_databases.Clear();
	m_map.Clear();
}

void C_TagDBManager::DirtyQueryCache() const
{
	m_tagQueryCache.Clear();
}
