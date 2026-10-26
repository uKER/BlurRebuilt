#include <GameApp.h>

#include <GameLogic/Profile/GameProfileManager.h>

#include "AppPhotoCapture.h"
#include <jpeghelper/JpegDecoder.h> 
#include <jpeghelper/JpegEncoder.h>

#include <ui/Game/UILocaliser.h>

#include <Game/Game.h>
#include <GameApp.h>
#include <GameLogic/Structures/LevelState.h>
#include <ui/UiHelpers.h>

#include <GameLogic\ContentServer\ContentServerManager.h>

#include <GameLogic\Photomode\PhotomodeManager.h>

#include <compression.h>

#include <Game\GameUtil\ScopedSuspendResume.h>
#include "Structures\VehiclePickupInfo.h"
#include "Game\GameActors\VehicleLogic\VehicleLogicActor.h"

namespace UI
{

// --------------------------
// C_SaveInfoInterfaceProfile
// --------------------------
class C_SaveInfoInterfaceProfile : public C_SaveInfoInterface
{
public:
	C_SaveInfoInterfaceProfile()
		: C_SaveInfoInterface()
	{
	#if PLATFORM_PC
		m_EnumerationFileExtension = "save";
	#endif
	#ifdef PLATFORM_PS3
		m_TitleId = PlatformPS3::g_ps3_titleId;
		m_FilePath = "MemSavePS3";
		m_uSizeIcon0 = 68327;
		m_uSizeIcon1 = 0;
		m_uSizePic1 = 546985;
		m_uSizeSnd0 = 0;

		SetParamSFO_Title( UI::C_TextStringID( "PRO_PS3_SAVE_TITLE" ).String() );
		SetParamSFO_Detail( UI::C_TextStringID( "PRO_PS3_SAVE_DETAIL" ).String() );
		SetSubtitle( UI::C_TextStringID( "PRO_PS3_SAVE_SUBTITLE" ).String() );

		m_ListName = "TEST";								// debug - not used here
		SetListNewDataTitle( L"SHOULD NEVER SEE THIS!" );	// debug - not used here

		SetSecureFieldId( "BEAUTIFUL_CHAOS!" );
	#endif
	#ifdef PLATFORM_PC
		m_SaveRoot = "SAVE";
	#endif
	}

public:
	void	Setup( const bool bLoading )
			{
				C_ScopedDeviceSuspend scopedDeviceSuspend;

				m_FileInfo.Clear();

				U32 uFileSize = 0;

				if( !bLoading )
				{
					m_pStream = new Sys::C_MemoryStream( 4 * 1024 );
					if( !m_pStream )
					{
						LOGERROR( g_Log, DBSTR "Failed to allocate");
						return;
					}

					Sys::BinarySerialiserAutoEndian serial( *m_pStream );
					
					// Implement SteveB's debug save for now
					bool result = Game::SaveGameImpl( serial );
					if( !result )
					{
						LOGERROR( g_Log, DBSTR "Failed to save");
						return;
					}
					

					CCompression compress(false);
					U8* pCompressedPtr;
					U32 uCompressedSize;
					U32 uMagicNumber = 0xABCDE337;
					U32 uOriginalSize = m_pStream->GetBufferSize();
					U8* pSourceData = m_pStream->GetBufferPtr();

					// We finished writing okay.
					pSourceData[0] = 0;

					U32 pSourceDataSize = m_pStream->GetBufferSize();
					compress.Compress(pSourceData, pSourceDataSize, &pCompressedPtr, &uCompressedSize );
					
					delete m_pStream;
									
					m_pStream = new Sys::C_MemoryStream( 4 * 1024 );
					Sys::BinarySerialiserAutoEndian serialcompressed( *m_pStream );
					serialcompressed << uMagicNumber;
					serialcompressed << uOriginalSize;
					serialcompressed << uCompressedSize;
					for (U32 x = 0 ; x < uCompressedSize ; x++) 
					{
						char c = pCompressedPtr[x];
						serialcompressed << c;
					}
					
					delete pCompressedPtr;

					// PR - this is hardcoded so we can do a PS3 TRC
//					uFileSize = ( 5 * 1024 * 1024 );

					uFileSize = (U32)m_pStream->GetBufferSize();
				}

				m_FileInfo.PushBack( 
					C_SaveFileInfo(
						  uFileSize												// m_uFileSize
					#ifdef PLATFORM_PC
						, "BLURSAVE"											// m_FileName ([Blur2PC] T250: retail 0x00c20180)
					#else
						, "AMAXSAVE"											// m_FileName
					#endif
					)
				);
			}

	bool	Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
			{
				// Implement SteveB's debug save for now
				U8* pData = pBuffer;
				U32 uMagicNumber		= 0xABCDE337;
			#if PLATFORM_PC
				// [Blur2PC] Save() writes the header through the native (little-endian) serialiser; the
				// big-endian read below never matched on PC, so every save was rejected on load.
				U32 uMagicNumberRead	= pData[3] << 24 | pData[2] << 16 | pData[1] << 8 | pData[0];
				U32 uOriginalSize		= pData[7] << 24 | pData[6] << 16 | pData[5] << 8 | pData[4];
				U32 uCompressedSize		= pData[11] << 24 | pData[10] << 16 | pData[9] << 8 | pData[8];
			#else
				U32 uMagicNumberRead	= pData[0] << 24 | pData[1] << 16 | pData[2] << 8 | pData[3];
				U32 uOriginalSize		= pData[4] << 24 | pData[5] << 16 | pData[6] << 8 | pData[7];
				U32 uCompressedSize		= pData[8] << 24 | pData[9] << 16 | pData[10] << 8 | pData[11];
			#endif
				bool result = false;
				if (uMagicNumberRead == uMagicNumber)
				{
					pData+=sizeof(uOriginalSize)+sizeof(uCompressedSize)+sizeof(uMagicNumberRead);
					CCompression decompress;
					U8* pDecompressed = new U8[ uOriginalSize ];
					decompress.Decompress(pData, uCompressedSize, pDecompressed, uOriginalSize);
					Sys::C_MemoryStream stream( pDecompressed, uOriginalSize );
					Sys::BinaryDeserialiserAutoEndian serial( stream );
					result = Game::LoadGameImpl( serial );
				#if PLATFORM_PC
					// [Blur2PC] T250: bytes the load left unread (0 when the format matched)
					U32 uUnread = 0;
					U8 uExtra;
					while ( result && serial.Read( &uExtra, 1 ) )
						uUnread++;
					LOGINFO( g_Log, "[Blur2PC] profile save load %s (version %d, %u bytes, %u unread)\n", result ? "succeeded" : "failed",
						GetLoadingSaveGameVersion(), uOriginalSize, uUnread );
				#endif
					delete pDecompressed;
				}
				else
				{
					// Uncompressed 
					Sys::C_MemoryStream stream( pBuffer, uDataSize );
					Sys::BinaryDeserialiserAutoEndian serial( stream );
					result = Game::LoadGameImpl( serial );
					
				}
				
				return result;
			}

	bool	Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
			{
				if( !m_pStream )
					return false;

				memcpy( pBuffer, m_pStream->GetBufferPtr(), m_pStream->GetBufferSize() );

				SAFE_DELETE_PTR( m_pStream );
				return true;
			}

	Sys::C_MemoryStream*	m_pStream;
};

// -------------------------
// C_SaveInfoInterfacePhotos
// -------------------------
class C_SaveInfoInterfacePhotos : public C_SaveInfoInterface
{
public:
	C_SaveInfoInterfacePhotos()
		: C_SaveInfoInterface()
	{
		m_eType = E_UiSaveInfoType_Enumerate;
	#if PLATFORM_PC
		m_EnumerationFileExtension = "photo";
	#endif
	#ifdef PLATFORM_PS3
		m_TitleId = PlatformPS3::g_ps3_titleId;
		m_FilePath = "MemSavePS3";
		m_uSizeIcon0 = 68327;
		m_uSizeIcon1 = 0;
		m_uSizePic1 = 546985;
		m_uSizeSnd0 = 0;

		SetParamSFO_Title( UI::C_TextStringID( "PRO_PS3_SAVE_PHOTO_TITLE" ).String() );
		SetParamSFO_Detail( UI::C_TextStringID( "PRO_PS3_SAVE_PHOTO_DETAIL" ).String() );
		SetSubtitle( UI::C_TextStringID( "PRO_PS3_SAVE_PHOTO_SUBTITLE" ).String() );

		m_ListName = "PHOTO";
		SetListNewDataTitle( UI::C_TextStringID( "PRO_PS3_SAVE_PHOTO_NEW_DATA" ).String() );

		SetSecureFieldId( "BEAUTIFUL_CHAOS!" );
	#endif
	#ifdef PLATFORM_PC
		m_SaveRoot = "PHOTO";
	#endif
	}

public:
	void	Setup( const bool bLoading, const S32 sEnumeration )
			{
				m_FileInfo.Clear();

				m_sEnumeration = sEnumeration;

				U32 uFileSize = 0;	/* set from save-system on load */

				if ( !bLoading )
				{
					// Lock buffer and encode to JPEG:
					PhotoMode::C_AppPhotoCapture& photoCapture = PhotoMode::AppPhotoCapture();
					U8* bits = 0;
					S32 pitch = 0, width = 0, height = 0, format = 0;
					photoCapture.LockBitmap(bits, pitch, width, height, format);

					//make a comma delimited comment for the upload
					Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

					Util::C_String16 str;
					Util::C_String256 comment_str;

					if(Game::C_PhotomodeManager::Instance().InUI())
					{
						comment_str.Printf("DOWNLOADED");
					}
					else
					{
						//CONTENT_SERVER_TAGS_PHOTO_CONTENT_TYPE
						str.Printf("%x,",Game::CONTENT_SERVER_TYPE_PHOTO); comment_str+=str;

						//CONTENT_SERVER_TAGS_PHOTO_LOCATION
						str.Printf("%x,",levelState.mCity); comment_str+=str;

						Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
						pLayer->CalculateBiggestOnScreenVehicles();

						Btl::Vector<int> sorted_vehicles;
						for(unsigned int a=0;a<levelState.mNumCars;a++)
						{
							sorted_vehicles.PushBack(a);
						}

						bool sorting=true;
						while(sorting)
						{
							sorting=false;

							for(int a=0;a<sorted_vehicles.Size()-1;a++)
							{
								if(pLayer->mBiggestOnScreenVehicles[sorted_vehicles[a]] < pLayer->mBiggestOnScreenVehicles[sorted_vehicles[a+1]])
								{
									int temp=sorted_vehicles[a];
									sorted_vehicles[a]=sorted_vehicles[a+1];
									sorted_vehicles[a+1]=temp;

									sorting=true;
								}
							}
						}

						for(unsigned int a=0;a<Game::MAX_CONTENT_SERVER_PHOTO_VEHICLE_TAGS;a++)
						{
							U32 carID=-1;
							if(a < levelState.mNumCars && pLayer->mBiggestOnScreenVehicles[sorted_vehicles[a]])
							{
								carID=levelState.mCars[sorted_vehicles[a]].mCarID;
							}

							str.Printf("%x,",carID); comment_str+=str;
						}

						for(int a=0;a<Game::MAX_CONTENT_SERVER_PHOTO_POWERUP_TAGS;a++)
						{
							if(UI::GetVehicleLogicActor(a))
							{
								VehiclePickupInfo info=UI::GetVehicleLogicActor(a)->GetPickupInfo();
								
								if(info.mCurrentlyUsingType == E_PICKUP_TYPE_NONE)
								{
									str.Printf("%x,",info.mCurrentlyUsingType); comment_str+=str;
								}
								else
								{
									str.Printf("%x,",info.mCurrentlyUsingType + (info.mCurrentlyUsingLevel*100)); comment_str+=str;
								}
							}
							else
							{
								str.Printf("%x,",-1); comment_str+=str;
							}
						}

						//race position (for an achievement)
						str.Printf("%x,",UI::GetVehicleLogicActor(0)->GetPositionInfo().mRacePosition); comment_str+=str;
					}

					m_Encoder.SetComments(comment_str.Chars(), comment_str.Length() + 1);


					m_Encoder.EncodeFromBuffer(bits, pitch, width, height, format);


					photoCapture.UnlockBitmap();

					uFileSize = m_Encoder.JpegLength();

					// current time
					Time::C_DecomposedTime time( Time::GetCurrentTime() );

				#if PLATFORM_PC
					m_FileName.Printf( "%S.photo", C_UILocaliser::Instance().ConvertSystemTimeToFileName( time ) );
				#endif
				}

				m_FileInfo.PushBack( 
					C_SaveFileInfo(
						  uFileSize							// m_uFileSize
					#if PLATFORM_PC
						, m_FileName						// m_FileName
					#else
						, "PHOTO"							// m_FileName
					#endif
					)
				);
			}

	bool	Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
			{
				PhotoMode::C_AppPhotoCapture& photoCapture = PhotoMode::AppPhotoCapture();

				m_Decoder.SetData( pBuffer, uDataSize );
				
				U8* bits = 0;
				S32 pitch = 0, width = 0, height = 0, format = 0;
				photoCapture.LockBitmap(bits, pitch, width, height, format);
				m_Decoder.DecodeIntoBuffer(bits, pitch, width, height, format);
				
				Game::C_PhotomodeManager::Instance().SetJpegTagStr((const char *)m_Decoder.Comments());
				
				photoCapture.UnlockBitmap();

				m_Decoder.Exit();

				return true;
			}

	bool	Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
			{
				memcpy( pBuffer, m_Encoder.JpegData(), uDataSize );

				m_Encoder.Exit();

				if ( Game::C_GameProfileManager::Instance().GetProfile( sPadNo ) )
					Game::C_GameProfileManager::Instance().GetProfile( sPadNo )->mPhotosSaved++;

				return true;
			}

public:
	JpegHelper::C_JpegEncoder 	m_Encoder;
	JpegHelper::C_JpegDecoder 	m_Decoder;

#if PLATFORM_PC
	Util::C_FixedString< SAVE_FILENAME_SIZE >		m_FileName;
#endif
};

/*
// ---------------------------
// C_VinylInfoInterfaceProfile
// ---------------------------
class C_VinylInfoInterfaceProfile : public C_SaveInfoInterface
{
public:
	C_VinylInfoInterfaceProfile()
		: C_SaveInfoInterface()
	{
	#if PLATFORM_XENON || PLATFORM_PC
		m_EnumerationFileExtension = "vinyl";
	#endif
	#ifdef PLATFORM_XENON
		m_SaveRoot = "VINYL";
		m_bRoamable = true;
	#endif
	#ifdef PLATFORM_PS3
		m_TitleId = PlatformPS3::g_ps3_titleId;
		m_FilePath = "MemSavePS3";
		m_uSizeIcon0 = 68327;
		m_uSizeIcon1 = 0;
		m_uSizePic1 = 546985;
		m_uSizeSnd0 = 0;
		m_ListName = "VINYL";

		SetParamSFO_Title( L"Test" );							// TODO - localise
		SetParamSFO_Detail( L"Test Save Data Detail Info" );	// TODO - localise
		SetSubtitle( L"Save Game" );							// TODO - localise
		SetListNewDataTitle( L"Create New Data" );				// TODO - localise

		SetSecureFieldId( "BEAUTIFUL_CHAOS!" );
	#endif
	#ifdef PLATFORM_PC
		m_SaveRoot = "VINYL";
	#endif
	}

public:
	void			SetupFiles()
					{
						m_FileInfo.PushBack( 
							C_SaveFileInfo(
								  50 * 1024												// m_uFileSize
								, "VINYLSAVE"											// m_FileName
							#ifdef PLATFORM_XENON
								, "SAVE:\\vinyl"										// m_InternalName
								, UI::C_TextStringID( "PRO_SAVE_GAME_NAME" ).String()	// m_DisplayName
							#endif
							)
						);
					}

	bool			Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
					{
						return UI::C_VinylManager::Instance().ReadBuffer( pBuffer, uDataSize );
					}

	bool			Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
					{
						return UI::C_VinylManager::Instance().WriteBuffer( pBuffer, uDataSize );
					}
};
*/

}; // namespace UI
