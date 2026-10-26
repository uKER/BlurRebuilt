#include "FrameworkPch.h"
#include "Framework/Framework.h"

namespace Framework
{

Debug::Logger& g_Log = Debug::GetLogger("mercury.framework");
Debug::Logger& g_BootLog = Debug::GetLogger("mercury.framework.bootlog");

}; // namespace Framework
