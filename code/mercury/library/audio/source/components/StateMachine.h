#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	StateMachine
////////////////////////////////////////////////////////////////////////////////
    class StateMachine : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			AudioInt sInitialState;
			Control ctrlNextState;
            ArraySize uNumStates;
        };

        struct State
        {
            S32 sCurrState;
        };

        struct StateItem
        {
			Event evEnter;
			Event evExit;
        };

        enum Controls
        {
            ValueCtrl
        };

        enum Messages
        {
            IncrementMsg,
            DecrementMsg,
            ChangeMsg,
            ResetMsg,

            NumMessages
        };

    public:
        StateMachine(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "StateMachine"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(StateMachine); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumStates * sizeof(StateItem); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->sInitialState = -1;
			pData->ctrlNextState.sCom = -1;
			pData->ctrlNextState.uOut = 0;
            pData->uNumStates = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->ctrlNextState.sCom == -1)
                return false;
            else if (pData->uNumStates < 2)
                return false;
            else
                return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Value") == 0)
                return 0;
            else
                return -1;
        }

    private:
        inline void Increment( Instance* pInstance );
        inline void Decrement( Instance* pInstance );
        inline void Change( Instance* pInstance );
        inline void Reset( Instance* pInstance );

        inline void ChangeState( Instance* pInstance, S32 sNextState );

    private:
        ixInit m_uStateItemOffset;
    };
}

#endif // _STATE_MACHINE_H_
