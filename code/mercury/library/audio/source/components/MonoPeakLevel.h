#ifndef _PeakLevel_H_
#define _PeakLevel_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoPeakLevel
////////////////////////////////////////////////////////////////////////////////
    class MonoPeakLevel : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		};			  

        struct State
        {
			F32 fValue;
		};

		enum Controls
        {
            ValueCtrl,
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

    public:
        MonoPeakLevel(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
        void Update(Instance* pInstance);
		F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage( Instance* pInstance, U32 uMessage );

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "MonoPeakLevel"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagMono | flagTimeEffect); }

        static U32 Size() { return sizeof(MonoPeakLevel); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

        U32 DSPEffectUnit(U8* pStateData, U32 uIndex) { return m_uDSP; }
		U32 DSPEffectType(U8* pStateData, U32 uIndex) { return Core::DSP::Effect::ePeakLevel; }
		
        static void DefaultData(Data* pData)
        {
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

    private:
		ixCore m_uDSP;
    };
}

#endif // _PeakLevel_H_


