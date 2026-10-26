#ifndef SYSTEM_DATALOGGING_H
#define SYSTEM_DATALOGGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>

namespace DataLog
{
	class DataLogTable;
	class DataLogEvent;

	enum eDataLogType
	{
		kDlSubSession,
	};

	typedef void (*TableCreationCallback)(eDataLogType type, DataLogTable* dlt);
	typedef void (*TableUpdateCallback)(eDataLogType type, DataLogEvent* e);

	//
	// VarType
	//

	enum DlType
	{
		kDlBool,
		kDlS8,
		kDlU8,
		kDlS32,
		kDlU32,
		kDlF32,
		kDlS64,
		kDlString,
		//kVtVector3,
	};

	class DataLogVarBase
	{
	private:
		const char*		m_name;
		DlType			m_type;
		int				m_offset;	// Offset to location in temporary buffer while building event

		DataLogTable*	m_table;

	public:
		DataLogVarBase(const char* name, DlType type, DataLogTable* table) : m_name(name), m_type(type), m_offset(-1), m_table(table)
		{
		}

		const char* Name() const
		{
			return m_name;
		}

		int Offset() const
		{
			return m_offset;
		}

		DlType Type() const
		{
			return m_type;
		}

		const DataLogTable* Table() const
		{
			return m_table;
		}

		friend class DataLogTable;
	};

	class DataLogBool : public DataLogVarBase
	{
	public:
		DataLogBool(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlBool, table)
		{
		}
	};


	class DataLogS8 : public DataLogVarBase
	{
	public:
		DataLogS8(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlS8, table)
		{
		}
	};

	class DataLogU8 : public DataLogVarBase
	{
	public:
		DataLogU8(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlU8, table)
		{
		}
	};

	class DataLogS32 : public DataLogVarBase
	{
	public:
		DataLogS32(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlS32, table)
		{
		}
	};

	class DataLogU32 : public DataLogVarBase
	{
	public:
		DataLogU32(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlU32, table)
		{
		}
	};

	class DataLogS64 : public DataLogVarBase
	{
	public:
		DataLogS64(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlS64, table)
		{
		}
	};

	class DataLogF32 : public DataLogVarBase
	{
	public:
		DataLogF32(const char* name, DataLogTable* table) : DataLogVarBase(name, kDlF32, table)
		{
		}
	};

	class DataLogString : public DataLogVarBase
	{
	private:
		int		m_maxLength;

	public:
		DataLogString(const char* name, int maxLength, DataLogTable* table) :
			DataLogVarBase(name, kDlString, table),
			m_maxLength(maxLength)
		{
		}

		unsigned int MaxLength() const
		{
			return m_maxLength;
		}
	};

	class DataLogTable
	{
	private:
		int								m_maxDataSize;	// How much space do we need to construct the message

		Btl::Vector<DataLogVarBase*>	m_dltVariables;

		const char*						m_name;
		int								m_dbIndex;
		int								m_id;

		static int						m_uniqueId;

		bool							m_linkedToSubSession;

	public:
		DataLogTable(int dbIndex, const char* name, bool linkedToSubSession=true);

		DataLogBool*	AddBool(const char* name);
		DataLogS8*		AddS8(const char* name);
		DataLogU8*		AddU8(const char* name);
		DataLogS32*		AddS32(const char* name);
		DataLogU32*		AddU32(const char* name);
		DataLogS64*		AddS64(const char* name);
		DataLogF32*		AddF32(const char* name);
		DataLogString*	AddString(const char* name, int length);

		bool			Finalise();
		bool			SendDefinitionToServer() const;

		int Id() const
		{
			return m_id;
		}

		const char* Name() const
		{
			return m_name;
		}

		int DbIndex() const
		{
			return m_dbIndex;
		}

		unsigned int MaxDataSize() const
		{
			return m_maxDataSize;
		}

		const Btl::Vector<DataLogVarBase*>& Variables() const
		{
			return m_dltVariables;
		}
	};

	class DataLogEvent
	{
	private:
		const DataLogTable*	m_table;
		unsigned int		m_bufferLength;
		U8*					m_buffer;

	public:
		DataLogEvent(const DataLogTable* table);
		~DataLogEvent();

		void Set(DataLogBool* var, bool value);
		void Set(DataLogS8* var, S8 value);
		void Set(DataLogU8* var, U8 value);
		void Set(DataLogS32* var, S32 value);
		void Set(DataLogU32* var, U32 value);
		void Set(DataLogS64* var, U64 value);
		void Set(DataLogF32* var, F32 value);
		void Set(DataLogString* var, const char* value);

		bool Send();
	};

	void Initialise();

	int Connect(const char* databaseName);
	int Connect(const char* databaseName, TableCreationCallback creationCallback, TableUpdateCallback updateCallback);

	void SetEnabledStatus(int dbIndex, bool enable);

	bool IsConnected(int dbIndex);

	void SetReference(int dbIndex, const char* ref);

	void BeginSubSession(int dbIndex);
	void EndSubSession(int dbIndex);

	void NetworkUpdate();
}

#endif
