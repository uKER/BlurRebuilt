#ifndef _SURROUNDLFEAUDIOINPUT_H_
#define _SURROUNDLFEAUDIOINPUT_H_

namespace Audio
{
	namespace Core { class Input; }
////////////////////////////////////////////////////////////////////////////////
//	SurroundLFEAudioInput
////////////////////////////////////////////////////////////////////////////////
    class SurroundLFEAudioInput : public AudioComponent
    {
    public:
        struct Data
        {
			StrOffset strName;
        };

        struct State
        {
			F32* pFrameInput;
			bool bActive;
			bool bMuted;
        };

    public:
        SurroundLFEAudioInput(Object* pObject) : AudioComponent(pObject)
        {
			m_uInput = 0;
        }
        ~SurroundLFEAudioInput() {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void Update(Instance* pInstance);
		void ReleaseContext(Instance* pInstance);

        const char* Type() const { return "SurroundLFEAudioInput"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagReciever | flagSurroundLFE | flagAudioOutput | flagInputConnector | flagAudioConnector); }

        static U32 Size() { return sizeof(SurroundLFEAudioInput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }
		U32 CoreMemory(U32 uOffset);

		const U32 AudioOutType(Instance* pInstance) const
		{
			return Core::eInput;
		}
		const U32 AudioOutOffset(Instance* pInstance) const
		{
			return m_uInput;
		}

		Audio::Core::Input* AudioInput(Instance* pInstance);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance( Instance* pThisInstance );
		Com GetOtherComponent( Instance* pThisInstance );
		bool IsActive( Instance* pInstance );
		void SetMute( Instance* pInstance, bool bMuted );
#endif

	private:
		ixCore m_uInput;
    };
}

#endif // _SURROUNDLFEAUDIOINPUT_H_

