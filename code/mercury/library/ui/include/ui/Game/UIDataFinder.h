#ifndef UIDATAFINDER_H
#define UIDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"
#include <ui/Util/WideString.h>

#include <btl/Containers/containers.h>

namespace UI
{

/// @addtogroup DataFinder
/// @{

// ----------------------
// C_UIBaseGameDataFinder
// ----------------------
/// base class for a game data finder
class C_UIBaseGameDataFinder
{
public:	
	C_UIBaseGameDataFinder()							{ m_pHeader[ 0 ] = '\0'; }
	C_UIBaseGameDataFinder( const char* pHeader )		{ STRCPY( m_pHeader, m_uHeaderSize, pHeader ); }
	virtual ~C_UIBaseGameDataFinder()					{  }

public:
	/// initialise - called when you add it to the C_UIDataFinder list
	virtual void			Init()						{  }

	/// return the string - pVariableName will be the string minus the header
	virtual const WCHAR*	GetVariableText( const char* pVariableName ) = 0;

	/// header for this plugin part - e.g. "NETWORK_"
	const char*				GetHeader()					{ return m_pHeader; }

	/// parse index helper function
	/// @remarks - returns @e -1 if number is not found
	S64						ParseIndex( const char* pVariableName, const char* pHeader, const bool bFindFirstNumber = false );

	S64						ParseIndexAdvance( const char*& pVariableName, const char* pHeader, const bool bFindFirstNumber = false );

protected:
	static const U32		m_uHeaderSize = 32;
	char					m_pHeader[ m_uHeaderSize ];

	C_WideString512			m_VariableText;
};

// --------------
// C_UIDataFinder
// --------------
/// holder class for all of the derived I_UIGameDataFinder - this is just a pointer storage class - nothing is allocated/deleted
class C_UIDataFinder
{
public:
	static C_UIDataFinder&	Instance()			{ return m_sInstance; }

private:
	C_UIDataFinder()							{  }
	C_UIDataFinder( const C_UIDataFinder& o	)	{  }

public:
	/// add in a new game data finder
	void			AddGameDataFinder( C_UIBaseGameDataFinder* pGameDataFinder );

	/// cleanup the list
	void			Cleanup()											{ m_GameDataFinders.Clear(); }

	/// get the string
	/// @remarks - get from the StringStore is built in
	const WCHAR*	GetVariableText( const char* pVariableName );

private:
	static	C_UIDataFinder	m_sInstance;

private:
	static const U32																				m_MaxGameDataFinders = 12;
	Btl::Vector< C_UIBaseGameDataFinder*, Btl::VectorParameters_Static< m_MaxGameDataFinders > >	m_GameDataFinders;
};

/// @}

}; // namespace UI

#endif // UIDATAFINDER_H
