
#ifndef UI_UIADOBESCENEMANAGER_H
#define UI_UIADOBESCENEMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Illustrator/UIAdobeHandle.h"
#include "ui/Game/UILua.h"

namespace UI
{

///@addtogroup Illustrator
///@{

/*--------------------------------------------------------------------------------------
	Forward declaration
--------------------------------------------------------------------------------------*/
class C_AdobeGradient;
class C_AdobePath;


/*--------------------------------------------------------------------------------------
	Class : C_AdobeSceneManager

	Desc :	Manager class for storing and loading adobe data.
			Is also an interface for getting data.
--------------------------------------------------------------------------------------*/
/// Manager class to handle the creation and removal of scene as well as the interface between the user and AdobeScenes.
class C_AdobeSceneManager
{
public:
	/// @a
	C_AdobeSceneManager();

	/// @a
	~C_AdobeSceneManager();

										/// @a
	static C_AdobeSceneManager&			Instance(){ return m_sInstance; }

										/// Push all of the available scene names into the lua enums.
	bool								CreateLuaEnums( lua_State *pState );

										/// Load a single scene at from the given file.
	bool								LoadScene( const char *filepath );

										/// Load all of the scenes in the given directory.
	bool								LoadScenes( const char* directory );

	void								ReloadScene( const S32 sIndex );

	void								ReloadScenes();

	void								RemoveScene( const char* pName );

										/// Erase all of the Scenes currently loaded into memory!!! 
	void								FlushData();

										/// @a
	void								Update();

										/// @a
	const C_AdobeHandle*				GetHandle( const S32 sIndex ) const;

										/// @a
	const C_AdobeHandle*				GetHandleByName( const char* pName ) const;

										/// @a
	const S32							GetHandleIndexByName( const char* pName ) const;

										/// @a
	const S32							GetNumberOfScenes() const 
										{ 
											return m_Handles.Size(); 
										}

										/// @a
	const U32							GetNumberOfGradients() const 
										{
											return m_NumberOfGradients; 
										}

private:
	/*--------------------------------------------------------------------------------------
		Limits for adobe elements.
	--------------------------------------------------------------------------------------*/
	static const U32					m_MaxNumberOfGradients	= 64;				///< @a

	/*--------------------------------------------------------------------------------------
		Data members
	--------------------------------------------------------------------------------------*/
	static C_AdobeSceneManager			m_sInstance;								///< @a
	U32									m_NumberOfGradients;						///< @a
				
	Btl::Vector< C_AdobeHandle >		m_Handles;									///< @a
	MT::Lock							m_CriticalSecion;

	//UIGradientHandle					m_GradientHandles[m_MaxNumberOfGradients];	///< @a
};

///@}

} // namespace UI

#endif // UI_UIADOBESCENEMANAGER_H
