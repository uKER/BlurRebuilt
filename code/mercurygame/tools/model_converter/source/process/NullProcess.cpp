#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>

#include <process/NullProcess.h>

C_ProcessNull::C_ProcessNull( const Util::C_String& name )
	: m_Name( name )
{
}

const Util::C_String C_ProcessNull::GetName() const
{
	return m_Name;
}

bool C_ProcessNull::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	//LOGDEBUG( Tools::g_log, "Processing '%s'", GetName().Chars() );

	return true;
}

