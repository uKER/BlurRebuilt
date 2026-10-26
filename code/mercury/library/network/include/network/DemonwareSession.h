// ***************************************************************
//  DemonwareSession   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Demonware session creation / enumeration handling.
// ***************************************************************

#ifndef NETPLAY_DEMONWARE_SESSION_H
#define NETPLAY_DEMONWARE_SESSION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/SessionParameter.h>
#include <Btl/Containers/vector.h>
#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>
#include <bdLobby/bdMatchMaking/bdSessionParams.h>
#include <bdCore/bdContainers/bdBitBuffer.h>
#include <bdSocket/bdQoS/bdQoSProbeListener.h>


// define the max size of Qos data that we can send when using demonware.
#define BIZ_MAX_QOS_DATA_SIZE (512)


namespace NetPlay
{
	/// This is a class for creating and searching for online demonware sessions.
	/** The game does not need to create an instance of this, instead it's used internally to the C_Session class which provides a cross platform way of 
	 *	creating and managing online sessions.
	 */
	class C_DemonwareSession : public bdMatchMakingInfo
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		explicit C_DemonwareSession() 
		{Flush();}

		virtual ~C_DemonwareSession()
		{}

		// this should be called when creating a session, not enumerating..
		virtual bdUInt sizeOf()
		{
			bdUInt size = bdMatchMakingInfo::sizeOf();

			// we always should write a session key.
			// imp schema on server should have a data blob to store this as the first parameter.
			size += BD_SECURITY_KEY_LENGTH;

			for (U32 i=0; i <m_NumAttributesInUse; i++)
			{
				const SessionParameter& p = m_Attributes[i];
				if (SessionParameter::eTypeProperty == p.m_ParameterType)
				{
					switch (p.m_Data.m_PropertyData.m_ValueType)
					{
					case SessionParameter::Int32:
						size += sizeof(SessionParameter::Int32);
						break;

					case SessionParameter::Int64:
						size += sizeof(SessionParameter::Int64);
						break;

					case SessionParameter::Float64:
						size += sizeof(SessionParameter::Float64);
						break;

					case SessionParameter::Float32:
						size += sizeof(SessionParameter::Float32);
						break;

					default:
						assert(false);
						break;
					}
				}
				else
				{
					assert(false);
				}
			}

			return size;
		}

		// used when creating sessions
		virtual void serialize(bdByteBuffer &buffer) const
		{
			bdMatchMakingInfo::serialize(buffer);

			// write session key data blob.
			buffer.writeBlob(m_SecurityKey, BD_SECURITY_KEY_LENGTH); 

			for (U32 i = 0; i<m_NumAttributesInUse; i++)
			{
				const SessionParameter& p = m_Attributes[i];
				if (SessionParameter::eTypeProperty == p.m_ParameterType)
				{
					switch (p.m_Data.m_PropertyData.m_ValueType)
					{
					case SessionParameter::Int32:
						buffer.writeInt32(p.m_Data.m_PropertyData.m_Value.m_S32);
						break;

					case SessionParameter::Int64:
						buffer.writeInt64(p.m_Data.m_PropertyData.m_Value.m_S64);
						break;

					case SessionParameter::Float64:
						buffer.writeFloat64(p.m_Data.m_PropertyData.m_Value.m_F64);
						break;

					case SessionParameter::Float32:
						buffer.writeFloat32(p.m_Data.m_PropertyData.m_Value.m_F32);
						break;

					default:
						assert(false);
						break;
					}
				}
				else
				{
					// don't support contexts on anything other than xbox360
					assert(false);
				}
			}
		}

		void Flush(void)
		{
			memset(m_SecurityKey, 0, BD_SECURITY_KEY_LENGTH);
			SessionParameter blank;
			m_NumAttributesInUse = 0;
			for (S32 i=0; i<m_Attributes.Size(); i++)
				m_Attributes[i] = blank;

			m_QoSValid = false;
			m_QoSDataSize = 0;
			m_Ping = 0.0f;
		}

		// attribute data.
		U32	m_NumAttributesInUse;
		Btl::Vector<SessionParameter, Btl::VectorParameters_Array<BIZ_MAX_DEMONWARE_SESSION_ATTRIBUTES> > m_Attributes;
		
		// security key needs to be manually retrieved in 2.0.
		// it's no longer part of bdMatchMakingInfo.
		U8 m_SecurityKey[BD_SECURITY_KEY_LENGTH];

		// Qos
		bool m_QoSValid;
		float m_Ping;
		U32 m_QoSDataSize;
		U8	m_QoSData[BIZ_MAX_QOS_DATA_SIZE];
	};

	// this is what we supply to demonware when enumerating sessions.
	class C_EnumeratedDemonwareSession : public C_DemonwareSession
	{
	public:
		explicit C_EnumeratedDemonwareSession() 
		{Flush();}

		virtual ~C_EnumeratedDemonwareSession()
		{}

		// this should be called when creating a session, not enumerating..
		virtual bdUInt sizeOf()
		{
			return sizeof(C_EnumeratedDemonwareSession);
		}

		// used when enumerating sessions.
		virtual bdBool deserialize(bdByteBufferRef buffer)
		{
			bool ok = bdMatchMakingInfo::deserialize(buffer);

			// we should always return the security key first.
			bdUInt32 len = BD_SECURITY_KEY_LENGTH;
			ok = ok && buffer->readBlob(m_SecurityKey, len);

			// in demonware 2.0 the server returns all matchmaking attributes, so we need to parse the data stream.
			// although we normally use the QoS data to retrieve session settings as it's more up to date.
			for (U32 i = 0; ok && (i< m_NumAttributesInUse); i++)
			{
				SessionParameter& p = m_Attributes[i];
				if (SessionParameter::eTypeProperty == p.m_ParameterType)
				{
					switch (p.m_Data.m_PropertyData.m_ValueType)
					{
					case SessionParameter::Int32:
						{
							S32 temp = 0;
							ok = ok && buffer->readInt32(temp);
							p.CreatePropertyS32(i, temp);
						}
						break;

					case SessionParameter::Int64:
						{
							S64 temp = 0;
							ok = ok && buffer->readInt64(temp);
							p.CreatePropertyS64(i, temp);
						}
						break;

					case SessionParameter::Float64:
						{
							F64 temp = 0.0f;
							ok = ok && buffer->readFloat64(temp);
							p.CreatePropertyF64(i, temp);
						}
						break;

					case SessionParameter::Float32:
						{
							F32 temp = 0;
							ok = ok && buffer->readFloat32(temp);
							p.CreatePropertyF32(i, temp);
						}
						break;

					default:
						assert(false);
						break;
					}
				}
				else
					ok = false;
			}
			return ok;
		}

		void operator = (const C_EnumeratedDemonwareSession& original)	
		{
			if (this == &original)
				return; // don't copy yourself!

			// copy demonware parameters.
			//--------------------------

			// host address
			m_hostAddrSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
			for (U32 i=0; i< m_hostAddrSize; i++)
				m_hostAddr[i] = original.m_hostAddr[i];

			// security key id
			m_sessionID = original.m_sessionID;

			// player data
			m_numPlayers = original.m_numPlayers;
			m_maxPlayers = original.m_maxPlayers;

			// game type
			m_gameType =  original.m_gameType;

			// copy game parameters.
			// --------------------

			// security key
			for (U32 i=0; i<BD_SECURITY_KEY_LENGTH; i++)
				m_SecurityKey[i] = original.m_SecurityKey[i];

			// copy game parameters.
			m_NumAttributesInUse = original.m_NumAttributesInUse;
			for (S32 i=0; i<BIZ_MAX_DEMONWARE_SESSION_ATTRIBUTES; i++)
				m_Attributes[i] = original.m_Attributes[i];

			// copy qos.
			m_QoSValid		=	original.m_QoSValid;
			m_Ping			=	original.m_Ping;
			m_QoSDataSize	=	original.m_QoSDataSize;
			memcpy (m_QoSData, original.m_QoSData, BIZ_MAX_QOS_DATA_SIZE);
		}
	};


	/// This is a class for searching for online demonware sessions.
	/** The game does not need to create an instance of this, instead it's used internally to the C_SessionEnumerator class which provides a cross platform way of 
	 *	enumerating online sessions.
	 */
	class C_DemonwareQuery : public bdSessionParams
	{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		explicit C_DemonwareQuery() 
		{Flush();}

		virtual ~C_DemonwareQuery()
		{}

		void Flush(void)
		{
			m_NumAttributesInUse = 0;
		}

		virtual void serialize(bdByteBuffer &buffer) const
		{
			bdSessionParams::serialize(buffer);
			
			for (U32 i = 0; i<m_NumAttributesInUse; i++)
			{
				const SessionParameter& p = m_Attributes[i];
				if (SessionParameter::eTypeProperty == p.m_ParameterType)
				{
					switch (p.m_Data.m_PropertyData.m_ValueType)
					{
					case SessionParameter::Int32:
						buffer.writeInt32(p.m_Data.m_PropertyData.m_Value.m_S32);
						break;

					case SessionParameter::Int64:
						buffer.writeInt64(p.m_Data.m_PropertyData.m_Value.m_S64);
						break;

					case SessionParameter::Float64:
						buffer.writeFloat64(p.m_Data.m_PropertyData.m_Value.m_F64);
						break;

					case SessionParameter::Float32:
						buffer.writeFloat32(p.m_Data.m_PropertyData.m_Value.m_F32);
						break;

					default:
						assert(false);
						break;
					}
				}
				else
				{
					// don't support contexts on anything other than xbox360
					assert(false);
				}
			}
		}

		virtual bdUInt sizeOf()
		{
			bdUInt size = bdSessionParams::sizeOf();
			for (U32 i=0; i<m_NumAttributesInUse; i++)
			{
				const SessionParameter& p = m_Attributes[i];
				if (SessionParameter::eTypeProperty == p.m_ParameterType)
				{
					switch (p.m_Data.m_PropertyData.m_ValueType)
					{
					case SessionParameter::Int32:
						size += sizeof(SessionParameter::Int32);
						break;

					case SessionParameter::Int64:
						size += sizeof(SessionParameter::Int64);
						break;

					case SessionParameter::Float64:
						size += sizeof(SessionParameter::Float64);
						break;

					case SessionParameter::Float32:
						size += sizeof(SessionParameter::Float32);
						break;

					default:
						assert(false);
						break;
					}
				}
				else
				{
					assert(false);
				}
			}
			return size;
		}

		U32	m_NumAttributesInUse;
		Btl::Vector<SessionParameter, Btl::VectorParameters_Array<BIZ_MAX_DEMONWARE_SESSION_ATTRIBUTES> > m_Attributes;
	};

	struct DemonWareSessionSortHelper 
	{
		DemonWareSessionSortHelper() : m_SessionQueryIndex(0), m_Ping(0.0f)
		{}
		U32		m_SessionQueryIndex;
		float	m_Ping;
	};
}

#endif
