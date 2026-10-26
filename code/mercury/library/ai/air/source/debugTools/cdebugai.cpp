#include "stdafx.h"
#include <debugTools/cdebugai.h>

#include <system/serialise.h>
#include <system/serialise/TextSerialise.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{
	CDebugAI::CDebugAI()
	{
		m_pIData = 0;
		m_pIVehicleState = 0;
		m_pVehState = 0;
		m_pIGameServerTime = 0;
		m_IParams = 0;
		m_ok = false;
		m_lastTime = 0.0f;
	}

	CDebugAI::~CDebugAI()
	{

	}

	void CDebugAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_DEBUG_AI, CID_DEBUG_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_DEBUG_AI, CDebugAI::CreateMe, CDebugAI::DestroyMe, CHash("DebugAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DEBUG_AI, MT_POST_INIT_AI_SYSTEM);

	}

	IComponent *CDebugAI::CreateMe()
	{
		return new CDebugAI;
	}

	bool CDebugAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CDebugAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CDebugAI::Deinit(void)
	{
		
	}

	EMessageResult CDebugAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

			case MT_POST_INIT_AI_SYSTEM:
			{
				CachedInterfaces();
				Configure();
			}
			break;

			default:
				break;
		}

		return msgRet;
	}

	EComponentTypeId CDebugAI::GetComponentTypeId(void)
	{
		return CID_DEBUG_AI;
	}

	void CDebugAI::CachedInterfaces()
	{
		// get the pointer to a IData

		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		m_pIVehicleState = static_cast<IVehicleState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_STATE));
		if (m_pIVehicleState)
		{
			m_pVehState = m_pIVehicleState->GetState();
		}

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));
	
		m_IParams = static_cast<IParamsAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PARAMS_AI));
	
	}


	void CDebugAI::Configure()
	{
#if 0
		if (m_pIData && m_pIVehicleState && m_pVehState && m_IParams)
		{
			std::string n = m_pIData->GetObjectId().GetHashName();
			m_filename = std::string("GAME:\\dontship\\DebugAI\\"); 
			// Character template name is not needed....
			//m_filename += m_IParams->GetDriverData()->m_name.GetHashName();
			//m_filename += "_";
			m_filename += n;
			m_filename += ".csv";

			Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(m_filename.c_str(), Sys::E_FileAccess_Write);
		
			if (pFile)
			{
				m_ok = true;	
				delete pFile;
			}
		}
#else
		m_ok = true;
#endif
	}

	
	void CDebugAI::Update()
	{
#if 0
		if (m_ok && m_pIData && m_pIVehicleState && m_pVehState && m_IParams && m_pIGameServerTime)
		{
			CaptureData();

			if (m_pIGameServerTime->GetTime() > m_lastTime)
			{
				Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(m_filename.c_str(), Sys::E_FileAccess_Append);
				if (pFile)
				{
					Sys::TextSerialiser serial(*pFile);

					serial << m_currentData.m_Position.X() << "," << m_currentData.m_Position.Y() << "," << m_currentData.m_Position.Z() << "," ;
					serial << m_currentData.m_Zaxis.X() << "," << m_currentData.m_Zaxis.Y() << "," << m_currentData.m_Zaxis.Z() << "," ;

					serial << m_currentData.m_angularVelocity.X() << "," << m_currentData.m_angularVelocity.Y() << "," << m_currentData.m_angularVelocity.Z() << "," ;
					serial << m_currentData.m_Velocity.X() << "," << m_currentData.m_Velocity.Y() << "," << m_currentData.m_Velocity.Z() << "," ;
					serial << m_currentData.m_Zaxis.X() << "," << m_currentData.m_Zaxis.Y() << "," << m_currentData.m_Zaxis.Z() << "," ;

					serial << m_currentData.m_Speed << "," ;
					serial << m_currentData.m_accel << "," ;
					serial << m_currentData.m_brake << "," ;
					serial << m_currentData.m_steering;
				
					serial << "\n";

					delete pFile;

					const float timeStep = 1.0f / 10.0f;
					m_lastTime = m_pIGameServerTime->GetTime() + timeStep;
				}
			}
		}
#endif
	}


	void CDebugAI::CaptureData()
	{
		m_currentData.m_Position = m_pVehState->GetPosition();
		m_currentData.m_Zaxis = m_pVehState->GetZaxis();
		m_currentData.m_angularVelocity = m_pVehState->GetAngularVelocity();
		m_currentData.m_Velocity = m_pVehState->GetVelocity();
		m_currentData.m_Speed = m_pVehState->GetSpeedMPH();
		m_currentData.m_accel = m_pVehState->GetAccel();
		m_currentData.m_brake = m_pVehState->GetBrake();
		m_currentData.m_steering = m_pVehState->GetSteering();
	}

}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


