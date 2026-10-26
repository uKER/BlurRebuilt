#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "tools_common/SystemDefines.h"

#include "cc++/Persist.h"
#include "cc++/Thread.h"
#include "cc++/SocketPort.h"
#include "cc++/Socket.h"
#include "cc++/Address.h"
typedef ost::BaseObject BaseObject;

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include "core/utility/CoreDebug.h"

#ifdef __GNUC__
#define min(A,B) ((A<B)?A:B)
#define max(A,B) ((A>B)?A:B)
#endif

#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/maths/utmaths/mmatrix.h>
#include <core/maths/utmaths/colour.h>
#include <system/debuglog.h>

#include "Network/Socket.h"
#include "Network/Network.h"
#include "Network/IServer.h"
#include "Network/Server.h"
#include "Network/Connection.h"
#include "Network/JobScheduler.h"
#include "Network/Debug.h"
#include "Network/ClientInterface.h"
#include "Network/JobScheduler.h"
#include "Network/Interface.h"
