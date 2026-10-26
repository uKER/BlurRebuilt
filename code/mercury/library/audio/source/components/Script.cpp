#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Script::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	m_uCodeOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uCodeOffset + pInitData->uLength * sizeof(U32);

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Script::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uPC = 0;
    pState->uStack = 0;
    pState->bEqual = false;
    pState->bGreaterThan = false;
    pState->bLessThan = false;
	pState->bActive = false;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Script::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.Script");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pStateIn = (State*)pInstance->GetStateData(m_uStateOffset);
	State* pStateOut = (State*)pInstance->GetNewStateData(m_uStateOffset);

	//	if active
	if( pStateIn->bActive )
	{
		while( DoInstruction( pInstance ) );

		pStateOut->bActive = false;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Script::DoMessage(Instance* pInstance, U32 uMessage)
{
#if defined (AUDIO_ENABLE_PROFILING)
	//Profiler::Instance().PostWarningMessage(pInstance, Profiler::eUnknownWarning);
#endif

	switch (uMessage)
    {
    case ExecuteMsg:
        Execute(pInstance);
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
void Audio::Script::Execute(Instance* pInstance)
{
	//while (DoInstruction(pInstance));

	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	pState->bActive = true;
}

////////////////////////////////////////////////////////////////////////////////
//	DoInstruction
////////////////////////////////////////////////////////////////////////////////
bool Audio::Script::DoInstruction(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U32* auCode = (U32*)m_pObject->GetInitData(m_uCodeOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

	if (pState->uPC >= pInitData->uLength)
	{
//		Stop(pInstance);
		Reset(pStateData);

		return false;
	}

    U32 uInstruction = auCode[pState->uPC++];

    switch (uInstruction)
    {
    case OpBreak:
        {
            Assert(0);
            Reset(pStateData);
			return false;
        }
        break;

    case OpSend:
        {
            U32 uSendCom = auCode[pState->uPC++] >> 16;

			Assert(uSendCom != (U32)-1);
            Assert(uSendCom < m_pObject->GetNumComponents());

            U32 uSendMsg = auCode[pState->uPC++];

#ifdef PLATFORM_SPU
//			U32 u32NumMessages = System::Get()->GetComponentNumMessages(m_pObject->GetComponentTypeId(uSendCom));
#else
            Component* pCom = m_pObject->GetComponent(uSendCom);
			U32 u32NumMessages = pCom->GetNumMessages();

            Assert(uSendMsg < u32NumMessages);
#endif

			//m_pObject->DoComponentMessage(uSendCom, pInstance, uSendMsg);
//            pCom->DoMessage(pInstance, uSendMsg);

			Component::Event ev;
			ev.sCom = uSendCom;
			ev.uMsg = uSendMsg;
			m_pObject->GetComponent(uSendCom)->DoEvent(pInstance, ev);
        }
        break;

	case OpPush:
		{
			U32 uPushCom = auCode[pState->uPC]  >> 16;
			U32 uPushOut = auCode[pState->uPC++] & 0x0000ffff;

			Assert(uPushCom != (U32)-1);
            Assert(uPushCom < m_pObject->GetNumComponents());

			pState->aStack[pState->uStack] = m_pObject->GetComponentValue(uPushCom, pInstance, uPushOut);

			pState->uStack++;
		}
		break;

	case OpPushFloat:
		{
			F32 f32PushVal = *((F32*)(&(auCode[pState->uPC++])));

			pState->aStack[pState->uStack] = f32PushVal;

			pState->uStack++;
		}
		break;

	case OpPull:
		{
			U32 uPullCom = auCode[pState->uPC++] >> 16;

			Assert(uPullCom != (U32)-1);
            Assert(uPullCom < m_pObject->GetNumComponents() && "Attempting to Pull data from empty stack!");


			pState->uStack--;

			Assert (pState->uStack != (U32)-1);
			
			#ifndef CFG_GOLD
			if (pState->uStack == (U32)-1)
			{
				COMPONENT_WARN("Attempting to Pull data from empty stack! Resetting stack.");
				pState->uStack = 0;
			}
			#endif

			Assert ((m_pObject->GetComponentFlags(uPullCom) & flagVariable) != 0);

			m_pObject->SetComponentValue(uPullCom, pInstance, pState->aStack[pState->uStack]);
		}
		break;

	case OpDiscard:
		{
			pState->uStack--;

			Assert (pState->uStack != (U32)-1);

			#ifndef CFG_GOLD
			if (pState->uStack == (U32)-1)
			{
				COMPONENT_WARN("Attempting to Discard data from empty stack! Resetting stack.");
				pState->uStack = 0;
			}
			#endif
		};
		break;

	case OpAdd:
		{
            Assert(pState->uStack >= 1);

            U32 uAddCom = auCode[pState->uPC] >> 16;
			U32 uAddOut = auCode[pState->uPC++] & 0x0000ffff;

            Assert(uAddCom != (U32)-1);
            Assert(uAddCom < m_pObject->GetNumComponents());

			pState->aStack[pState->uStack - 1] += m_pObject->GetComponentValue(uAddCom, pInstance, uAddOut);
		}
		break;

	case OpSubtract:
		{
            Assert(pState->uStack >= 1);

            U32 uSubCom = auCode[pState->uPC] >> 16;
			U32 uSubOut = auCode[pState->uPC++] & 0x0000ffff;

            Assert(uSubCom != (U32)-1);
            Assert(uSubCom < m_pObject->GetNumComponents());

			pState->aStack[pState->uStack - 1] -= m_pObject->GetComponentValue(uSubCom, pInstance, uSubOut);
		}
		break;

	case OpMultiply:
		{
            Assert(pState->uStack >= 1);

            U32 uMulCom = auCode[pState->uPC];
			U32 uMulOut = auCode[pState->uPC++] & 0x0000ffff;

            Assert(uMulCom != (U32)-1);
            Assert(uMulCom < m_pObject->GetNumComponents());

			pState->aStack[pState->uStack - 1] *= m_pObject->GetComponentValue(uMulCom, pInstance, uMulOut);
		}
		break;

	case OpDivide:
		{
            Assert(pState->uStack >= 1);
            Assert(m_pObject->GetComponentValue(auCode[pState->uPC], pInstance) >= 0.f);
			
            U32 uDivCom = auCode[pState->uPC];
			U32 uDivOut = auCode[pState->uPC++] & 0x0000ffff;

            Assert(uDivCom != (U32)-1);
            Assert(uDivCom < m_pObject->GetNumComponents());

            pState->aStack[pState->uStack - 1] /= m_pObject->GetComponentValue(uDivCom, pInstance, uDivOut);
		}
		break;

	case OpAddFloat:
		{
            Assert(pState->uStack >= 1);

			pState->aStack[pState->uStack - 1] += *((F32*)(&(auCode[pState->uPC++])));
		}
		break;

	case OpSubtractFloat:
		{
            Assert(pState->uStack >= 1);

			pState->aStack[pState->uStack - 1] -= *((F32*)(&(auCode[pState->uPC++])));
		}
		break;

	case OpMultiplyFloat:
		{
            Assert(pState->uStack >= 1);

			pState->aStack[pState->uStack - 1] *= *((F32*)(&(auCode[pState->uPC++])));
		}
		break;

	case OpDivideFloat:
		{
            Assert(pState->uStack >= 1);
            Assert((F32)auCode[pState->uPC++] >= 0.f);

            pState->aStack[pState->uStack - 1] /= *((F32*)(&(auCode[pState->uPC++])));
		}
		break;

    case OpCompare:
        {
            Assert(pState->uStack >= 2);

            F32 fFirst = pState->aStack[pState->uStack - 2];
            F32 fSecond = pState->aStack[pState->uStack - 1];

            pState->bEqual = (fabs(fFirst - fSecond) < Maths::fEpsilon) ? true : false;
            pState->bLessThan = (fFirst < fSecond) ? true : false;
            pState->bGreaterThan = (fFirst > fSecond) ? true : false;
        }
        break;

	case OpCompareParams:
		{
            U32 uCompCom = auCode[pState->uPC] >> 16;
			U32 uCompOut = auCode[pState->uPC++] & 0x0000ffff;

            Assert(uCompCom != (U32)-1);
            Assert(uCompCom < m_pObject->GetNumComponents());

			F32 fFirst = m_pObject->GetComponentValue(uCompCom, pInstance, uCompOut);

			F32 fSecond = *((F32*)(&(auCode[pState->uPC++])));

            pState->bEqual = (fabs(fFirst - fSecond) < Maths::fEpsilon) ? true : false;
            pState->bLessThan = (fFirst < fSecond) ? true : false;
            pState->bGreaterThan = (fFirst > fSecond) ? true : false;
		}
		break;

    case OpJump:
        {
			Assert(pState->uPC < pInitData->uLength);

//            pState->uPC += m_pwCode[pState->uPC++] - 2; //  ..offset from jump address
            pState->uPC = auCode[pState->uPC]; //  ..offset from start of script
        }
        break;

    case OpJumpEqual:
        {
			Assert(pState->uPC < pInitData->uLength);

            if (pState->bEqual)
                pState->uPC = auCode[pState->uPC];
            else
                pState->uPC++;
        }
        break;

    case OpJumpNotEqual:
        {
			Assert(pState->uPC < pInitData->uLength);

            if (!pState->bEqual)
                pState->uPC = auCode[pState->uPC];
            else
                pState->uPC++;
        }
        break;

    case OpJumpLessThan:
        {
			Assert(pState->uPC < pInitData->uLength);

            if (pState->bLessThan)
                pState->uPC = auCode[pState->uPC];
            else
                pState->uPC++;
        }
        break;

    case OpJumpNotLessThan:
        {
			Assert(pState->uPC < pInitData->uLength);

            if (!pState->bLessThan)
                pState->uPC = auCode[pState->uPC];
            else
                pState->uPC++;
        }
        break;

    case OpJumpGreaterThan:
        {
			Assert(pState->uPC < pInitData->uLength);

            if (pState->bGreaterThan)
                pState->uPC = auCode[pState->uPC];
            else
                pState->uPC++;
        }
        break;

    case OpJumpNotGreaterThan:
        {
			Assert(pState->uPC < pInitData->uLength);

            if (!pState->bGreaterThan)
                pState->uPC = auCode[pState->uPC];
            else
                pState->uPC++;
        }
        break;

    case OpExecute:
        {
            U32 uCom = auCode[pState->uPC++] >> 16;

            Assert((m_pObject->GetComponentFlags(uCom) & Component::flagEvent) > 0);

            m_pObject->ExecuteComponent(uCom, pInstance);
        }
        break;

    case OpExit:
        {
//            Stop(pInstance);
            Reset(pStateData);
			return false;
        }
        break;

	case OpDestruct:
		{
			// TODO: implement this
//            Stop(pInstance);
            Reset(pStateData);
			return false;
		}
		break;

    default:
        Assert(0);
		return false;
        break;
    }

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
////	Stop
//////////////////////////////////////////////////////////////////////////////////
//void Audio::Script::Stop(Instance* pInstance)
//{
//    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
//
//    pState->bActive = false;
//}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Script::Reset(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

    pState->uPC = 0;
    pState->uStack = 0;
    pState->bEqual = false;
    pState->bGreaterThan = false;
    pState->bLessThan = false;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

