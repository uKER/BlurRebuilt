#include "LogicalFsPch.h"

#include <Tools_Shared/SchedulerConfig.h>

#include <PakFs/PakGlobals.h>
 
namespace Pak
{

Scheduler::PolicyHandle g_pakPacketCompressionPolicy = -1;

void SetPacketCompressionPolicy(S32	policy)
{
	g_pakPacketCompressionPolicy = policy;
}

S32 GetPacketCompressionPolicy()
{
	return g_pakPacketCompressionPolicy;
}

}