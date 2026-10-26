#ifndef PROFILEDATAFINDER_H
#define PROFILEDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>

namespace UI
{

// -------------------
// C_ProfileDataFinder
// -------------------
class C_ProfileDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_ProfileDataFinder&	Instance()							{ return m_sInstance; }

private:
	C_ProfileDataFinder() : C_UIBaseGameDataFinder( "PROFILE_" )	{  }
	C_ProfileDataFinder( const C_ProfileDataFinder& o )				{  }

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	static C_ProfileDataFinder	m_sInstance;

private:
	//
};

}; //namespace UI

#endif /* PROFILEDATAFINDER_H */
