#ifndef _MONOTOMONOSPECTRALMIXER_H_
#define _MONOTOMONOSPECTRALMIXER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MonoToMonoSpectralMixer
////////////////////////////////////////////////////////////////////////////////
//    class MonoToMonoSpectralMixer : public MixerComponent
//    {
//    public:
//        struct Data
//        {
//            //  name
//			StrOffset strName;
//            //  fx
//            ArraySize uEffects;
//            //  inputs
//            ArraySize uInputs;
//        };
//
//        struct State
//        {
//            Audio::Core::Mixer* pMixer;
//        };
//
//        struct InputGain
//        {
//            //  input
//            S32 sInputCom;
//            //  gain
//		    S32 sGainCom;
//		    U32 uGainOut;
//            F32 fGainVal;
//        };
//
//    public:
//        MonoToMonoSpectralMixer(Object* pObject) : MixerComponent(pObject) {}
//
//        void* Init(void* pData);
//        U32 ContextSize() { return sizeof(State); }
//        void InitContext(Instance* pInstance);
//        void ReleaseContext(Instance* pInstance);
//        void Update(Instance* pInstance);
//
//        const char* Type() const { return "MonoToMonoSpectralMixer"; }
//
//		const U32 Flags() { return StaticFlags(); }
//		static U32 StaticFlags() { return (flagUpdater | flagMono | flagSpectralAudioOutput); }
//
//		static U32 Size() { return sizeof(MonoToMonoSpectralMixer); }
//		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uEffects * sizeof(U32) + ((Data*)pData)->uInputs * sizeof(InputGain); }
//
//    };
}

#endif // _MONOTOMONOSPECTRALMIXER_H_

