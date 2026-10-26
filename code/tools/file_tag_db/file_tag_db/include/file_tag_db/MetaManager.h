#ifndef FILE_TAG_METAMANAGER_H
#define FILE_TAG_METAMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace FileTag
{
	class C_MetaManager : public Ut::Singleton<C_MetaManager, Ut::CreateUsingNew>
	{
	public:
		virtual ~C_MetaManager();

		bool AddTag(C_MetaTag& metaTag, const Util::C_String& filePath);

		bool GetTag(C_MetaTag& metaTag, const Util::C_String& filePath);

		// Saves or creates a new xml file.
		bool Write();

		// Clears all open databases
		void Clear();
	};

}// namespace FileTag

#endif // FILE_TAG_METAMANAGER_H
