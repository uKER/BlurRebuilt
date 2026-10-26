// ***************************************************************
//  ScoreboardStructures   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Specific scoreboard structures used throughout.
// ***************************************************************

#ifndef SCOREBOARD_STRUCTURES_H
#define SCOREBOARD_STRUCTURES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <SystemProfile/SystemProfile.h>
#include <bdLobby\bdLobbyCommon\bdLobbyServicesCommon.h>


namespace Scoreboard
{
	// When using demonware this determine how the server will decide if the stats are to be written or not.
	typedef enum eWriteType
	{
		eWriteTypeReplace = 0,
		eWriteTypeAdd,
		eWriteTypeMax,
		eWriteTypeMin,
	};

	/// maximum columns in any scoreboard is locked at 64.
	const U32 MAX_SERVER_SCOREBOARD_COLUMNS = 64;


	/// All the types of data we can read from scoreboards..
	typedef enum
	{
		Int32 =	0,
		Int64,
		Float64,
		Float32,
		DateTime,
		None = 255,
	}eColumnDataType;


	/// Holds data for a specific single column for a single row
	struct ColumnData
	{
		ColumnData() : m_FieldId(0), m_DataType(Scoreboard::None)
		{}

		// id of the field for which this data represents
		U32	m_FieldId;
		
		// what type of data this object represents (setup m_Data correctly)
		eColumnDataType	m_DataType;

		// different types of data we can write to scoreboards..
		union
		{
			S32			m_S32;
			F32			m_F32;		
			S64			m_S64;		
			F64			m_F64;
#if defined(LIVE_ON_WINDOWS)
			FILETIME	m_DateTime;
#endif
		}m_Data;

		
		/// Setup internal data to be of type Int32
		void SetS32(const U32 fieldId,  const S32 data) {m_FieldId = fieldId; m_DataType = Scoreboard::Int32; m_Data.m_S32 = data;}

		/// Setup internal data to be of type Int64
		void SetS64(const U32 fieldId,  const S64 data) {m_FieldId = fieldId; m_DataType = Scoreboard::Int64; m_Data.m_S64 = data;}

		/// Setup internal data to be of type Float32
		void SetF32(const U32 fieldId,  const F32 data) {m_FieldId = fieldId; m_DataType = Scoreboard::Float32; m_Data.m_F32 = data;}

		/// Setup internal data to be of type Float64
		void SetF64(const U32 fieldId,  const F64 data) {m_FieldId = fieldId; m_DataType = Scoreboard::Float64; m_Data.m_F64 = data;}

#if defined(LIVE_ON_WINDOWS)
		/// Setup internal data to be of type DateTime
		void SetFiletime(const U32 fieldId,  const FILETIME data) {m_FieldId = fieldId; m_DataType = Scoreboard::DateTime; m_Data.m_DateTime = data;}
#endif
	};


	/// Each column on the scoreboards has an identifier, this is just a list of column identifiers used to define which columns are returned in a query.
	/** When using demonware we should always request all columns of a scoreboard since it's not possible to only return a selection like it is on xbox360.
	 */
	typedef Btl::Vector<U32, Btl::VectorParameters_Static<MAX_SERVER_SCOREBOARD_COLUMNS> > ColumnIdentifierList;


	/// When enumerating scoreboards via demonware we need to pass in what format the returned columns are in.
	/** On Xbox360 the data type is returned with the actual data when using demonware it's not.
	 */
	typedef Btl::Vector<eColumnDataType, Btl::VectorParameters_Static<MAX_SERVER_SCOREBOARD_COLUMNS> > ColumnTypeList;


	/// List of columns which contain data, used when enumerating scoreboard data.
	typedef Btl::Vector<ColumnData, Btl::VectorParameters_Static<MAX_SERVER_SCOREBOARD_COLUMNS> > ColumnList;


	/// Structure that holds the full data for a single user / single scoreboard row.
	struct RowData
	{
		RowData() : m_ScoreboardId(0), m_Rank(0), m_Rating(0), m_EntityId(0)
		{}
		SysProfile::BaseProfile		m_Userdata;			// holds name + user identifier (user identifier is not set when using demonware, it will be all zeros).
		bdEntityID					m_EntityId;			// used on demonware to hold the Id of the player or team who this score belongs to.
		U32							m_ScoreboardId;		// this data relates tho this scoreboard identifier.
		U32							m_Rank;				// scoreboard position.
		S64							m_Rating;
		ColumnList					m_Columns;			// column data for this specific player.				
	};

}// end namespace


#endif


