#ifndef UI_SP_DATA_FINDER_H
#define UI_SP_DATA_FINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>

namespace UI
{

class C_SpDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_SpDataFinder&	Instance() 
	{ 
		return m_sInstance; 
	}

private:
	C_SpDataFinder() : C_UIBaseGameDataFinder( "SPL_" )		{}
	C_SpDataFinder( const C_SpDataFinder& o )				{}

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	static C_SpDataFinder	m_sInstance; 
};

}; //namespace UI

#endif // UI_SP_DATA_FINDER_H
