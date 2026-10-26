#ifndef _EVENTIN_H_
#define _EVENTIN_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	ActionIn
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_SPU
    class ActionIn : public Component
#else
    class ActionIn : public Component
#endif
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
			Event evTrigger;
        };

		struct State
		{
            bool bActive;
		};

        struct Packet
        {
            ActionIn* pThis;
            U8* pStateData;
        };

    public:
        ActionIn(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);

        const char* Type() const { return "ActionIn"; }

        const U32 Flags();
#endif	//	#ifndef PLATFORM_SPU
        void Update(Instance* pInstance);
		static U32 StaticFlags() { return flagUpdater | flagActionIn | flagInputConnector; }

		static U32 Size() { return sizeof(ActionIn); }
        static U32 DataSize(void* pData) { return sizeof(Data); }

        //void Stop(Instance* pInstance);
        //void Reset(U8* pStateData);

        //  Queue methods
        bool DoExecute(Instance* pInstance);
		bool TriggerFromScript(Instance* pInstance);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->evTrigger.sCom = -1;
			pData->evTrigger.uMsg = 0;
        }

        static bool ValidateData(Data* pData)
        {
			return (pData->evTrigger.sCom != -1);
        }
		
		#ifdef ENABLE_DEBUG_EXTENSIONS
		const char* Name() { return Component::Name(); }
		#endif
    private:
    };
}

#endif // _EVENTIN_H_
