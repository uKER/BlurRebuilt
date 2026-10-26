#ifndef APP_NULL_PROCESS_H
#define APP_NULL_PROCESS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>


class C_ProcessNull : public Tools::ProcessGraph::C_Process
{
public:
	C_ProcessNull( const Util::C_String& name );

	// inherited from C_Process
	const Util::C_String GetName() const;

	bool Process( Progress::I_ProgressBar* pProgress );

private:
	Util::C_String m_Name;
};

#endif




