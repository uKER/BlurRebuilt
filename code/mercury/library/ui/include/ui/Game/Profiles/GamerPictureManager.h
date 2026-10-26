
#ifndef UI_GAMER_PICTURE_MANAGER_H
#define UI_GAMER_PICTURE_MANAGER_H

#include "UI/Graphics/Simple2D.h"
#include <SystemProfile/SystemProfile.h>	// [Blur2PC] SysProfile::UserIdentifier
#include "Core/Threads/Thread.h"

#ifdef PLATFORM_PS3
#include <np.h>
#endif

namespace UI
{

#if defined PLATFORM_PS3
	typedef SceNpCommunicationId	TTitleID;
	#define MAX_JOB_SIZE			10
#else
	typedef	S32						TTitleID;
	#define MAX_JOB_SIZE			10
#endif


/*---------------------------------------------------------------------------------------
	Class : C_GamerPicture
---------------------------------------------------------------------------------------*/
class C_GamerPicture : public C_Simple2D::I_AlternateTexture
{
public:
	C_GamerPicture( const Util::C_String32& sName, UITextureHandle pDefaultTexture ) 
		:	m_Ref( 0 ), m_Name( sName ), m_Active( false ), m_Free( true ), m_DefaultTexture( pDefaultTexture )
	{
	}

	~C_GamerPicture()
	{
		Display::Device()->Destroy( &m_Texture );
	}

	const Util::C_String32&				Name()				{ return m_Name; }
	UITextureHandle						Handle()			{ return ( m_Active ) ? &m_Texture : m_DefaultTexture; }
	void								Clear();

	const U16							Ref() const			{ return m_Ref; }
	const SysProfile::UserIdentifier&	UserData() const	{ return m_UserData; }
	const bool							Active() const		{ return m_Active; }
	const bool							Free() const		{ return m_Free; }

private:
	// Ughh im breaking the system 
	friend class I_GamerPictureManager;
	friend class C_GamerPictureManager_Xenon;
	friend class C_GamerPictureManager_Ps3;
	friend class C_GamerPictureManager_Pc;

	U16								m_Ref;				// User reference
	SysProfile::UserIdentifier		m_UserData;			// User data
	bool							m_Active;			// Picture has been downloaded and ready for rendering
	bool							m_Free;				// Requested picture has been assigned this slot

	Util::C_String32				m_Name;				// Texture name
	Display::C_Texture2D			m_Texture;			// Gamerpicture texture
	UITextureHandle					m_DefaultTexture;	// Pointer to default gamerpicture
};

/*---------------------------------------------------------------------------------------
	Class : I_GamerPictureManager

	Desc : Interface manager for retrieving local and remote gamer pictures
---------------------------------------------------------------------------------------*/
class I_GamerPictureManager : public MT::Thread
{
public:
	I_GamerPictureManager()
		: m_Busy( false )
	{}

									// Allocates memory for local and remote gamerpictures. Pass in the default picture to display while requests are pending
	virtual const bool				Create( const S32 sLocalMax, const S32 sRemoteMax, UITextureHandle pDefaultTexture, bool bSmall = false );

									// Deallocates memory
	void 							Destroy();

									// Update call needs calling once per frame
	void							Update();

									// Invalidates all remote gamerpictures
	void							Flush();

									// Invalidates local gamerpicture matching in the given slot
	void							ClearLocalPicture( const S32 sPadIndex );

									// Invalidates remote gamerpicture matching in the given slot
	void							ClearRemotePictureByIndex( const S32 sIndex );

									// Invalidates remote gamerpicture matching the ref
	void							ClearRemotePicture( const U16 uRef );

									// Requests a local gamerpicture for the given pad
	virtual const bool 				GetLocalPicture( const S32 sPadIndex ) = 0;

									// Creates an async job for a remote players gamerpicture if we dont already have it
	void 							RequestRemotePicture( const S32 sPadIndex, const SysProfile::UserIdentifier& sUser, const U16 uRef );

									// Returns the picture pointer for a local picture
	const C_GamerPicture*			LocalSlot( const S32 sIndex )
									{
										return m_LocalGamerPictures[sIndex];
									}

									// Returns the picture pointer for a remote picture
	const C_GamerPicture*			RemoteSlot( const S32 sIndex )
									{
										return m_RemoteGamerPictures[sIndex];
									}

									// Returns the total number of pictures in the given list
	const S32						TotalSize( const bool bRemote = true ) const
									{
										return ( bRemote ) ? m_RemoteGamerPictures.Size() : m_LocalGamerPictures.Size();
									}

									// Returns true if remote pictures are being requested async
	const bool						Busy() const
									{
										return m_Busy;
									}

									// Sets the title id
	void							SetTitleID( const TTitleID tTitleID )
									{
										m_TitleID = tTitleID;
									}

private:
									// Thread Entry point
	void							ThreadMain();

									// Updates the remote picture request job list
	virtual const bool				GetRemotePictures() = 0;

									// return true when there are free slots
	bool							FindFreeSlot( S32 &sSlot );

									// Returns true if remote gamerpicture has already been requested and is still active
	bool							PictureExist( const SysProfile::UserIdentifier& sUser, const U16 uRef );

protected:
	Btl::Vector<C_GamerPicture*>	m_LocalGamerPictures;
	Btl::Vector<C_GamerPicture*>	m_RemoteGamerPictures;
	bool							m_Busy;
	TTitleID						m_TitleID;

	// Job structure for the remote picture request
	struct RemoteRequest
	{
		S32							slot;	// slot into elements array
		SysProfile::UserIdentifier	user;	// data of requested picture
		bool						local;	// for ps3 so we know which container to store it in
	};
	Btl::Vector< RemoteRequest >												m_RemoteRequestList;
	Btl::Vector< RemoteRequest, Btl::VectorParameters_Static< MAX_JOB_SIZE > >	m_RemoteJobList;
};


}; // namespace UI

#endif // UI_GAMER_PICTURE_MANAGER_H
