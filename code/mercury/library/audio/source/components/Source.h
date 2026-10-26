#ifndef _SOURCE_H_
#define _SOURCE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
///	\brief This class describes an audio source component.
///
///	A source converts 3D geometric information into mixer controls for panning.
////////////////////////////////////////////////////////////////////////////////
    class Source : public Component
    {
    public:
		///	Source init data structure
        struct Data
        {
			StrOffset strName;
			Option optSize;
            AudioFloat fSourcePositionXVal;
            AudioFloat fSourcePositionYVal;
            AudioFloat fSourcePositionZVal;
            AudioFloat fSourceDirectionXVal;
            AudioFloat fSourceDirectionYVal;
            AudioFloat fSourceDirectionZVal;
            AudioFloat fSourceVelocityXVal;
            AudioFloat fSourceVelocityYVal;
            AudioFloat fSourceVelocityZVal;
            AudioFloat fObjectPositionXVal;
            AudioFloat fObjectPositionYVal;
            AudioFloat fObjectPositionZVal;
            AudioFloat fObjectDirectionXVal;
            AudioFloat fObjectDirectionYVal;
            AudioFloat fObjectDirectionZVal;
            AudioFloat fObjectUpXVal;
            AudioFloat fObjectUpYVal;
            AudioFloat fObjectUpZVal;
            AudioFloat fObjectVelocityXVal;
            AudioFloat fObjectVelocityYVal;
            AudioFloat fObjectVelocityZVal;
            AudioFloat fListenerPositionXVal;
            AudioFloat fListenerPositionYVal;
            AudioFloat fListenerPositionZVal;
            AudioFloat fListenerDirectionXVal;
            AudioFloat fListenerDirectionYVal;
            AudioFloat fListenerDirectionZVal;
            AudioFloat fListenerUpXVal;
            AudioFloat fListenerUpYVal;
            AudioFloat fListenerUpZVal;
            AudioFloat fListenerVelocityXVal;
            AudioFloat fListenerVelocityYVal;
            AudioFloat fListenerVelocityZVal;
            Com sSourcePositionVec;
            Com sSourceDirectionVec;
            Com sSourceVelocityVec;
            Com sObjectPositionVec;
            Com sObjectDirectionVec;
            Com sObjectUpVec;
            Com sObjectVelocityVec;
            Com sListenerPositionVec;
            Com sListenerDirectionVec;
            Com sListenerUpVec;
            Com sListenerVelocityVec;
        };

        struct State
        {
            #if defined( USER_JDENTON )
            Maths::Vector garbage;
            #endif
            
            Maths::Vector vObjectRelativePosition;
            Maths::Vector vObjectRelativeDirection;
            Maths::Vector vObjectRelativeVelocity;

            Maths::Vector vListenerRelativePosition;
            Maths::Vector vListenerRelativeDirection;
            Maths::Vector vListenerRelativeVelocity;

            Maths::Vector vWorldPosition;
            Maths::Vector vWorldDirection;
            Maths::Vector vWorldVelocity;

            Maths::Matrix mObjectOrientation;
            Maths::Matrix mListenerOrientation;
            Maths::Matrix mInvListenerOrientation;

            F32 fAzimuth;
            F32 fElevation;
            F32 fDistance;
            F32 fWidth;
            F32 fVelocity;
            F32 fSourceIncidence;
            F32 fListenerIncidence;
        };

        enum Controls
        {
            AzimuthCtrl,
            ElevationCtrl,
            DistanceCtrl,
            WidthCtrl,
            VelocityCtrl,
            SourceIncidenceCtrl,
            ListenerIncidenceCtrl
        };

    public:
        Source(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Source"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

		static U32 Size() { return sizeof(Source); }
        static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->optSize.ctrl.sCom = -1;
			pData->optSize.ctrl.uOut = 0;
            pData->optSize.fVal = 0.f;
            pData->sSourcePositionVec = -1;
            pData->fSourcePositionXVal = 0.f;
            pData->fSourcePositionYVal = 0.f;
            pData->fSourcePositionZVal = 0.f;
            pData->sSourceDirectionVec = -1;
            pData->fSourceDirectionXVal = 0.f;
            pData->fSourceDirectionYVal = 0.f;
            pData->fSourceDirectionZVal = 1.f;
            pData->sSourceVelocityVec = -1;
            pData->fSourceVelocityXVal = 0.f;
            pData->fSourceVelocityYVal = 0.f;
            pData->fSourceVelocityZVal = 0.f;
            pData->sObjectPositionVec = -1;
            pData->fObjectPositionXVal = 0.f;
            pData->fObjectPositionYVal = 0.f;
            pData->fObjectPositionZVal = 0.f;
            pData->sObjectDirectionVec = -1;
            pData->fObjectDirectionXVal = 0.f;
            pData->fObjectDirectionYVal = 0.f;
            pData->fObjectDirectionZVal = 1.f;
            pData->sObjectUpVec = -1;
            pData->fObjectUpXVal = 0.f;
            pData->fObjectUpYVal = 1.f;
            pData->fObjectUpZVal = 0.f;
            pData->sObjectVelocityVec = -1;
            pData->fObjectVelocityXVal = 0.f;
            pData->fObjectVelocityYVal = 0.f;
            pData->fObjectVelocityZVal = 0.f;
            pData->sListenerPositionVec = -1;
            pData->fListenerPositionXVal = 0.f;
            pData->fListenerPositionYVal = 0.f;
            pData->fListenerPositionZVal = 0.f;
            pData->sListenerDirectionVec = -1;
            pData->fListenerDirectionXVal = 0.f;
            pData->fListenerDirectionYVal = 0.f;
            pData->fListenerDirectionZVal = 1.f;
            pData->sListenerUpVec = -1;
            pData->fListenerUpXVal = 0.f;
            pData->fListenerUpYVal = 1.f;
            pData->fListenerUpZVal = 0.f;
            pData->sListenerVelocityVec = -1;
            pData->fListenerVelocityXVal = 0.f;
            pData->fListenerVelocityYVal = 0.f;
            pData->fListenerVelocityZVal = 0.f;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Azimuth") == 0)
                return 0;
            else if (strcmp(pchControl, "Elevation") == 0)
                return 1;
            else if (strcmp(pchControl, "Distance") == 0)
                return 2;
            else if (strcmp(pchControl, "Width") == 0)
                return 3;
            else if (strcmp(pchControl, "Velocity") == 0)
                return 4;
            else if (strcmp(pchControl, "SourceIncidence") == 0)
                return 5;
            else if (strcmp(pchControl, "ListenerIncidence") == 0)
                return 6;
            else
                return -1;
        }

    private:
    };
}

#endif // _SOURCE_H_
