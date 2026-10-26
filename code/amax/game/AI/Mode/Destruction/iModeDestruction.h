#ifndef _MODE_DESTRUCTION_AI_I_H
#define _MODE_DESTRUCTION_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>
#include <misc_shared/UID/FourCC.h>

namespace airacingtech
{

	enum E_DestructionModeMessage	
	{
		/****************************************************************************\
		|						Destruction Mode Messages									 |
		\****************************************************************************/

		E_DestructionModeMessage_HiBits = FOURCC_HI20('DTRC'),	
		E_DestructionModeMessage_WaveCompleted,
	};

	BEGIN_INTERFACE_DEC(IModeDestruction)

	public:

		virtual void Update() = 0;

#ifndef CFG_GOLD
		virtual void DebugRender() = 0;
#endif 

	END_INTERFACE_DEC


}


#endif //_MODE_DESTRUCTION_AI_I_H