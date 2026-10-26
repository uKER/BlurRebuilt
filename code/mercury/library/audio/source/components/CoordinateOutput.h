#ifndef _COORDINATEOUTPUT_H_
#define _COORDINATEOUTPUT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	CoordinateOutput
////////////////////////////////////////////////////////////////////////////////
    class CoordinateOutput : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Com sInputCom;
        };

        struct State
        {
            Instance* pInstanceIn;
            Maths::Vector vCoords;
			F32 fMagnitude;
		    Com uCoordinateInput;
        };

        enum Controls
        {
            MagnitudeCtrl,
            XCtrl,
            YCtrl,
            ZCtrl,
        };

    public:
        CoordinateOutput(Object* pObject) : Component(pObject)
        {
        }
        ~CoordinateOutput() {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
		void Coords(Instance* pInstance, Maths::Vector* pVector);

        const char* Type() const { return "CoordinateOutput"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return ( flagVector | flagOutputConnector | flagVectorConnector ); }

        static U32 Size() { return sizeof(CoordinateOutput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        void SetInput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uCoordinateInput);

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
		Instance* GetOtherInstance(Instance* pThisInstance);
		Com GetOtherComponent(Instance* pThisInstance);
#endif

    private:
    };
}

#endif // _COORDINATEOUTPUT_H_
