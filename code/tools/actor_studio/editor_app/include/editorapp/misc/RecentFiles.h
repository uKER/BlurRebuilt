#ifndef EDITOR_RECENT_FILES_H
#define EDITOR_RECENT_FILES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_RecentFiles
	{
	public:
		static int GetCount();
		static int GetIndex();
		static Btl::String GetName(int index);
		static Btl::String GetArgs(int index);
		static void AddRecentFile(const Btl::String& name, const Btl::String& args);
		static void MoveToTop(int index);

	private:
		static Btl::String GetPrefix();
		static int FileInList(const Btl::String& args);
		static void WriteName(int index, const Btl::String& name);
		static void WriteArgs(int index, const Btl::String& args);
	};
}

#endif