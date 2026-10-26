#ifndef NETMESSAGEENUMS_H
#define NETMESSAGEENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>
#include <misc_shared/Messaging/MessageBase.h>

/////////////////////////////////////////////////////////////////////////////
// Class       : netMessage
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 15 July 2008
// Description : Base message class for net messages (only local messages).
//               need a confirm flag for popups. 
//               this class will be macroed into several versions, 
//               depending on number of parameters required for the message.
/////////////////////////////////////////////////////////////////////////////
template <class MessageClass, Messaging::TMsgType TypeID, class BaseClass = Messaging::C_MessageBase>
class netMessage: public Messaging::C_MessageDerived<MessageClass, TypeID, BaseClass>
{
public:
	netMessage() { m_confirmed = false; }
	bool m_confirmed;
};

#define NET_DECLARE_MESSAGE_BEGIN(FCC)\
	enum { E_Msg_None	= FOURCC_HI20(FCC) };\
	enum { E_Msg_Begin	= E_Msg_None+1 };

#define NET_DECLARE_MESSAGE_END(COUNT)\
	enum { E_Msg_Count = COUNT };\
	enum { E_Msg_End = E_Msg_Begin + E_Msg_Count }; 

#if !defined( CFG_GOLD )
	#define NET_DECLARE_MESSAGE0(id, NAME)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME()\
				{}\
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				static void Register() \
				{\
					Messaging::RegisterDebugInfo(StaticType(), "NetMessage::E_Msg_"#NAME); \
				}\
			 };

	#define NET_DECLARE_MESSAGE1(id, NAME, VALUETYPE1)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1)\
				: m_value1(value1) {}\
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				static void Register() \
				{\
					Messaging::RegisterDebugInfo(StaticType(), "NetMessage::E_Msg_"#NAME); \
				}\
				VALUETYPE1 m_value1;\
			};

	#define NET_DECLARE_MESSAGE2(id, NAME, VALUETYPE1, VALUETYPE2)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1, const VALUETYPE2& value2)\
				: m_value1(value1)\
				, m_value2(value2)\
				{} \
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				static void Register() \
				{\
					Messaging::RegisterDebugInfo(StaticType(), "NetMessage::E_Msg_"#NAME); \
				}\
				VALUETYPE1 m_value1;\
				VALUETYPE2 m_value2;\
			};

	#define NET_DECLARE_MESSAGE3(id, NAME, VALUETYPE1, VALUETYPE2, VALUETYPE3)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1, const VALUETYPE2& value2, const VALUETYPE3& value3)\
				: m_value1(value1)\
				, m_value2(value2)\
				, m_value3(value3)\
				{} \
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				static void Register() \
				{\
					Messaging::RegisterMessageDebugInfo(StaticType(), "NetMessage::E_Msg_"#NAME); \
				}\
				VALUETYPE1 m_value1;\
				VALUETYPE2 m_value2;\
				VALUETYPE3 m_value3;\
			};

	#define NET_DECLARE_MESSAGE4(id, NAME, VALUETYPE1, VALUETYPE2, VALUETYPE3, VALUETYPE4)\
			static const int E_Msg_##NAME = E_Msg_Begin + id;\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1, const VALUETYPE2& value2, const VALUETYPE3& value3, const VALUETYPE3& value4)\
				: m_value1(value1)\
				, m_value2(value2)\
				, m_value3(value3)\
				, m_value4(value4)\
				{} \
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				static void Register() \
				{\
					Messaging::RegisterDebugInfo(StaticType(), "NetMessage::E_Msg_"#NAME); \
				}\
				VALUETYPE1 m_value1;\
				VALUETYPE2 m_value2;\
				VALUETYPE3 m_value3;\
				VALUETYPE4 m_value4;\
			};
#else

	#define NET_DECLARE_MESSAGE0(id, NAME)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME()\
				{}\
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
			 };

	#define NET_DECLARE_MESSAGE1(id, NAME, VALUETYPE1)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1)\
				: m_value1(value1) {}\
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				VALUETYPE1 m_value1;\
			};

	#define NET_DECLARE_MESSAGE2(id, NAME, VALUETYPE1, VALUETYPE2)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1, const VALUETYPE2& value2)\
				: m_value1(value1)\
				, m_value2(value2)\
				{} \
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				VALUETYPE1 m_value1;\
				VALUETYPE2 m_value2;\
			};

	#define NET_DECLARE_MESSAGE3(id, NAME, VALUETYPE1, VALUETYPE2, VALUETYPE3)\
			enum { E_Msg_##NAME = E_Msg_Begin + id };\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1, const VALUETYPE2& value2, const VALUETYPE3& value3)\
				: m_value1(value1)\
				, m_value2(value2)\
				, m_value3(value3)\
				{} \
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				VALUETYPE1 m_value1;\
				VALUETYPE2 m_value2;\
				VALUETYPE3 m_value3;\
			};

	#define NET_DECLARE_MESSAGE4(id, NAME, VALUETYPE1, VALUETYPE2, VALUETYPE3, VALUETYPE4)\
			static const int E_Msg_##NAME = E_Msg_Begin + id;\
			class C_Msg_##NAME: public netMessage<C_Msg_##NAME, E_Msg_##NAME>\
			{ \
			public:\
				C_Msg_##NAME() {}\
				C_Msg_##NAME(const VALUETYPE1& value1, const VALUETYPE2& value2, const VALUETYPE3& value3, const VALUETYPE3& value4)\
				: m_value1(value1)\
				, m_value2(value2)\
				, m_value3(value3)\
				, m_value4(value4)\
				{} \
				static Messaging::TMsgType StaticType() {return E_Msg_##NAME; }\
				VALUETYPE1 m_value1;\
				VALUETYPE2 m_value2;\
				VALUETYPE3 m_value3;\
				VALUETYPE4 m_value4;\
			};
#endif // ( CFG_GOLD )

#endif //NETMESSAGEENUMS_H