
#include "stdafx.h"
#include "netStorage.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"
#include "netStream/netStream.h"

#include <ui/Game/Profiles/ProfileInterface.h>
#include <ui/GameSaveSystem.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/UIDefines.h>
#include <misc_shared/Messaging/MessageBase.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif
namespace netStorage
{
	// --------------------------
	// C_SaveInfoInterfaceProfile
	// --------------------------
	BD_REFERENCE(Interface);
	Interface::Interface(netStreamRef data, int size)
	: m_data(data)
	, m_size(size)
	{}

	Interface::~Interface()
	{}

	void Interface::SetupFiles()
	{
		m_FileInfo.Clear();

		m_FileInfo.PushBack( 
			UI::C_SaveFileInfo(
				  m_size												// m_uFileSize
				, "MULTIPLAYER"											// m_FileName
			)
		);
	}

	bool Interface::Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
	{
		assert(uDataSize == m_FileInfo[ 0 ].m_uFileSize);

		memcpy(m_data->buffer(), pBuffer, m_FileInfo[ 0 ].m_uFileSize);
		
		m_data->setWriteByteCount(uDataSize);

		return true;
	}

	bool Interface::Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
	{
		assert(uDataSize == m_FileInfo[ 0 ].m_uFileSize);

		memcpy(pBuffer, m_data->buffer(), m_FileInfo[ 0 ].m_uFileSize);

		return true;
	}

	
	Device::Device(InterfaceRef storageInterface)
	: m_interface(storageInterface)
	{
		SetSubscriberSystem(&(UI::gButtons.GetSubscriberSystem()));
		AddSubscriber(UI::E_UiMsg_SaveLocationFound);
		AddSubscriber(UI::E_UiMsg_SaveLocationNotFound);
		AddSubscriber(UI::E_UiMsg_SaveEnumerationFinished);
		clear();
	}

	Device::~Device()
	{
		stop(STATUS_OK);
	}

	netStreamRef Device::dataBuffer()
	{
		return m_interface->m_data;
	}

	bool Device::MessageHandler(const Messaging::C_MessageBase* msg)
	{
		switch(msg->Type())
		{
		default:
			{
				return false;
			}
		case UI::E_UiMsg_SaveLocationFound:
			{
				assert(m_state == STATE_DEVICE_SEARCH_WAIT);
				switch(m_mode)
				{
				default:
					{
						stop(STATUS_OK);
						return true;
					}
				case MODE_LOADING:
					{
						setState(STATE_LOAD_START);
						return true;
					}
				case MODE_SAVING:
					{
						setState(STATE_SAVE_START);
						return true;
					}
				}
				return false;
			}
		case UI::E_UiMsg_SaveLocationNotFound:
			{
				assert(m_state == STATE_DEVICE_SEARCH_WAIT);
				assert(!"<netStorage::Device> save location not found.");
				
				// the HUD is up. THis, on the XBox, will trigger a 'device not found' message
				if(SysProfile::C_Manager::Instance().ShowingSystemOverlay())
					setState(STATE_DEVICE_SEARCH_START);
				else
					stop(STATUS_NO_DEVICE);
				return false;
			}
		case UI::E_UiMsg_SaveEnumerationFinished:
			{
				assert(m_state == STATE_DEVICE_SEARCH_WAIT);
				assert(false);
				switch(m_mode)
				{
				default:
					{
						stop(STATUS_OK);
						return true;
					}
				case MODE_LOADING:
					{
						setState(STATE_LOAD_START);
						return true;
					}
				case MODE_SAVING:
					{
						setState(STATE_SAVE_START);
						return true;
					}
				}
			}
		}
	}

	bool Device::start(int pad, int mode)
	{
		assert(m_state == STATE_NOT_SET);
		clear();

		m_pad = pad;
		m_mode = mode;
		setState(STATE_DEVICE_SEARCH_START);
		return true;
	}

	bool Device::stop(int status)
	{
		m_state   = STATE_NOT_SET;
		m_status  = status;
		return true;
	}
	bool Device::clear()
	{
		m_interface->m_data->clear();
		m_pad	  = -1;
		m_mode	  = MODE_NOT_SET;
		m_state   = STATE_NOT_SET;
		m_status  = STATUS_NOT_SET;
		return true;
	}

	int Device::uiDeviceStatusToStatus(int deviceStatus)
	{
		#define CASETOSTR(A) case UI::E_UiGameProfile_Error_##A: LOGINFO(netDebug::g_Log, "<netStorage::Device> status(%s)", #A); break;
		switch(deviceStatus)
		{
		default: assert(!"<netStorage::Device> status : " "UNKNOWN"); break;
		CASETOSTR(None);
		CASETOSTR(Okay);
		CASETOSTR(FileDoesNotExist);
		CASETOSTR(FileExists);
		CASETOSTR(CannotReadFile);
		CASETOSTR(CannotWriteFile);
		CASETOSTR(CannotDeleteFile);
		CASETOSTR(DeviceNotConnected);
		CASETOSTR(NoFreeSpace);
		CASETOSTR(UserCancelled);
		}
	
		switch(deviceStatus)
		{
		default:												return STATUS_ERROR;
		case UI::E_UiGameProfile_Error_None:					return STATUS_ERROR;
		case UI::E_UiGameProfile_Error_Okay:					return STATUS_OK;
		case UI::E_UiGameProfile_Error_FileDoesNotExist:		return STATUS_FILE_NOT_FOUND;
		case UI::E_UiGameProfile_Error_FileExists:				return STATUS_FILE_FOUND;
		case UI::E_UiGameProfile_Error_CannotReadFile:			return STATUS_FILE_CANT_READ;
		case UI::E_UiGameProfile_Error_CannotWriteFile:			return STATUS_FILE_CANT_WRITE;
		case UI::E_UiGameProfile_Error_CannotDeleteFile:		return STATUS_FILE_CANT_DELETE;
		case UI::E_UiGameProfile_Error_DeviceNotConnected:		return STATUS_NO_DEVICE;
		case UI::E_UiGameProfile_Error_NoFreeSpace:				return STATUS_NO_SPACE;
		case UI::E_UiGameProfile_Error_UserCancelled:			return STATUS_CANCELLED;
		}
	}

	bool Device::update()
	{
		switch(m_state)
		{
		default:
		case STATE_NOT_SET:
			{	
				return true;
			}
		case STATE_DEVICE_SEARCH_START:
			{
				// wait for HUD to close
				if(SysProfile::C_Manager::Instance().ShowingSystemOverlay())
					return true;

				UI::gGameSaveSystem.SetUserIndex( m_pad );

					switch(m_mode)
					{
					default:
						{
							stop(STATUS_OK);
							return true;
						}
					case MODE_LOADING:
						{
							setState(STATE_LOAD_START);
							return true;
						}
					case MODE_SAVING:
						{
							setState(STATE_SAVE_START);
							return true;
						}
					}
				return true;
			}
		case STATE_DEVICE_SEARCH_WAIT:
			{
				return true;
			}
		case STATE_LOAD_START:
			{
				m_interface->m_data->clear();

				m_interface->SetupFiles();
				UI::C_SaveSystem::Instance().SaveInfoInterface( m_interface );
				
				if(!UI::C_SaveSystem::Instance().StartAsyncLoad( m_pad ))
				{
					stop(STATUS_ERROR);
					return true;
				}
				setState(STATE_LOAD_WAIT);
				return true;
			}
		case STATE_LOAD_WAIT:
			{
				if(!UI::C_SaveSystem::Instance().ContinueAsyncLoad())
				{
					return true;
				}

				int status = uiDeviceStatusToStatus(UI::C_SaveSystem::Instance().GetLoadErrorCode());
				stop(status);
				return true;
			}
		case STATE_SAVE_START:
			{
				m_interface->SetupFiles();
				UI::C_SaveSystem::Instance().SaveInfoInterface( m_interface );
				
				if(!UI::C_SaveSystem::Instance().StartAsyncSave( m_pad ))
				{
					stop(STATUS_ERROR);
					return true;
				}

				setState(STATE_SAVE_WAIT);
				return true;
			}
		case STATE_SAVE_WAIT:
			{
				if(!UI::C_SaveSystem::Instance().ContinueAsyncSave())
				{
					return true;
				}

				int status = uiDeviceStatusToStatus(UI::C_SaveSystem::Instance().GetSaveErrorCode());
				stop(status);
				return true;
			}
		}
	}
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

