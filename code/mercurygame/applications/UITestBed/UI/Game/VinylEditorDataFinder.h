#ifndef VINYLEDITORDATAFINDER_H
#define VINYLEDITORDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Util/WideString.h>

namespace UI
{

class C_VinylEditorDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_VinylEditorDataFinder&	Instance()						{ static C_VinylEditorDataFinder gameDataFinder; return gameDataFinder; }

private:
	C_VinylEditorDataFinder() : C_UIBaseGameDataFinder( "VNL_" )		{  }
	C_VinylEditorDataFinder( const C_UIDataFinder& o )					{  }

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	//
};

}; //namespace UI

#endif /* VINYLEDITORDATAFINDER_H */
