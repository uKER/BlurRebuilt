
#include "stdafx.h"
#include "./netSessionState.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netSessionState::netSessionState()
{
	m_flags		= -1;
	m_titleId	= -1;
	
	memset(&m_migrationInfo, 0, sizeof(m_migrationInfo));
	memset(&m_arbitrationInfo, 0, sizeof(m_arbitrationInfo));
	memset(&m_securityInfo, 0, sizeof(m_securityInfo));
}

netSessionState::~netSessionState()
{}

NetPlay::C_Session::SessionUser* netSessionState::findUser(const NetPlay::C_Session::SessionUser& user) 
{ 
	return findUser(user.m_User); 
}

NetPlay::SessionParameter* netSessionState::findParameter(const NetPlay::SessionParameter& param)
{
	for(NetPlay::SessionParameterList::Iterator it = m_parameters.Begin(); it != m_parameters.End(); ++it)
	{
		// comparaison results
		int result = compareParameters(&(*it), &param);
		
		// ignore value changes.
		result &= ~(RESULT_FLAG_MISMATCH_VALUE);

		// same data id, type, and value format.
		if(result == 0)
			return &(*it);
	}
	return NULL;
}

NetPlay::C_Session::SessionUser* netSessionState::findUser(const SysProfile::UserIdentifier& user)
{
	netUserId userId = netPlatform::Core::instance()->getUserId(user);

	for(NetPlay::SessionUserList::Iterator it = m_players.Begin(); it != m_players.End(); ++it)
	{
		if(netPlatform::Core::instance()->getUserId(it->m_User) == userId)
		{
			return &(*it);
		}
	}
	return NULL;

}

bool netSessionState::addUser(const NetPlay::C_Session::SessionUser& user)
{
	if(findUser(user.m_User))
	{
		LOGFATAL(netDebug::g_Log, "<netSessionState> user(%llu) already in list.", netPlatform::Core::instance()->getUserId(user.m_User));
		return false;
	}
	
	m_players.PushBack(user);
	return true;
}


bool netSessionState::removeUser(const NetPlay::C_Session::SessionUser& user)
{
	NetPlay::C_Session::SessionUser* slot = findUser(user);

	if(slot == NULL)
	{
		LOGWARN(netDebug::g_Log, "<netSessionState> user(%llu) not found in list.", netPlatform::Core::instance()->getUserId(user.m_User));
		return false;
	}

	*slot = m_players.Back();
	m_players.PopBack();
	return true;
}

int netSessionState::compareParameters(const NetPlay::SessionParameter* a, const NetPlay::SessionParameter* b)
{
	int flags = 0;
	if(a->m_ParameterType != b->m_ParameterType)
	{
		flags |= RESULT_FLAG_MISMATCH_TYPE;
		return flags;
	}
	
	switch(a->m_ParameterType)
	{
	default: 
		{
			assert(false); 
			break;
		}
	case NetPlay::SessionParameter::eTypeContext:
		{
			// same id
			if(a->m_Data.m_ContextData.m_Id != b->m_Data.m_ContextData.m_Id)
			{
				flags |= RESULT_FLAG_MISMATCH_ID;
				return flags;
			}
		
			// same value
			if(a->m_Data.m_ContextData.m_Value != b->m_Data.m_ContextData.m_Value)
			{
				flags |= RESULT_FLAG_MISMATCH_VALUE;
				return flags;
			}
			return flags;
		}
	case NetPlay::SessionParameter::eTypeProperty:
		{
			// same id
			if(a->m_Data.m_PropertyData.m_Id != b->m_Data.m_PropertyData.m_Id)
			{
				flags |= RESULT_FLAG_MISMATCH_ID;
				return flags;
			}			
		
			if(a->m_Data.m_PropertyData.m_ValueType != b->m_Data.m_PropertyData.m_ValueType)
			{
				flags |= RESULT_FLAG_MISMATCH_VALUE_TYPE;
				return flags;
			}			
		
			// try verify the property value
			switch(a->m_Data.m_PropertyData.m_ValueType)
			{
			case NetPlay::SessionParameter::Int32:
				if(a->m_Data.m_PropertyData.m_Value.m_S32 != b->m_Data.m_PropertyData.m_Value.m_S32)
				{
					flags |= RESULT_FLAG_MISMATCH_VALUE;
					return flags;
				}
				break;
			case NetPlay::SessionParameter::Int64:
				if(a->m_Data.m_PropertyData.m_Value.m_S64 != b->m_Data.m_PropertyData.m_Value.m_S64)
				{
					flags |= RESULT_FLAG_MISMATCH_VALUE;
					return flags;
				}
				break;
			case NetPlay::SessionParameter::Float64:
				if(a->m_Data.m_PropertyData.m_Value.m_F64 != b->m_Data.m_PropertyData.m_Value.m_F64)
				{
					flags |= RESULT_FLAG_MISMATCH_VALUE;
					return flags;
				}
				break;
			case NetPlay::SessionParameter::Float32:
				if(a->m_Data.m_PropertyData.m_Value.m_F32 != b->m_Data.m_PropertyData.m_Value.m_F32)
				{
					flags |= RESULT_FLAG_MISMATCH_VALUE;
					return flags;
				}
				break;
			case NetPlay::SessionParameter::None:
				{
					return flags;
				}
				break;
			default:
				assert(false);
				flags |= RESULT_FLAG_MISMATCH_VALUE;
				return flags;
			}
		}
	}
	return flags;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

