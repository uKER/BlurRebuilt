#ifndef _LAN_DATAFINDER_H_
#define _LAN_DATAFINDER_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>

namespace UI
{

class C_LanDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_LanDataFinder&	Instance() { static C_LanDataFinder lobbyDataFinder; return lobbyDataFinder; }

private:
	C_LanDataFinder() : C_UIBaseGameDataFinder( "LAN_" )		{  }
	C_LanDataFinder( const C_LanDataFinder& o )					{  }

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	//
};

}; //namespace UI

#endif /* _LAN_DATAFINDER_H_ */
