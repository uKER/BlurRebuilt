// ***************************************************************
//  SessionParameter   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Online session parameter creation.
// ***************************************************************

#ifndef SESSION_PARAMETER_H
#define SESSION_PARAMETER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NetPlay
{
	/// Defines a single parameter used when creating / searching for online Network sessions.
	struct SessionParameter
	{
		/// 2 types of session parameters Contexts and Properties.
		typedef enum
		{
			eTypeContext = 0,
			eTypeProperty,
		}Type;

		/// All the different data types we can associate with a property
		typedef enum
		{
			Int32	=	0,
			Int64,
			Float64,
			Float32,
			None = 255,
		}ePropertyValueType;

		// data required for the Context property type
		struct Context
		{
			U32	m_Id;			// this comes from the XLAST Schema for Xbox360. On Demonware we use contexts to represent the game_type within bdmatchmakingInfo so any unique value will do.	
			U32	m_Value;		// enumeration value;
		};

		// data required for the property type.
		struct Property
		{
			U32					m_Id;			// this comes from the XLAST schema for Xbox360
			ePropertyValueType	m_ValueType;	// which member within m_Value should we reference.
			union
			{
				S32		m_S32;
				F32		m_F32;		
				S64     m_S64;		
				F64		m_F64;				
			}m_Value;
		};

		// default constructor
		SessionParameter()
		{
			m_ParameterType				= eTypeContext;
			m_Data.m_ContextData.m_Id	= 0;
			m_Data.m_ContextData.m_Value= 0;
		}

		/// Build a context.
		void CreateContext(const U32 Id, const U32 val) {m_ParameterType = eTypeContext; m_Data.m_ContextData.m_Id = Id; m_Data.m_ContextData.m_Value = val;}

		/// Build a 32bit integer property.
		void CreatePropertyS32(const U32 Id, const S32 val) {m_ParameterType = eTypeProperty; m_Data.m_PropertyData.m_Id = Id; m_Data.m_PropertyData.m_ValueType = Int32; m_Data.m_PropertyData.m_Value.m_S32 = val;}

		/// Build a 32bit floating point property.
		void CreatePropertyF32(const U32 Id, const F32 val) {m_ParameterType = eTypeProperty; m_Data.m_PropertyData.m_Id = Id; m_Data.m_PropertyData.m_ValueType = Float32; m_Data.m_PropertyData.m_Value.m_F32 = val;}		

		/// Build a 64bit integer property.
		void CreatePropertyS64(const U32 Id, const S64 val) {m_ParameterType = eTypeProperty; m_Data.m_PropertyData.m_Id = Id; m_Data.m_PropertyData.m_ValueType = Int64; m_Data.m_PropertyData.m_Value.m_S64 = val;}

		/// Build a 64bit floating point property.
		void CreatePropertyF64(const U32 Id, const F64 val) {m_ParameterType = eTypeProperty; m_Data.m_PropertyData.m_Id = Id; m_Data.m_PropertyData.m_ValueType = Float64; m_Data.m_PropertyData.m_Value.m_F64 = val;}		

		// actual structure data
		Type	m_ParameterType;
		union
		{
			Context		m_ContextData;
			Property	m_PropertyData;
		}m_Data;
	};


	// maximum number of attribues than can be in a server query, (create or search).
	#define BIZ_MAX_DEMONWARE_SESSION_ATTRIBUTES	(20)

	/// defines a list of property value types that will will be returned from the server when we search for sessions via demonware 2.0.
	/** we need this so we can correctly pase the returned data stream from the server.
	 */
	typedef Btl::Vector<SessionParameter::ePropertyValueType, Btl::VectorParameters_Static<BIZ_MAX_DEMONWARE_SESSION_ATTRIBUTES> > SesionQueryReturnTypes;
}



#endif


