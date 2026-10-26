#ifndef _SURROUNDLFEAUDIOOUTPUT_H_
#define _SURROUNDLFEAUDIOOUTPUT_H_

namespace Audio
{
	namespace Core { class Output; }
	namespace Core { class Input; }
////////////////////////////////////////////////////////////////////////////////
//	SurroundLFEAudioOutput
////////////////////////////////////////////////////////////////////////////////
    class SurroundLFEAudioOutput : public AudioComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
            Com sInputCom;
        };

        struct State
        {
			bool bActive;
			bool bMuted;
        };

    public:
        SurroundLFEAudioOutput(Object* pObject) : AudioComponent(pObject)
        {
			m_uOutput = 0;
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void ReleaseContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        void Generate(Instance* pInstance);

        const char* Type() const { return "SurroundLFEAudioOutput"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagTransmitter | flagSurroundLFE | flagOutputConnector | flagAudioConnector); }

        static U32 Size() { return sizeof(SurroundLFEAudioOutput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

		void OutputTo(Instance* pInstance, Core::Input* pInput);
        void DisconnectFrom(Instance* pInstance, Core::Input* pInput);

		void ConfigureAudio(Instance* pInstance);

		void ProcessConnection(Instance* pInstance);

#ifndef CFG_GOLD
		void DumpProcessFlags( Instance* pInstance );
#endif

		const U32 AudioOutType(Instance* pInstance) const
		{
			return Core::eOutput;
		}
		const U32 AudioOutOffset(Instance* pInstance) const
		{
			return m_uOutput;
		}

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->sInputCom = -1;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->sInputCom == -1)
                return false;
            else
                return true;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		U16 GetNumInputs();
		Com GetInputCom(U32 uIndex);
		bool IsActive(Instance* pInstance);
		void SetMute(Instance* pInstance, bool bMuted);
#endif

	private:
		ixCore m_uOutput;
    };
}

#endif // _SURROUNDLFEAUDIOOUTPUT_H_

