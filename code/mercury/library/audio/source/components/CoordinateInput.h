#ifndef _COORDINATEINPUT_H_
#define _COORDINATEINPUT_H_

namespace Audio
{
    class CoordinateOutput;
////////////////////////////////////////////////////////////////////////////////
//	CoordinateInput
////////////////////////////////////////////////////////////////////////////////
    class CoordinateInput : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
        };

        struct State
        {
            Instance* pInstanceOut;
            Maths::Vector vCoords;
			F32 fMagnitude;
            Com uCoordinateOutput;
        };

        enum Controls
        {
            MagnitudeCtrl,
            XCtrl,
            YCtrl,
            ZCtrl,
        };

    public:
        CoordinateInput(Object* pObject);
        ~CoordinateInput();

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
		void Coords(Instance* pInstance, Maths::Vector* pVector);

        const char* Type() const { return "CoordinateInput"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagVector | flagInputConnector | flagVectorConnector; }

        static U32 Size() { return sizeof(CoordinateInput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        void SetOutput( Instance* pThisInstance, Instance* pThatInstance, Audio::Com uCoordinateOutput );

        inline State* GetContext(U8* pStateData) { return (State*)&pStateData[m_uStateOffset]; }

		void ProcessConnection(Instance* pInstance);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Magnitude") == 0)
                return 0;
            else if (strcmp(pchControl, "X") == 0)
                return 1;
            else if (strcmp(pchControl, "Y") == 0)
                return 2;
            else if (strcmp(pchControl, "Z") == 0)
                return 3;
            else
                return -1;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance(Instance* pThisInstance);
		Com GetOtherComponent(Instance* pThisInstance);
#endif

    private:
    };
}

#endif // _COORDINATEINPUT_H_
