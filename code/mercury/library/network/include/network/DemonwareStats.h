// ***************************************************************
//  DemonwareStats   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Demonware scoreboard reading / writing.
// ***************************************************************

#ifndef NETPLAY_DEMONWARE_STATS_H
#define NETPLAY_DEMONWARE_STATS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/ScoreboardStructures.h>
#include <bdLobby/bdStats/bdStatsInfo.h>


namespace Scoreboard
{
	/// This is a helper class for storing the column data that is to be written to a single demonware scoreboard.
	/** The game does not need to create an instance of this, instead it's used internally to the C_ScoreboardWriter class which provides a cross platform way of 
	 *	writing scoreboards.
	 */
	class C_DemonwareWriter : public bdStatsInfo
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		explicit C_DemonwareWriter() 
		{Flush();}

		virtual ~C_DemonwareWriter()
		{}

		// used when writing the stats data.
		virtual void serialize(bdByteBuffer &buffer) const
		{
			if (!m_Valid)
				return; // data isn't valid.

			// write rating
			bdStatsInfo::serialize(buffer);

			// write column data.
			for (S32 i=0; i<m_ColumnData.Size(); i++)
			{
				const ColumnData& c = m_ColumnData[i];
				switch (c.m_DataType)
				{
				case Int32:
					buffer.writeInt32(c.m_Data.m_S32);
					break;
				case Int64:
					buffer.writeInt64(c.m_Data.m_S64);
					break;
				case Float64:
					buffer.writeFloat64(c.m_Data.m_F64);
					break;
				case Float32:
					buffer.writeFloat32(c.m_Data.m_F32);
					break;
				default:
					assert(false); // invalid type.
					break;
				}
			}
		}

		virtual bdUInt sizeOf()
		{
			bdUInt size = bdStatsInfo::sizeOf();
			for (S32 i=0; i<m_ColumnData.Size(); i++)
			{
				const ColumnData& c = m_ColumnData[i];
				switch (c.m_DataType)
				{
				case Int32:
					size += sizeof(Int32);
					break;
				case Int64:
					size += sizeof(Int64);
					break;
				case Float64:
					size += sizeof(Float64);
					break;
				case Float32:
					size += sizeof(Float32);
					break;
				default:
					assert(false); // invalid type.
					break;
				}
			}
			return size;
		}

		void Init(const S64 rating, const U32 leaderboardId, const eWriteType writeType)
		{
			m_rating = rating;
			m_leaderboardID = leaderboardId;
			switch(writeType)
			{
			case eWriteTypeReplace:
				m_writeType = bdStats::STAT_WRITE_REPLACE;
				break;
			case eWriteTypeAdd:
				m_writeType = bdStats::STAT_WRITE_ADD;
				break;
			case eWriteTypeMax:
				m_writeType = bdStats::STAT_WRITE_MAX;
				break;
			case eWriteTypeMin:
				m_writeType = bdStats::STAT_WRITE_MIN;
				break;
			default:
				m_writeType = bdStats::STAT_WRITE_REPLACE;
				break;
			}
			m_ColumnData.Clear();
			m_Valid = true;
		}

		U32 getLeaderboardId(void) const {return m_leaderboardID;}

		// flush to default.
		void Flush(void)
		{
			m_Valid = false;
			m_rating = m_rank = m_entityID = m_leaderboardID = 0;
			m_writeType = bdStats::STAT_WRITE_REPLACE;
			m_entityName[0] = '\0';
			m_ColumnData.Clear();
		}

		// trt and add a column.
		bool AddColumn(const ColumnData& c)
		{
			if (!m_Valid)
				return false;

			if (m_ColumnData.Size() >= m_ColumnData.Capacity())
				return false; // out of columns !
			
			switch (c.m_DataType)
			{
			case Int32:
			case Int64:
			case Float64:
			case Float32:
				m_ColumnData.PushBack(c);
				break;

			default:
				return false;
				break;
			}

			return true;
		}

	private:
		bool		m_Valid;
		ColumnList	m_ColumnData;	// actual column data to write.
	};


	/// This is a helper class for storing the column data that has been read from a single demonware scoreboard row.
	/** The game does not need to create an instance of this, instead it's used internally to the C_ScoreboardReaderXXX class which provides a cross platform way of 
	 *	reading scoreboards.
	 */
	class C_DemonwareRow : public bdStatsInfo
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		explicit C_DemonwareRow() 
		{Flush();}

		virtual ~C_DemonwareRow()
		{}

		// flush to default.
		void Flush(void)
		{
			m_rating = m_rank = m_entityID = m_leaderboardID = 0;
			m_writeType = bdStats::STAT_WRITE_REPLACE;
			m_entityName[0] = '\0';

			m_pColumnTypeList = NULL;
			m_ColumnData.Clear();
		}

		virtual bdUInt sizeOf()
		{
			// need to return full size of class since demonware need to know the size of the class when reading
			// data back into this class when enumerating sessions.
			return sizeof(C_DemonwareRow);
		}

		// read data that has been returned.
		bdBool deserialize(bdByteBufferRef buffer)
		{
			bdBool ok = bdStatsInfo::deserialize(buffer);
			if (m_pColumnTypeList)
			{
				for (S32 i=0; ok && (i<m_pColumnTypeList->Size()); i++)
				{
					const Scoreboard::eColumnDataType expectedType = (*m_pColumnTypeList)[i];
					if (expectedType == Scoreboard::Int32)
					{
						S32 t=0;
						ok = buffer->readInt32(t);
						if (ok)
						{
							ColumnData c;
							c.SetS32(i, t);
							m_ColumnData.PushBack(c);
						}
					}
					else if (expectedType == Scoreboard::Int64)
					{
						S64 t=0;
						ok = buffer->readInt64(t);
						if (ok)
						{
							ColumnData c;
							c.SetS64(i, t);
							m_ColumnData.PushBack(c);
						}
					}
					else if (expectedType == Scoreboard::Float32)
					{
						float t=0;
						ok = buffer->readFloat32(t);
						if (ok)
						{
							ColumnData c;
							c.SetF32(i, t);
							m_ColumnData.PushBack(c);
						}
					}
					else if (expectedType == Scoreboard::Float64)
					{
						double t=0;
						ok = buffer->readFloat64(t);
						if (ok)
						{
							ColumnData c;
							c.SetF64(i, t);
							m_ColumnData.PushBack(c);
						}
					}
					else
					{
						// undefined type returned.
						assert(false);
						ok = false;
					}
				}
			}
			return ok;
		}

	private:
		C_DemonwareRow(const C_DemonwareRow& original);				// private copy constructor
		C_DemonwareRow& operator = (const C_DemonwareRow rhs);		// private un coded assignment operator

	public:
		ColumnList		m_ColumnData;		// actual column data that has been read.
		ColumnTypeList*	m_pColumnTypeList;	// defines what types the returned columns should be and in what order.
	};


	/// This is a helper class for parsing the data returned in a scoreboard query.
	/** The game does not need to create an instance of this, instead it's used internally to the C_ScoreboardReaderXXX class which provides a cross platform way of 
	 *	reading scoreboards.
	 */
	class C_DemonwareReader 
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		explicit C_DemonwareReader()  : m_pRows(NULL), m_ScoreboardId(0)
		{Flush();}
		virtual ~C_DemonwareReader()
		{Flush();}

		void Configure(const ColumnTypeList& cols, const U32 scoreboardId)
		{
			m_ColumnTypes = cols;
			m_ScoreboardId = scoreboardId;
		}
		
		bool Configure(const U32 maxResults)
		{
			if (m_pRows)
				delete [] m_pRows;
			m_pRows = NULL;

			m_MaxRowsAllocated = maxResults;
			m_pRows = new C_DemonwareRow[m_MaxRowsAllocated];
			if (NULL == m_pRows)
				return false;

			// set expected return types before we deserialise
			for(U32 i=0; i<m_MaxRowsAllocated; i++)
			{	
				C_DemonwareRow& row = m_pRows[i];
				row.m_pColumnTypeList = &m_ColumnTypes; 
			}

			return true;
		}

		void Flush(void)
		{
			if (m_pRows)
				delete [] m_pRows;
			m_pRows = NULL;
			m_MaxRowsAllocated = 0;
			m_TotalScoreboardEnteries = 0;
			m_NumRowsReturned = 0;
		}

		C_DemonwareRow* Data(void) {return m_pRows;}
	
	private:
		C_DemonwareReader(const C_DemonwareReader& original);				// private copy constructor
		C_DemonwareReader& operator = (const C_DemonwareReader rhs);		// private un coded assignment operator

	public:
		ColumnTypeList	m_ColumnTypes;				// what are the coloum types that we expect to return
		C_DemonwareRow*	m_pRows;					// actual row data that has been allocated.
		U32				m_MaxRowsAllocated;			// maximum number of rows we have allocated
		U32				m_TotalScoreboardEnteries;	// how many enteries are currently on this scoreboard.
		U32				m_ScoreboardId;				// what scoreboard was read ?
		U32				m_NumRowsReturned;			// how many rows did we return.
	};
}

#endif
