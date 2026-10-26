#ifndef CONVERTER_SHARED_ABFOLDERLIST_CONFIG_H
#define CONVERTER_SHARED_ABFOLDERLIST_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace ABFolderList
{
	class C_Config
	{
	public:
		static void ReadABFolders(const Sys::C_XmlDom& dom);
		static void AddToFolderList(const Util::C_String& folder);
		static bool IsFileInABFolderList(const Util::C_String& file);
	private:
		static Ut::UtDynamicVector<Util::C_String> m_abFolderArray;
	};

} // namespace ABFolderList

#endif
