#include "CorePch.h"
#include "core/threads/threadlocalstorage.h"

namespace MT
{

#ifdef PLATFORM_PS3
__thread uint32_t tlsStorage[ TLS_MINIMUM_AVAILABLE ];
bool tls_master_used[TLS_MINIMUM_AVAILABLE];
#endif

}
