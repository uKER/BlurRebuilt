#ifndef NETSTORAGE_H
#define NETSTORAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/SaveSystem/SaveSystem.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

BD_REFERENCE(netStream);

namespace netStorage
{
	// --------------------------
	// C_SaveInfoInterfaceProfile
	// --------------------------
	BD_REFERENCE(Interface);
	class Interface: public UI::C_SaveInfoInterface, public bdReferencable
	{
	public:
		Interface(netStreamRef data, int size);
		virtual ~Interface();

		void SetupFiles();

		bool Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData );
		bool Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData );

		netStreamRef m_data;
		int m_size;
	};

	class Device: public Messaging::C_MessageHandler, public bdReferencable
	{
	public:
		Device(InterfaceRef storageInterface);
		~Device();

		bool start(int pad, int mode);
		bool update();
		bool stop(int status);
		bool clear();

		netStreamRef dataBuffer();

		void setState(int state) { m_state = state; }
		void setStatus(int status) { m_status = status; }
		void setMode(int mode) { m_mode = mode; }

		int state() const { return m_state; }
		int status() const { return m_status; }
		int mode() const { return m_mode; }
		
		bool fileNotFound() const { return m_status == STATUS_FILE_NOT_FOUND; }
		bool finished() const { return m_state == STATE_NOT_SET; }
		bool failed() const   { return m_status != STATUS_OK; }
		bool success() const  { return m_status == STATUS_OK; }

	public:
		enum
		{
			STATE_NOT_SET=-1,
			STATE_DEVICE_SEARCH_START,
			STATE_DEVICE_SEARCH_WAIT,
			STATE_LOAD_START,
			STATE_LOAD_WAIT,
			STATE_SAVE_START,
			STATE_SAVE_WAIT,
		};

		enum
		{
			MODE_NOT_SET=-1,
			MODE_LOADING,
			MODE_SAVING,
		};
		
		enum
		{
			STATUS_NOT_SET=-2,
			STATUS_ERROR,
			STATUS_OK,
			STATUS_FILE_NOT_FOUND,
			STATUS_FILE_FOUND,
			STATUS_FILE_CANT_READ,
			STATUS_FILE_CANT_WRITE,
			STATUS_FILE_CANT_DELETE,
			STATUS_NO_DEVICE,
			STATUS_NO_SPACE,
			STATUS_CANCELLED,
		};

	public:
		bool MessageHandler(const Messaging::C_MessageBase* msg);
		int uiDeviceStatusToStatus(int deviceStatus);
		int m_state;
		int m_status;
		int m_mode;
		int m_pad;
		InterfaceRef m_interface;
	};
};

#endif //NETSTORAGE_H