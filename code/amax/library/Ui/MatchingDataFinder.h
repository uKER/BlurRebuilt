#ifndef _MATCHING_DATAFINDER_H_
#define _MATCHING_DATAFINDER_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>

namespace UI
{

// online sessions are displayed through this.
class C_MatchingDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_MatchingDataFinder&	Instance() { static C_MatchingDataFinder matchingDataFinder; return matchingDataFinder; }

private:
	C_MatchingDataFinder() : C_UIBaseGameDataFinder( "MATCHING_" )	{  }
	C_MatchingDataFinder( const C_MatchingDataFinder& o )			{  }

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	//
};

}; //namespace UI

#endif /* _Matching_DATAFINDER_H_ */
