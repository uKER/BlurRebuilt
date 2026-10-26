#ifndef _IMPACTFILTER_H_
#define _IMPACTFILTER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	ImpactFilter
////////////////////////////////////////////////////////////////////////////////
    class ImpactFilter : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
			Control ctrlForce;
			Option optDelay;
			Option optHalfLife;
			Event evImpact;
        };

        struct State
        {
			bool bImpact;
            F32 fThreshold;
            F32 fDelay;
            F32 fCurrect;
        };

        enum Controls
        {
            ValueCtrl
        };

		enum Messages
        {
            ImpactMsg,
            ResetMsg,

            NumMessages
        };

    public:
        ImpactFilter( Object* pObject ) : Component( pObject )
        {
        }

        void* Init( void* pData );
        U32 ContextSize() { return sizeof( State ); }
        void InitContext( Instance* pInstance );
        void Update( Instance* pInstance );
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage( Instance* pInstance, U32 uMessage );

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "ImpactFilter"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return ( flagUpdater | flagControl ); }

        static U32 Size() { return sizeof( ImpactFilter ); }
		static U32 DataSize( void* pData ) { return sizeof( Data ); }

        static void DefaultData( Data* pData )
        {
            pData->strName = -1;
			pData->ctrlForce.sCom = -1;
			pData->ctrlForce.uOut = 0;
			pData->optDelay.ctrl.sCom = -1;
			pData->optDelay.ctrl.uOut = 0;
			pData->optDelay.fVal = 0.f;
			pData->optHalfLife.ctrl.sCom = -1;
			pData->optHalfLife.ctrl.uOut = 0;
			pData->optHalfLife.fVal = 0.1f;
			pData->evImpact.sCom = -1;
			pData->evImpact.uMsg = 0;
        }

        static bool ValidateData( Data* pData )
        {
            if( pData->ctrlForce.sCom == -1 )
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

		static S32 GetMessage( const char* pchMessage )
		{
			if( strcmp( pchMessage, "Impact" ) == 0 )
				return 0;
			if( strcmp( pchMessage, "Reset" ) == 0 )
				return 1;
			else
				return -1;
		}

    private:
        inline void Impact( Instance* pInstance );
        inline void Reset( Instance* pInstance );

    private:
    };
}

#endif // _IMPACTFILTER_H_
