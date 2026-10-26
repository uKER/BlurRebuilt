#include "SystemPch.h"

#include <System/Artery/Artery.h>
#include "system/automation/Automation.h"
#include <system/memory/MemoryTracker.h>

#include <system/debug/variables.h>
#include <system/debug/BuildStamp.h>

#ifdef PLATFORM_PS3
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <system\artery\artery.h>
#include <system\datalogging\DataLogging.h>
#include <system/debug/BuildStamp.h>
#include <system\serialise.h>
#include <system\serialise\TextSerialise.h>
#include <system/time/time.h>

/*
	The datalog system looks for a file called 'datalog.ini'. The format is

	[databaseName]
	server=192.168.1.1
	enabled=true
	reference=customidentifier
*/

namespace DataLog
{
	static Debug::Logger &gDataLogLogger( Debug::GetLogger("DataLogging") );

	static Debug::Bool gShowDataLog("System.DataLogging.ShowLoggedData", false, Debug::Persist());

	static bool AllowTimeout = false;

	static Time::C_Time	g_prevTime = Time::GetPerformanceCounter();

	int DataLogTable::m_uniqueId = 1;

	MT::Lock			gDataLock;

	bool				gInitialised		= false;
	bool				gSubSessionActive	= false;

	const char* GetBuildConfig()
	{
		#ifdef BUILD_CONFIG_DEBUG
		return "Debug";
		#endif

		#ifdef BUILD_CONFIG_RELEASE
		return "Release";
		#endif

		#ifdef BUILD_CONFIG_SPEED
		return "Speed";
		#endif

		#ifdef BUILD_CONFIG_PROFILE
		return "Profile";
		#endif

		#ifdef BUILD_CONFIG_MASTER
		return "Master";
		#endif
	}

	class DatabaseLocation
	{
	public:
		DatabaseLocation() :
		  dataLogConnection(NULL),
			  initialised(false),
			  subSessionActive(false),
			  m_pDebugVar(NULL),
			  subsessionTable(NULL),
			  creationCallback(NULL),
			  updateCallback(NULL)
		{
		}

		void Disconnect()
		{
			initialised = false;
			dataLogConnection = NULL;
		}

		bool Enabled() const
		{
			return *m_pDebugVar;
		}

		bool IsInitialised() const
		{
			return initialised;
		}

		Util::C_String				name;
		Util::C_String				ipAddress;
		Util::C_String				reference;
		ArteryConnection			aConnection;
		ArteryConnection*			dataLogConnection;

		TableCreationCallback		creationCallback;
		TableUpdateCallback			updateCallback;

		Debug::Bool*				m_pDebugVar;

		DataLogTable*				subsessionTable;
		Btl::Vector<DataLogTable*>	dataLogTables;

		bool						subSessionActive;
		bool						initialised;
	};

	Btl::Vector<DatabaseLocation>	gDatabaseLocations;

	bool SendMessage(int dbIndex, ArteryMessageBuilder& mb, bool allowedToSendBeforeFinishedSetup=false)
	{
		bool result = false;

		if (dbIndex != -1)
		{
			gDataLock.Enter();

			ArteryConnection* dlc = gDatabaseLocations[dbIndex].dataLogConnection;
			
			if (dlc && dlc->IsConnected())
			{
				if (allowedToSendBeforeFinishedSetup || gDatabaseLocations[dbIndex].IsInitialised())
				{
					ArteryMessage m;
					
					if (mb.Build(m))
					{
						dlc->Send(m);
						gSubSessionActive = true;
						result = true;
					}
				}
				else
				{
					LOGWARN(gDataLogLogger, "DataLog: Trying to send to server before tables have been established.");
				}
			}

			gDataLock.Exit();
		}

		return result;
	}

	void BeginSubSession(int dbIndex)
	{
		if (IsConnected(dbIndex))
		{
			DatabaseLocation& dbLoc = gDatabaseLocations[dbIndex];

			// Have we created subsession table yet? If not, do it quick
			if (dbLoc.subsessionTable == NULL)
			{
				dbLoc.subsessionTable = new DataLogTable(dbIndex, "_subsessions", false);

				if (dbLoc.creationCallback)
					dbLoc.creationCallback(kDlSubSession, dbLoc.subsessionTable);

				dbLoc.subsessionTable->Finalise();
			}

			DataLog::DataLogEvent* e = new DataLog::DataLogEvent(dbLoc.subsessionTable);

			if (dbLoc.updateCallback)
				dbLoc.updateCallback(kDlSubSession, e);

			e->Send();
			delete e;

			dbLoc.subSessionActive = true;
		}
	}

	void EndSubSession(int dbIndex)
	{
		if (gSubSessionActive)
		{
			ArteryMessageBuilder mb;
			mb.Write("ENDSUBSESSION");

			SendMessage(dbIndex, mb);

			gDatabaseLocations[dbIndex].subSessionActive = false;
		}
	}

	void MrCallback(ArteryConnection con, void* context);

	static int timeout = 0;

	bool IsConnected(int dbIndex)
	{
		if (dbIndex != -1)
		{
			DatabaseLocation& dbl = gDatabaseLocations[dbIndex];

			if (dbl.Enabled() && dbl.dataLogConnection != NULL)
			{
				return true;
			}
		}

		return false;
	}

	void SetReference(int dbIndex, const char* ref)
	{
		if (dbIndex != -1)
		{
			DatabaseLocation& dbl = gDatabaseLocations[dbIndex];

			if (dbl.reference != ref)
			{
				if (IsConnected(dbIndex))
				{
					dbl.dataLogConnection->Close();
					dbl.dataLogConnection = NULL;
				}
				
				dbl.reference = ref;
			}
		}
	}

	void SetEnabledStatus(int dbIndex, bool enable)
	{
		if (dbIndex != -1)
		{
			DatabaseLocation& dbl = gDatabaseLocations[dbIndex];

			dbl.m_pDebugVar->SetValue(enable);
		}
	}

	void NetworkUpdate()
	{
		const Time::C_Time currTime = Time::GetPerformanceCounter();
		float timeDiff = (F32)(currTime - g_prevTime) / (F32)Time::GetPerformanceFrequency();

		bool ping = false;

		if (timeDiff > 5.f)	// 5 secs
		{
			ping = true;
			g_prevTime = currTime;
		}

		gDataLock.Enter();

		for (int index=0; index < gDatabaseLocations.Size(); index++)
		{
			DatabaseLocation& dbl = gDatabaseLocations[index];

			if (dbl.Enabled() && dbl.dataLogConnection == NULL)
			{
				if (--timeout <= 0)
				{
					unsigned long ipAddr = inet_addr(dbl.ipAddress.Chars());

					if (ipAddr != INADDR_NONE)
					{
						ArterySockAddr atarg(ipAddr, 6317);

						bool ret = ArteryConnect(dbl.aConnection, atarg, "datalog", MrCallback, (void*)index);
					
						LOGDEBUG(gDataLogLogger, "DataLog: NetworkUpdate: %s - Enabled but no connection - Ret(%d)\n", dbl.name.Chars(), ret);
					}

					timeout = 60;
				}
			}

			if (dbl.dataLogConnection && dbl.dataLogConnection->IsConnected())
			{
				// Once a connection is made we need to send over any tables that have been created
				if (!dbl.initialised)
				{
					for (int i=0; i < dbl.dataLogTables.Size(); i++)
						dbl.dataLogTables[i]->SendDefinitionToServer();

					dbl.initialised = true;
				}
				else if (ping)
				{
					// Ping server to keep it happy
					ArteryMessageBuilder mb;
					mb.Write("PING");
					SendMessage(index, mb);
				}
			}
		}
	
		gDataLock.Exit();
	}

	void MrCallback(ArteryConnection con, void* context)
	{
#ifndef CFG_GOLD
		ArteryWaitContext wc;

		DatabaseLocation& dbLocation = gDatabaseLocations[(int)(INTPTR)context];	// [Blur2PC][x64] index carried in the context pointer

		if (con.IsConnected())
		{
			gDataLock.Enter();

			dbLocation.dataLogConnection = &con;

			{
				ArteryMessageBuilder mb;
				mb.Write("SESSION");

				mb.Write((int)1);	// Version

#if defined(PLATFORM_PS3)
				mb.Write((int)2);	// PS3
#elif defined(PLATFORM_PC)
				mb.Write((int)3);	// PC
#else
				mb.Write((int)0);	// Unknown
#endif

				mb.Write(dbLocation.name); // Database name

				char name[256];
				strcpy_s(name, lengthof(name), "unknown");

				mb.Write("KITNAME");
				mb.Write(name);
				mb.Write("CONFIG");
				mb.Write(GetBuildConfig());
				mb.Write("VERSION");
				mb.Write(Debug::GetBuildVersion());
				
				if (dbLocation.reference.Length() > 0)
				{
					mb.Write("REFERENCE");
					mb.Write(dbLocation.reference.Chars());
				}

				ArteryMessage m;
				if (mb.Build(m))
				{
					dbLocation.dataLogConnection->Send(m);
				}
			}

			gDataLock.Exit();
		}

		while (con.IsConnected())
		{
			bool waitRet;
			
			if (AllowTimeout)
				waitRet = con.Wait(wc, 5000);
			else
				waitRet = con.Wait(wc);

			if (waitRet)
			{
				ArteryMessage msg;

				if (con.TryReceive(msg))
				{
					LOGWARN(gDataLogLogger, "MrCallback - Message arrived\n");
				}
			}
		}

		gDataLock.Enter();
		dbLocation.Disconnect();
		gDataLock.Exit();
#endif
	}

	/*
		Initialise the data logging system.

		Checks for a file which describes where to connect to for the different data log sources
	*/
	void Initialise()
	{
#ifndef CFG_GOLD
		Sys::I_File* file = Sys::GetDefaultFileSystem()->Open("datalog.ini", Sys::E_FileAccess_Read);
		
		if (file)
		{
			Sys::TextDeserialiser deserial(*file);

			DatabaseLocation* currentDl = NULL;

			do
			{
				Util::C_String line = "";

				deserial >> line;

				line.ConvertToLower();

				int rpos = line.Search(']');

				// Start of a new database
				if (line[0] == '[' && rpos != -1 && rpos > 0)
				{
					DatabaseLocation dl;

					dl.name.CopyFrom(line.Chars()+1, rpos-1);
					dl.m_pDebugVar = new Debug::Bool((Util::C_String("System.DataLogging.") + dl.name).Chars(), false, Debug::Dynamic());
					
					gDatabaseLocations.PushBack(dl);

					currentDl = &gDatabaseLocations.Back();
				}
				else if (currentDl != NULL)
				{
					int separator = line.Search('=');

					if (separator > 0 && separator < line.Length()-2)
					{
						Util::C_String key, value;

						// Trim whitespace off the end of the line
						int length = line.Length();
						for (; isspace(line[length-1]); length--);
						
						key.CopyFrom(line.Chars(), separator);
						value.CopyFrom(line.Chars()+separator+1, length-separator-1);

						if (key == "server")
						{
							currentDl->ipAddress = value;
						}
						else if ((key == "reference") || (key == "ref"))
						{
							currentDl->reference = value;
						}
						else if (key == "enabled")
						{
							if (value == "true")
								currentDl->m_pDebugVar->SetValue(true);
						}
					}
				}
			}
			while (file->GetPosition() < file->Length());

			delete file;
		}
#endif
	}

	int Connect(const char* databaseName, TableCreationCallback creationCallback, TableUpdateCallback updateCallback)
	{
#ifndef CFG_GOLD
		for (int index=0; index < gDatabaseLocations.Size(); index++)
		{
			DatabaseLocation& dbl = gDatabaseLocations[index];

			if (!stricmp(dbl.name, databaseName))
			{
				dbl.creationCallback = creationCallback;
				dbl.updateCallback = updateCallback;
				return index;
			}
		}
#endif

		// Couldn't find match in file, or couldn't make connect
		return -1;
	}

	int Connect(const char* databaseName)
	{
		return Connect(databaseName, NULL, NULL);
	}


	DataLogTable::DataLogTable(int dbIndex, const char* name, bool linkedToSubSession) :
		m_name(name),
		m_dbIndex(dbIndex),
		m_maxDataSize(0),
		m_linkedToSubSession(linkedToSubSession)
	{
		m_id = m_uniqueId++;
	}

	DataLogBool* DataLogTable::AddBool(const char* name)
	{
		DataLogBool* dltv = new DataLogBool(name, this);

		m_dltVariables.PushBack(dltv);

		return dltv;
	}

	DataLogS8* DataLogTable::AddS8(const char* name)
	{
		DataLogS8* dltv = new DataLogS8(name, this);

		m_dltVariables.PushBack(dltv);

		return dltv;
	}

	DataLogU8* DataLogTable::AddU8(const char* name)
	{
		DataLogU8* dltv = new DataLogU8(name, this);

		m_dltVariables.PushBack(dltv);

		return dltv;
	}

	DataLogS32* DataLogTable::AddS32(const char* name)
	{
		DataLogS32* dltv = new DataLogS32(name, this);

		// We do this for aligned types to pack them together at the front of the buffer
		dltv->m_offset = m_maxDataSize;
		m_maxDataSize += 4;

		m_dltVariables.PushBack(dltv);

		return dltv;
	}

	DataLogU32* DataLogTable::AddU32(const char* name)
	{
		DataLogU32* dltv = new DataLogU32(name, this);

		// We do this for aligned types to pack them together at the front of the buffer
		dltv->m_offset = m_maxDataSize;
		m_maxDataSize += 4;

		m_dltVariables.PushBack(dltv);

		return dltv;
	}

	DataLogF32* DataLogTable::AddF32(const char* name)
	{
		DataLogF32* dltv = new DataLogF32(name, this);

		// We do this for aligned types to pack them together at the front of the buffer
		dltv->m_offset = m_maxDataSize;
		m_maxDataSize += 4;

		m_dltVariables.PushBack(dltv);

		return dltv;
	}

	DataLogString* DataLogTable::AddString(const char* name, int maxLength)
	{
		DataLogString* dltv = new DataLogString(name, maxLength, this);

		m_dltVariables.PushBack(dltv);
		m_maxDataSize += maxLength;

		return dltv;
	}

	bool DataLogTable::SendDefinitionToServer() const
	{
		// Now create the message to send to the server
		ArteryMessageBuilder mb;
			
		mb.Write("TABLE");
		mb.Write((int)1);	// Version	 - // 2

		mb.Write(m_linkedToSubSession);

		// Table name
		mb.Write(m_name);

		// The Id we'll use to identify this table type when sending data
		mb.Write(m_id);

		// Column count of the table
		mb.Write(m_dltVariables.Size());

		// Package up column names and types
		for (int i=0; i < m_dltVariables.Size(); i++)
		{
			const DataLogVarBase& dlv = *(m_dltVariables[i]);

			mb.Write(dlv.Name());
			mb.Write(dlv.Type());

			//if (dlv.Type() == kDlString)
			//{
			//	const DataLogString& dlvs = (DataLogString&) *(m_dltVariables[i]);

			//	mb.Write(dlvs.MaxLength());
			//}
		}

		return SendMessage(m_dbIndex, mb, true);
	}

	bool DataLogTable::Finalise()
	{
		// First we work out the offsets - Big stuff first, then bools, then strings
		int offset = 0;

		for (int i=0; i < m_dltVariables.Size(); i++)
		{
			DataLogVarBase& dlv = *(m_dltVariables[i]);

			switch (dlv.Type())
			{
			case kDlBool:
			case kDlS8:
			case kDlU8:
				dlv.m_offset = m_maxDataSize;
				m_maxDataSize += 1;
				break;

			case kDlString:
				dlv.m_offset = m_maxDataSize;
				m_maxDataSize += ((DataLogString&)dlv).MaxLength();
				break;

				// These guys have already sorted themselves out
			case kDlS32:
			case kDlU32:
			case kDlF32:
				break;

			default:
				BIZ_ASSERT_FAIL("New type we don't handle");
				break;
			}
		}

		if (m_dbIndex != -1)
		{
			gDataLock.Enter();
			gDatabaseLocations[m_dbIndex].dataLogTables.PushBack(this);
			gDataLock.Exit();

			SendDefinitionToServer();

			return true;
		}
		
		return false;
	}

	/*
		DataLogEvent
	*/

	DataLogEvent::DataLogEvent(const DataLogTable* table)
	{
		m_table = table;
		m_bufferLength = table->MaxDataSize();
		m_buffer = new U8[m_bufferLength];
		memset(m_buffer, 0, m_bufferLength);
	}

	DataLogEvent::~DataLogEvent()
	{
		delete m_buffer;
	}

	void DataLogEvent::Set(DataLogBool* var, bool value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + sizeof(bool));

		*((bool*)(m_buffer + var->Offset())) = value;
	}

	void DataLogEvent::Set(DataLogS8* var, S8 value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + sizeof(S8));

		*((S8*)(m_buffer + var->Offset())) = value;
	}

	void DataLogEvent::Set(DataLogU8* var, U8 value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + sizeof(U8));

		*((U8*)(m_buffer + var->Offset())) = value;
	}

	void DataLogEvent::Set(DataLogS32* var, S32 value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + sizeof(S32));

		*((S32*)(m_buffer + var->Offset())) = value;
	}
	
	void DataLogEvent::Set(DataLogU32* var, U32 value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + sizeof(U32));

		*((U32*)(m_buffer + var->Offset())) = value;
	}

	void DataLogEvent::Set(DataLogF32* var, F32 value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + sizeof(F32));
		
		*((F32*)(m_buffer + var->Offset())) = value;
	}

	void DataLogEvent::Set(DataLogString* var, const char* value)
	{
		BIZ_ASSERT(var != NULL);
		BIZ_ASSERT(m_bufferLength >= var->Offset() + var->MaxLength());
		BIZ_ASSERT(m_table == var->Table());
		
		strncpy_s((char*)(m_buffer + var->Offset()), var->MaxLength(), value, _TRUNCATE);
	}

	bool DataLogEvent::Send()
	{
		ArteryMessageBuilder mb;
			
		mb.Write("EVENT");
		mb.Write((int)1);	// Version

		// Write out event type ID
		mb.Write(m_table->Id());

		// Loop over variables - put values into buffer
		const Btl::Vector<DataLogVarBase*>& variables = m_table->Variables();

		for (int i=0; i < variables.Size(); i++)
		{
			const DataLogVarBase& dlv = *(variables[i]);

			switch (dlv.Type())
			{
			case kDlBool:
				{
					bool value = *((bool*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}
			
			case kDlS8:
				{
					S8 value = *((S8*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}

			case kDlU8:
				{
					U8 value = *((U8*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}

			case kDlS32:
				{
					S32 value = *((S32*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}

			case kDlU32:
				{
					U32 value = *((U32*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}

			case kDlS64:
				{
					S64 value = *((S64*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}

			case kDlF32:
				{
					F32 value = *((F32*)(m_buffer + dlv.Offset()));
					mb.Write(value);
					break;
				}

			case kDlString:
				{
					const char* value = (const char*)(m_buffer + dlv.Offset());
					mb.Write(value);
					break;
				}
			}
		}

		SendMessage(m_table->DbIndex(), mb);

		if (gShowDataLog)
		{
			Util::C_String debugInfo;

			debugInfo = Util::C_String(m_table->Name()) + ":";

			for (int i=0; i < variables.Size(); i++)
			{
				const DataLogVarBase& dlv = *(variables[i]);

				switch (dlv.Type())
				{
				case kDlBool:
					{
						bool value = *((bool*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%s)", dlv.Name(), value ? "true" : "false");
						break;
					}

				case kDlS8:
					{
						S8 value = *((S8*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%d)", dlv.Name(), value);
						break;
					}

				case kDlU8:
					{
						U8 value = *((U8*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%d)", dlv.Name(), value);
						break;
					}

				case kDlS32:
					{
						S32 value = *((S32*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%d)", dlv.Name(), value);
						break;
					}

				case kDlU32:
					{
						U32 value = *((U32*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%d)", dlv.Name(), value);
						break;
					}

				case kDlS64:
					{
						S64 value = *((S64*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%d)", dlv.Name(), value);
						break;
					}

				case kDlF32:
					{
						F32 value = *((F32*)(m_buffer + dlv.Offset()));
						debugInfo += Util::C_String::Format(" %s(%f)", dlv.Name(), value);
						break;
					}

				case kDlString:
					{
						const char* value = (const char*)(m_buffer + dlv.Offset());
						debugInfo += Util::C_String::Format(" %s(%s)", dlv.Name(), value);
						break;
					}
				}
			}

			LOGINFO(gDataLogLogger, "DL: %s", debugInfo.Chars());
		}

		return true;
	}
}
