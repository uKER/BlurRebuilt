#include "SystemPch.h"
#include <System/Debug/Debug.h>
//-----------------------------------------

Debug::Float g_DebugTestFloat5( "Test.Debug.Float.5", 0.0f, Debug::Range(0.0f, 1.0f) );
Debug::Float g_DebugTestFloat4( "Test.Debug.Float.4", 0.0f, Debug::Range(0.0f, 1.0f) );
Debug::Float g_DebugTestFloat3( "Test.Debug.Float.3", 0.0f, Debug::Range(0.0f, 1.0f) );
Debug::Float g_DebugTestFloat2( "Test.Debug.Float.2", 0.0f, Debug::Range(0.0f, 1.0f) );
Debug::Float g_DebugTestFloat1( "Test.Debug.Float.1", 0.0f, Debug::Range(0.0f, 1.0f) );

Debug::Int g_DebugTestInt5( "Test.Debug.Int.5", 0, Debug::Range(0, 10) );
Debug::Int g_DebugTestInt4( "Test.Debug.Int.4", 0, Debug::Range(0, 10) );
Debug::Int g_DebugTestInt3( "Test.Debug.Int.3", 0, Debug::Range(0, 10) );
Debug::Int g_DebugTestInt2( "Test.Debug.Int.2", 0, Debug::Range(0, 10) );
Debug::Int g_DebugTestInt1( "Test.Debug.Int.1", 0, Debug::Range(0, 10) );

Debug::Bool g_DebugTestBool5( "Test.Debug.Bool.5", false );
Debug::Bool g_DebugTestBool4( "Test.Debug.Bool.4", false );
Debug::Bool g_DebugTestBool3( "Test.Debug.Bool.3", false );
Debug::Bool g_DebugTestBool2( "Test.Debug.Bool.2", false );
Debug::Bool g_DebugTestBool1( "Test.Debug.Bool.1", false );
