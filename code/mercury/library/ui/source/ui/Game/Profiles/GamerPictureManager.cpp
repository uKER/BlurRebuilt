
#include "stdafx.h"
#include "UI/Game/Profiles/GamerPictureManager.h"

#include <core/threads/Atomic.h>
#include <systemprofile/SystemProfile.h>

#include "UI/Game/Profiles/ProfileInterface.h"

namespace UI
{

void C_GamerPicture::Clear()
{
	m_Ref = 0;
	m_UserData = SysProfile::UserIdentifier();
	m_Active = false;
	m_Free = true;
}


const bool I_GamerPictureManager::Create( const S32 sLocalMax, const S32 sRemoteMax, UITextureHandle pDefaultTexture, bool bSmall )
{
	Util::C_String32 name;

	U32 size = ( bSmall ) ? 32 : 64;

	// Allocate the local textures
	for ( S32 i = 0; i < sLocalMax; i++ )
	{
		name.Printf( "GAMERPIC_%i", i );
		C_GamerPicture* pGamerPic = new C_GamerPicture( name, pDefaultTexture );

		if ( Display::Device()->CreateTexture2D( size, size, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &pGamerPic->m_Texture ) )
		{
			m_LocalGamerPictures.PushBack( pGamerPic );
			gSimple2D.SetAlternateTexture( pGamerPic );
		}
		else
		{
			LOGERROR( g_Log, "Failed to create remote gamerpicture texture" ); 
			SAFE_DELETE_PTR( pGamerPic );
		}
	}

	// Allocate the remote textures
	for ( S32 i = 0; i < sRemoteMax; i++ )
	{
		name.Printf( "REMOTE_GAMERPIC_%i", i );
		C_GamerPicture* pGamerPic = new C_GamerPicture( name, pDefaultTexture );

		if ( Display::Device()->CreateTexture2D( size, size, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &pGamerPic->m_Texture ) )
		{
			m_RemoteGamerPictures.PushBack( pGamerPic );
			gSimple2D.SetAlternateTexture( pGamerPic );
		}
		else
		{
			LOGERROR( g_Log, "Failed to create remote gamerpicture texture" ); 
			SAFE_DELETE_PTR( pGamerPic );
		}
	}
	return true;
}

void I_GamerPictureManager::Destroy()
{
	for ( S32 i = 0; i < m_LocalGamerPictures.Size(); i++ )
		SAFE_DELETE_PTR( m_LocalGamerPictures[i] );

	m_LocalGamerPictures.Clear();

	for ( S32 i = 0; i < m_RemoteGamerPictures.Size(); i++ )
		SAFE_DELETE_PTR( m_RemoteGamerPictures[i] );

	m_RemoteGamerPictures.Clear();
}

void I_GamerPictureManager::Flush()
{
	for ( S32 i = 0; i < m_RemoteGamerPictures.Size(); i++ )
	{
		m_RemoteGamerPictures[i]->Clear();
	}
}

void I_GamerPictureManager::ClearRemotePicture( const U16 uRef )
{
	for ( S32 i = 0; i < m_RemoteGamerPictures.Size(); i++ )
	{
		if ( m_RemoteGamerPictures[i]->Ref() == uRef )
		{
			m_RemoteGamerPictures[i]->Clear();
			break;
		}
	}
}

void I_GamerPictureManager::ClearLocalPicture( const S32 sPadIndex )
{
	if ( sPadIndex < m_LocalGamerPictures.Size() )
		m_LocalGamerPictures[sPadIndex]->Clear();
}

void I_GamerPictureManager::ClearRemotePictureByIndex( const S32 sIndex )
{
	if ( sIndex < m_RemoteGamerPictures.Size() )
		m_RemoteGamerPictures[sIndex]->Clear();
}


bool I_GamerPictureManager::FindFreeSlot( S32 &sSlot )
{
	for ( S32 i = 0; i < m_RemoteGamerPictures.Size(); i++ )
	{
		if ( m_RemoteGamerPictures[i]->Free() )
		{
			sSlot = i;
			return true;
		}
	}
	return false;
}


bool I_GamerPictureManager::PictureExist( const SysProfile::UserIdentifier& sUser, const U16 uRef )
{
	for ( S32 i = 0; i < m_RemoteGamerPictures.Size(); i++ )
	{
		// Dont check empty pictures
		if ( m_RemoteGamerPictures[i]->Free() )
			continue;

		if ( m_RemoteGamerPictures[i]->UserData() == sUser )
		{
			m_RemoteGamerPictures[i]->m_Ref = uRef; // override it
			return true;
		}
	}
	return false;
}

void I_GamerPictureManager::RequestRemotePicture( const S32 sPadIndex, const SysProfile::UserIdentifier& sUser, const U16 uRef )
{
// Dont have any pictures on PC, unless its live for windows which im not supporting yet
#ifdef PLATFORM_PC
	return;
#endif // PLATFORM_PC

	// The player must be online in order to request remote gamer pictures
	SysProfile::FullProfile& profile = C_ProfileInterface::Instance().GetPadProfile( sPadIndex );
	if ( profile.m_State != SysProfile::eProfileStateOnline )
		return;

	// No point requesting the little beastie if we already have it
	if ( PictureExist( sUser, uRef ) )
		return;

	S32 slot = 0;
	if ( !FindFreeSlot( slot ) )
	{
		LOGWARN( g_Log, "No free gamerpicture slots" ); 
		return;
	}

	// Make a new request
	RemoteRequest request =
	{
		slot,
		sUser,
		false,
	};

	// so we dont try to fill this slot more than once
	m_RemoteGamerPictures[slot]->m_Free		= false; 
	m_RemoteGamerPictures[slot]->m_Ref		= uRef;
	m_RemoteGamerPictures[slot]->m_UserData	= sUser;

	// Push onto the job list so the worker thread can start grabbing them once its ready
	m_RemoteRequestList.PushBack( request );
}

void I_GamerPictureManager::Update()
{
	if ( !m_Busy )
	{
#ifdef PLATFORM_PS3
		// Need to join the thread once its finished running to kill it
		if ( !I_GamerPictureManager::IsRunning() )
			I_GamerPictureManager::Join();
#endif // PLATFORM_PS3

		// We have pictures to request
		if ( m_RemoteRequestList.Size() )
		{
			S32 max = ( m_RemoteRequestList.Size() > MAX_JOB_SIZE ) ? MAX_JOB_SIZE : m_RemoteRequestList.Size() ;
			Btl::Vector< RemoteRequest >::Iterator iter( m_RemoteRequestList.Begin() );

			for ( S32 i = 0; i < max; i++ )
			{
				m_RemoteJobList.PushBack( (*iter) );
				iter = m_RemoteRequestList.Erase( iter );
			}
			
			m_Busy = true;
			Start( 64 * 1024, 0, "UI::I_GamerPictureManager::GetRemotePlayers" ); // default stack size and normal priority
		}
	}
}


void I_GamerPictureManager::ThreadMain()
{
	GetRemotePictures();

	MT::MemoryFence();
	m_Busy = false;

#ifndef PLATFORM_PS3
	Close();
#endif // PLATFORM_PS3
}

}; // namespace UI