#ifndef EDITOR_BASE_EXPLORER_H
#define EDITOR_BASE_EXPLORER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xtwin/Explorer.h>

class C_BaseExplorer : public Explorer
{
public:
	C_BaseExplorer();
protected:
	virtual void OnContextMenu(Explorer::ExplorerData* item, const WinPoint& pt);
	void CreateNewPack(ExplorerData* pData);
	void CreateNewFolder(ExplorerData* data);
	void CreateNewRecord(ExplorerData* data, Btl::String type);
	void DuplicateItem(ExplorerData* data);

private:
	Btl::String GetUniqueFolderName(Explorer::ExplorerData* pData, const Btl::String& base);
};

#endif