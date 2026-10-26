#ifndef NETVOICE_LOCALTALKER_H
#define NETVOICE_LOCALTALKER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netEnums/netEnums.h"
#include "netTime/netTime.h"

#include <SystemProfile/SystemProfile.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netVoice
{
	BD_REFERENCE(LocalTalker);
	class LocalTalker: public bdReferencable
	{
	public:
		LocalTalker(const SysProfile::FullProfile& profile);
		~LocalTalker();

		bool talking();
		int poll(char* data, int maxData);
		
		const netUserId				m_userId;		// online user Id.
		SysProfile::FullProfile		m_profile;		// the full profile spec.
		netStopwatch				m_debounceTalk; // notifies whe a player starts talking

	private:
		bool registered();
		bool register_();
		bool unregister();
	};
}
#endif //NETVOICE_LOCALTALKER_H