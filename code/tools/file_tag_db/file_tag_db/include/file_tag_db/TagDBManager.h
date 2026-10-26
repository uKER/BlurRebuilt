#ifndef FILE_TAG_DB_MANAGER_H
#define FILE_TAG_DB_MANAGER_H

namespace FileTag
{
	// singleton for keeping track of file info databases
	class C_TagDBManager : public Ut::Singleton<C_TagDBManager, Ut::CreateUsingNew>
	{
	public:
		virtual ~C_TagDBManager();

		C_Tag* AddTag(const char* filePath);

		C_Tag* GetTagPtr(const char* filePath);

		C_TagDB* AddDatabase(const char* path);

		C_TagDB* GetDatabasePtr(const char* path);

		void Clear();

		//Write all active databases to disk.
		bool Write() const;

		void DirtyQueryCache() const;

	private:
		// list of loaded .xml files
		Ut::UtVector<Ut::DynamicVector<C_TagDB*> >	m_databases;
		//Mapping of filename -> database.
		Ut::UtMap<Util::C_String, S32>				m_map;

		mutable Ut::OpenHashMap<Util::C_String, C_Tag*, Ut::HashKeyOpString>
													m_tagQueryCache;
	};

}

#endif
