
#include "MessageHandler.h"
#include "Defines.h"

C_MessageHandler C_MessageHandler::m_sInstance;

void C_MessageHandler::ShowMessage( const E_MessageType type, const char *str )
{
	switch( type )
	{
		case E_MessageType_Error : // Fall through
			sADMBasic->ErrorAlert( str );
			break;

		case E_MessageType_Warning :
			sADMBasic->LightweightErrorAlert( str );
			break;

		case E_MessageType_Info :
			sADMBasic->LightweightErrorAlert( str );
			break;
	}
}