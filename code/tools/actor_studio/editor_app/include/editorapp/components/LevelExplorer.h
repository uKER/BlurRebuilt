#ifndef EDITOR_LEVEL_EXPLORER_H
#define EDITOR_LEVEL_EXPLORER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Components/BaseExplorer.h>

class C_LevelExplorer : public C_BaseExplorer
{
public:
	C_LevelExplorer();
protected:
	virtual bool OnFilterItem(Explorer::ExplorerData* pData);
	virtual void OnContextMenu(Explorer::ExplorerData* item, const WinPoint& pt);
};

#endif