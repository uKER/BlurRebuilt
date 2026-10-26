#ifndef MARKETPLACEDATAFINDER_H
#define MARKETPLACEDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/UIDataFinder.h"

namespace UI
{

// -----------------------
// C_MarketplaceDataFinder
// -----------------------
class C_MarketplaceDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_MarketplaceDataFinder&	Instance()							{ return m_sInstance; }

private:
	C_MarketplaceDataFinder() : C_UIBaseGameDataFinder( "MARKET_" )		{  }
	C_MarketplaceDataFinder( const C_MarketplaceDataFinder& o )			{  }

public:
	void							Init();
	const WCHAR*					GetVariableText( const char* pVariableName );

private:
	static C_MarketplaceDataFinder	m_sInstance;

private:
	//
};

}; //namespace UI

#endif /* MARKETPLACEDATAFINDER_H */
