#ifndef _MESSAGEDELAY_H_
#define _MESSAGEDELAY_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MessageDelay
////////////////////////////////////////////////////////////////////////////////
    class MessageDelay : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
		    Option optLength;
			Event evDelayed;
        };

        struct State
        {
            F32 fDelay;
        };

        enum Messages
        {
            DelayMsg,
            CancelMsg,

            NumMessages
        };

    public:
        MessageDelay( Object* pObject ) : Component( pObject )
        {
        }

        void* Init( void* pData );
        U32 ContextSize() { return sizeof(State); }
        void InitContext( Instance* pInstance );
        void Update( Instance* pInstance );
        void DoMessage( Instance* pInstance, U32 uMessage );

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "MessageDelay"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return ( flagUpdater | flagControl ); }

        static U32 Size() { return sizeof( MessageDelay ); }
		static U32 DataSize( void* pData ) { return sizeof( Data ); }

        static void DefaultData( Data* pData )
        {
            pData->strName = -1;
			pData->optLength.ctrl.sCom = -1;
			pData->optLength.ctrl.uOut = 0;
			pData->optLength.fVal = 1.f;
			pData->evDelayed.sCom = -1;
			pData->evDelayed.uMsg = 0;
        }

        static bool ValidateData( Data* pData )
        {
            return true;
        }

		static S32 GetMessage( const char* pchMessage )
		{
			if( strcmp( pchMessage, "Delay" ) == 0 )
				return 0;
			if( strcmp( pchMessage, "Cancel" ) == 0 )
				return 1;
			else
				return -1;
		}

    private:
        inline void Delay( Instance* pInstance );
        inline void Cancel( Instance* pInstance );

    private:
    };
}

#endif // _MESSAGEDELAY_H_
