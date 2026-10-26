#ifndef UILUA_H
#define UILUA_H

#ifdef _MSC_VER
#pragma once
#endif

#define MAX_LUA_SCRIPTS 500

#define USE_LUA_PAK_FILE

#include <compression.h>
#include <lua_shared/LuaHelper.h>

extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

namespace UI
{

#ifndef CFG_GOLD
extern Debug::Bool g_debugUseLUAFromHDD;
#endif

class C_UILua
{
#ifndef CFG_GOLD
public:
	class C_StackCheck
	{
	public:
		C_StackCheck( lua_State* pState ) : m_pState( pState ), m_Top( lua_gettop( pState ) ) { }
		~C_StackCheck();
	private:
		lua_State*	m_pState;
		int			m_Top;
	};
#endif

public:
	C_UILua();
	~C_UILua();
	CCompression	m_zlib;

protected:
	
	typedef struct
	{
		int		screenNameOffset;
		int		dataOffset;
		int		compressedSize;
		int		uncompressedSize;
	} LUA_SCREEN_INFO;

	LUA_SCREEN_INFO*		m_pScreens;			// Ptr to screen data in memory
	char*					m_pStringTable;		// Base offset to strings in memory

public:
	void					Init(bool full_init, const char *pHddScriptsDir, void* ptrToLuaScripts );

	void					DoConsole();
	Util::C_String			m_ConsoleString;						

	void					End( );

	lua_State *				GetState( )					{ return m_LuaState; }

	void					StackDump( lua_State *L, char *message );
	void					DumpCallStack( lua_State *L );

	void					Message( const U32 msg )		{ mMessage = msg; }
	const U32				Message( )						{ return mMessage; }

	const int				LoadCallFile(const char *fname, bool forceFromHDD=false);
	void					LoadCallSetupScript( const U32 initParam, const bool bForceOnlyHDD = false );
					
	const F32				GetNumberField(const char *key);
	const F32				GetNumberField(int key);
	const F64				GetDoubleNumberField(int key);
	const F64				GetDoubleNumberField(const char *key);

	const F32				GetNumberFieldFromTable(const char *table,int key);
	const F32				GetNumberFieldFromTable(const char *table,const char *key);
	const F32				GetNumberFieldFromTable(int table,int key);
	const F32				GetNumberFieldFromTable(int table,const char *key);

	const bool				GetBooleanFieldFromTable(const char *table,int key);
	const bool				GetBooleanFieldFromTable(const char *table,const char *key);
	const bool				GetBooleanFieldFromTable(int table,int key);
	const bool				GetBooleanFieldFromTable(int table,const char *key);

	const F64				GetDoubleNumberFieldFromTable(const char *table,int key);
	const F64				GetDoubleNumberFieldFromTable(const char *table,const char *key);
	const F64				GetDoubleNumberFieldFromTable(int table,int key);
	const F64				GetDoubleNumberFieldFromTable(int table,const char *key);

	const char *			GetStringFieldFromTable(const char *table,int key);
	const char *			GetStringFieldFromTable(const char *table,const char *key);
	const char *			GetStringFieldFromTable(int table,int key);
	const char *			GetStringFieldFromTable(int table,const char *key);

	const int				GetFieldTypeFromTable(const char *table,const char *key);

	const char *			GetStringField(const char *key);
	const char *			GetStringField(int key);

	const int				GetFieldType(const char *key);

	const bool				GetBooleanField(const char *key);
	const bool				GetBooleanField(int key);

	const S32				GetValFromTable(const char *table, const char *name);
	const S32				GetStringIDSFromName(const char *name);
	const S32				GetFontFromName(const char *name);

	// Thread related antics
	__forceinline void		EnterCriticalSection( )			{ m_Lock.Enter(); }
	__forceinline void		ExitCriticalSection( )			{ m_Lock.Exit(); }

	void SetCurrentPAK(const char *name);
	int GetCurrentPAKNumber() const
	{
		return mCurrentPAK;
	}
	int GetCurrentPAKOffset() const
	{
		assert(m_pScreens != NULL);
		assert((mLuaScriptNumber >= 0) && (mCurrentPAK < mLuaScriptNumber));
		return m_pScreens[mCurrentPAK].dataOffset;
	}
	int GetCurrentPAKSize() const
	{
		assert(m_pScreens != NULL);
		assert((mLuaScriptNumber >= 0) && (mCurrentPAK < mLuaScriptNumber));
		
		return m_pScreens[mCurrentPAK].uncompressedSize;
	}
	BYTE *GetPAKScripts()
	{
		return (U8*)m_pStringTable;
	}

//	void					SetCurrentPAK(char *name);
//	int						GetCurrentPAKNumber(){return mCurrentPAK;};
//	__int64					GetCurrentPAKOffset(){return mLuaScriptOffset[mCurrentPAK];};
//	__int64					GetCurrentPAKSize(){return mLuaScriptSizes[mCurrentPAK];};
//	BYTE *					GetPAKScripts(){return mLuaScripts;};

#ifndef CFG_GOLD
	C_StackCheck			CreateStackCheck()		{ return C_StackCheck( m_LuaState ); }
	void					ReceiveConsoleString(const char *message);
#endif

	bool					mDoingFullInit;
	U32						mMessage;

private:
	Util::C_String			m_HDDScriptsDir;
	U8*						m_pUIData; //only needed for PC versions, where data is loaded externally

	BYTE *					mLuaScripts;
	char *					mLuaScriptNames[MAX_LUA_SCRIPTS];
	__int64					mLuaScriptOffset[MAX_LUA_SCRIPTS];
	__int64					mLuaScriptSizes[MAX_LUA_SCRIPTS];	// [Rog] Are you really going to write a script over 2Gb in size?
	int						mLuaScriptNumber;
						
	int						mCurrentPAK;
	lua_State*				m_LuaState;
	MT::Lock				m_Lock;

};

extern C_UILua*		g_pUILua;

}; // namespace UI

#endif // UILUA_H
