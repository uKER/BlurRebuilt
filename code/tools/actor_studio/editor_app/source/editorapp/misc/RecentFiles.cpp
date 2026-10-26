#include "stdafx.h"

#include <wx/msw/registry.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>

#include <EditorApp/Misc/RecentFiles.h>

#include <EditorApp/Misc/Registry.h>

namespace Editor
{

int C_RecentFiles::GetCount()
{
	Btl::String registryCount = GetPrefix() + "Count";
	return Registry::Read(kRegKeyPath, registryCount.Chars(), 0);
}

int C_RecentFiles::GetIndex()
{
	Btl::String registryIndex = GetPrefix() + "Index";
	return Registry::Read(kRegKeyPath, registryIndex.Chars(), 0);
}

Btl::String C_RecentFiles::GetName(int index)
{
	Btl::String registryName;
	registryName.Printf("%s%d_Name", GetPrefix().Chars(), index);
	Btl::String registryValue = Registry::Read(kRegKeyPath, registryName.Chars(), wxString("Empty"));
	return registryValue;
}

Btl::String C_RecentFiles::GetArgs(int index)
{
	Btl::String registryArgs;
	registryArgs.Printf("%s%d_Args", GetPrefix().Chars(), index);
	Btl::String registryValue = Registry::Read(kRegKeyPath, registryArgs.Chars(), wxString(""));
	return registryValue;
}

Btl::String C_RecentFiles::GetPrefix()
{
	Btl::String registryPrefix;
	registryPrefix.Printf("RecentFiles_%x_",Btl::HashKeyOpBtlString::GetHash(
		EditorLib::C_Core::Instance().GetConfig().rootProjectPath));

	return registryPrefix;
}

void C_RecentFiles::MoveToTop(int index)
{
	int fileCount = GetCount();
	int fileIndex = GetIndex();

	index %= fileCount;
	if (index < 0) {
		index += fileCount;
	}

	int diff;
	if (index > fileIndex) {
		diff = fileIndex + fileCount - index; 
	} else {
		diff = fileIndex - index - 1;
	}

	if (diff < 0)
		diff += fileCount;

	Btl::String name = GetName(index);
	Btl::String args = GetArgs(index);

	for (int i=0; i<diff; i++) {
		Btl::String shiftName = GetName((index+i+1)%fileCount);
		Btl::String shiftArgs = GetArgs((index+i+1)%fileCount);
		WriteName((index+i)%fileCount, shiftName);
		WriteArgs((index+i)%fileCount, shiftArgs);
	}

	int newIndex = (fileIndex-1)%fileCount;
	if (newIndex < 0)
		newIndex += fileCount;

	WriteName(newIndex,name);
	WriteArgs(newIndex,args);
}

int C_RecentFiles::FileInList(const Btl::String& args)
{
	int levelCount = GetCount();

	for (int i=0; i < levelCount; i++) {
		Btl::String testValue = GetArgs(i);
		if (testValue == args) {
			return i;
		}
	}

	return -1;
}

void C_RecentFiles::WriteName(int index, const Btl::String& name)
{
	Btl::String registryArgs;
	registryArgs.Printf("%s%d_Name", GetPrefix().Chars(), index);
	Registry::Write(kRegKeyPath, registryArgs.Chars(), wxString(name.Chars()));
}

void C_RecentFiles::WriteArgs(int index, const Btl::String& args)
{
	Btl::String registryArgs;
	registryArgs.Printf("%s%d_Args", GetPrefix().Chars(), index);
	Registry::Write(kRegKeyPath, registryArgs.Chars(), wxString(args.Chars()));
}

void C_RecentFiles::AddRecentFile(const Btl::String& name, const Btl::String& args)
{
	int itemIndex = FileInList(args);
	if (itemIndex == -1) {
		Btl::String registryPrefix;
		registryPrefix.Printf("RecentFiles_%x_",Btl::HashKeyOpBtlString::GetHash(
			EditorLib::C_Core::Instance().GetConfig().rootProjectPath));

		Btl::String registryCount = registryPrefix + "Count";
		Btl::String registryIndex = registryPrefix + "Index";

		int levelCount = GetCount();
		int levelIndex = GetIndex();

		if (levelCount < 6)
		{
			levelCount++;
			Registry::Write(kRegKeyPath, registryCount.Chars(), levelCount);
		}
		
		WriteName(levelIndex, name);
		WriteArgs(levelIndex, args);

		levelIndex++;
		levelIndex %= 6;
		Registry::Write(kRegKeyPath, registryIndex.Chars(), levelIndex);
	} else {
		MoveToTop(itemIndex);
	}
}

}