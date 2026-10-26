#ifndef LOC_DATA_H
#define LOC_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>

class C_LocData
{
public:
	C_LocData()
		: m_bInitialised( false )
	{}

	void				Init();
	void				Update();
	bool				ReplaceLoc( const char *locName, const U8* pMemory, const U32 size );
	void				Delete();

	static C_LocData&	Instance() 
						{
							return m_sInstance;
						}

	static void			SorceryCallback( XtEventType type, XtRecord* pRecord, C_LocData* pLoc, XtCallbackContext* );

private:
	static void			Instance( const C_LocData& loc )		
						{ 
							m_sInstance = loc; 
						}

public:	// exposed for Xt
	XtArray< XtSz8 >	m_UITableNames;
	XtArray< XtSz8 >	m_GameTableNames;

private:
	static C_LocData	m_sInstance;

	bool				m_bInitialised;
};

#endif // LOC_DATA_H
