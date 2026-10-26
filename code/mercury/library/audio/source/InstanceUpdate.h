#ifndef _INSTANCE_UPDATE_H_
#define _INSTANCE_UPDATE_H_


#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
#include "SystemPS3shared.h"
#endif

namespace Audio { class Instance; }

struct InstanceUpdateParams
{
	Audio::Instance* pInstance;
	bool bUpdate;
	bool bGenerate;
	bool bBreak;
#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
	Audio::AudioSpuObjectUpdateInfo m_SpuInfo;
#endif
};


void InstanceUpdate(Audio::Instance* pInstance, bool bUpdate, bool bGenerate);
void JobScheduledInstanceUpdate(Scheduler::Task* pTask);


#endif	//	_INSTANCE_UPDATE_H_

