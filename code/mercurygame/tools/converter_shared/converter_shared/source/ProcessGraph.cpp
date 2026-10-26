#include "stdafx.h"

#include <system/time/time.h>
#include "converter_shared/ProcessGraph.h"
#include <system/perf/PerfEvent.h>
#include <btl/utility/autopointer.h>
#include <system/memory/MemoryTracker.h>

//#include <tools_shared/executer/ExecuterHelpers.h>
//#include <tools_shared/ToolsApplication.h>

namespace Tools
{

namespace ProcessGraph
{

// C_Data

C_Data::C_Data()
	:	m_ProcessedState(-1)
	,   m_VisitedState(-1)
	,	m_refCount(0)
{
}

C_Data::~C_Data()
{
}

void C_Data::IncRefCount()
{
	m_refCount++;
}

void C_Data::DecRefCount()
{
	m_refCount--;
	if (m_refCount == 0)
	{
		Cleanup();
	}
}

void C_Data::Cleanup()
{
}

bool C_Data::Build( const S32 desiredState, const Util::C_String& name, Progress::I_ProgressBar* pProgress )
{
	bool success = true;

	C_ProcessList processList;

	LOGINFO( g_log, "==========   %s   ==========", name.Chars() );
	const Time::C_Time startTime = Time::GetPerformanceCounter();

	ClearVisited();
	success = CreateProcessList( desiredState, processList );
	if(!success)
		return false;

	success = ExecuteProcessList( name, processList, pProgress );
	if(!success)
		return false;

	const Time::C_Time endTime = Time::GetPerformanceCounter();
	const Time::C_Time duration = endTime - startTime;

	LOGINFO( g_log, "==========   %s - Took %s   ==========", name.Chars(), Time::FormatDurationString(duration).Chars() );

	return true;
}

void C_Data::ClearVisited()
{
	m_VisitedState = -1;

	const S32 sourceProcessCount = GetSourceProcessCount();
	for(S32 sourceProcessNo=0; sourceProcessNo<sourceProcessCount; sourceProcessNo++)
	{
		C_Process* pSourceProcess = GetSourceProcess(sourceProcessNo);
		for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = pSourceProcess->m_InputList.Begin(); i != pSourceProcess->m_InputList.End(); ++i)
		{
			C_Process::C_DataItem* pDataItem = *i;
			C_Data* pInputData = pDataItem->m_pData;
			if(pInputData)
			{
				pInputData->ClearVisited();			
			}		
		}	
	}
}

void C_Data::DumpSourceProcesses()
{
	LOGDEBUG( g_log, "DumpSourceProcesses() for %s", GetName() );

	const S32 sourceProcessCount = GetSourceProcessCount();
	for(S32 sourceProcessNo=0; sourceProcessNo<sourceProcessCount; sourceProcessNo++)
	{
		C_Process* pSourceProcess = GetSourceProcess(sourceProcessNo);
		pSourceProcess->Dump();
	}
}

bool C_Data::CreateProcessList( const S32 desiredState, C_ProcessList& processList )
{
	if(GetProcessedState() == desiredState)
	{
		return true;
	}

	if(m_VisitedState == desiredState)
	{
		return true;
	}

	C_Process* pSourceProcess = FindSourceProcess(desiredState);
	if(!pSourceProcess)
	{
		LOGWARN( g_log, "ProcessGraph - Cannot process data %s into state %d, no source process", GetName().Chars(), desiredState );
		DumpSourceProcesses();
		return false;
	}

	// build all inputs to source process have been built
	bool success = true;

	S32 index = 0;
	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = pSourceProcess->m_InputList.Begin(); i != pSourceProcess->m_InputList.End(); ++i, ++index)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pInputData = pDataItem->m_pData;
		if(pInputData)
		{
			const bool buildSuccess = pInputData->CreateProcessList( pDataItem->GetType(), processList );

			if(!buildSuccess)
			{
				success = false;
				break;
			}
		}		
	}	

	if(success)
	{
		// Add this process to list
		processList.PushBack( pSourceProcess );
	}

	// set all outputs of process visited states
	pSourceProcess->SetVisitedStates();

	return success;
}

void C_Data::SetProcessedState( const S32 processedState )
{
	m_ProcessedState = processedState;
}

void C_Data::DumpGraph( const Util::C_String& printPrefix, const Util::C_String& subNodePrefix )
{
	LOGDEBUG( g_log, "%sData '%s' (%d)", printPrefix.Chars(), GetName().Chars(), GetProcessedState() );

	S32 sourceProcessCount = GetSourceProcessCount();
	for(S32 sourceProcessNo=0; sourceProcessNo<sourceProcessCount; sourceProcessNo++)
	{
		C_Process* pSourceProcess = GetSourceProcess(sourceProcessNo);

		Util::C_String sourceProcessName = "";
		if(pSourceProcess)
		{
			sourceProcessName = pSourceProcess->GetName();
		}

		LOGDEBUG( g_log, "%s|", subNodePrefix.Chars() );
		LOGDEBUG( g_log, "%s* Process '%s'", subNodePrefix.Chars(), pSourceProcess->GetName().Chars() );
		LOGDEBUG( g_log, "%s|", subNodePrefix.Chars() );

		const S32 inputDataCount = pSourceProcess->m_InputList.Size();
		for( S32 inputDataNo=0; inputDataNo<inputDataCount; inputDataNo++)
		{		
			C_Process::C_DataItem* pDataItem = pSourceProcess->m_InputList[inputDataNo];
			C_Data* pInputData = pDataItem->m_pData;
			if(pInputData)
			{
				const bool isFirstNode = (inputDataNo == 0);
				const bool isLastNode = (inputDataNo == (inputDataCount - 1));
				if(isLastNode)
				{
					// Last Node
					pInputData->DumpGraph( subNodePrefix + "`---", subNodePrefix + "    " );
				}
				else
				if(isFirstNode)
				{
					// First Node
					pInputData->DumpGraph( subNodePrefix + "+---", subNodePrefix + "|   " );
				}
				else
				{
					// Middle Node
					pInputData->DumpGraph( subNodePrefix + "|---", subNodePrefix + "|   " );
				}
			}
		}	

		if(inputDataCount > 0)
		{
			LOGDEBUG( g_log, "%s", subNodePrefix.Chars() );
		}

	}
}

void C_Data::DumpGraph()
{
	LOGDEBUG( Tools::g_log, "<tt><pre>" );

	DumpGraph( "", "" );

	LOGDEBUG( Tools::g_log, "</tt></pre>" );
}

S32 C_Data::GetProcessedState()
{
	return m_ProcessedState;
}

void C_Data::AddSourceProcess(C_Process* pProcess)
{
	m_pSourceProcessList.PushBack( pProcess );
}

const S32 C_Data::GetSourceProcessCount() const
{
	return m_pSourceProcessList.Size();
}

C_Process* C_Data::GetSourceProcess( const S32 index ) const
{
	return m_pSourceProcessList[index];
}


C_Process* C_Data::FindSourceProcess( const S32 desiredState ) const
{
	S32 sourceProcessCount = GetSourceProcessCount();
	for(S32 sourceProcessNo=0; sourceProcessNo<sourceProcessCount; sourceProcessNo++)
	{
		C_Process* pProcess = GetSourceProcess( sourceProcessNo );
		if( pProcess->GetOutput( desiredState ) == this )
		{
			return pProcess;
		}
	}

	return NULL;
}

// C_Process

C_Process::C_Process()
{
}

C_Process::~C_Process()
{
	// delete all items in input / output list
	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_InputList.Begin(); i != m_InputList.End(); ++i)
	{
		C_Process::C_DataItem*& pDataItem = *i;
		SAFE_DELETE_PTR( pDataItem );
	}

	// delete all items in input / output list
	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_OutputList.Begin(); i != m_OutputList.End(); ++i)
	{
		C_Process::C_DataItem*& pDataItem = *i;
		SAFE_DELETE_PTR( pDataItem );
	}
}

void C_Process::AddInput(C_Data* pNewInput, const S32 newType, const Util::C_String& newName)
{
	// check input with same type is not already in output list
	assert( GetInput( newType ) == NULL );

	pNewInput->IncRefCount();
	m_InputList.PushBack( new C_DataItem(pNewInput, newType, newName) );
}

void C_Process::AddOutput(C_Data* pNewOutput, const S32 newType, const Util::C_String& newName)
{
	// check output with same type is not already in output list
	assert( GetOutput( newType ) == NULL );

	m_OutputList.PushBack( new C_DataItem(pNewOutput, newType, newName) );

	// check there is not already a source process that produces this data type
	assert(pNewOutput->FindSourceProcess(newType) == NULL);
	pNewOutput->AddSourceProcess(this);
}

const C_Data* C_Process::GetInput( const S32 type )
{
	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_InputList.Begin(); i != m_InputList.End(); ++i)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pData = pDataItem->m_pData;
		if(pDataItem->GetType() == type)
			return pData;
	}
	return NULL;
}

C_Data* C_Process::GetOutput( const S32 type )
{
	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_OutputList.Begin(); i != m_OutputList.End(); ++i)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pData = pDataItem->m_pData;
		if(pDataItem->GetType() == type)
			return pData;
	}
	return NULL;
}

bool C_Process::ExecuteProcess( Progress::I_ProgressBar* pProgress )
{
	bool success = Process( pProgress );
	if( !success )
		return false;

	for(Ut::Vector<C_Process::C_DataItem*>::Iterator input = m_InputList.Begin(); input != m_InputList.End(); ++input)
	{
		C_Process::C_DataItem* pDataItem = *input;
		C_Data* pData = pDataItem->m_pData;
		pData->DecRefCount();
	}

	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_OutputList.Begin(); i != m_OutputList.End(); ++i)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pData = pDataItem->m_pData;
		pData->SetProcessedState( pDataItem->GetType() );
	}

	return true;
}

void C_Process::SetVisitedStates()
{
	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_OutputList.Begin(); i != m_OutputList.End(); ++i)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pOutputData = pDataItem->m_pData;
		if(pOutputData)
		{
			pOutputData->m_VisitedState = pDataItem->GetType();
		}
	}
}

void C_Process::Dump()
{
	LOGDEBUG( g_log, "Process %s", GetName().Chars() );

	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_InputList.Begin(); i != m_InputList.End(); ++i)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pData = pDataItem->m_pData;

		LOGDEBUG( g_log, "Input %s(%s)->%d", pDataItem->GetName().Chars(), pData->GetName().Chars(), pDataItem->GetType());
	}

	for(Ut::Vector<C_Process::C_DataItem*>::Iterator i = m_OutputList.Begin(); i != m_OutputList.End(); ++i)
	{
		C_Process::C_DataItem* pDataItem = *i;
		C_Data* pData = pDataItem->m_pData;
		LOGDEBUG( g_log, "Output %s(%s)->%d", pDataItem->GetName().Chars(), pData->GetName().Chars(), pDataItem->GetType());
	}
}

bool ExecuteProcessList( const Util::C_String& name, const C_ProcessList& processList, Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateMulti multiProgress( pProgress, name );

	const S32 processCount = processList.Size();
	for(S32 processNo=0; processNo<processCount; processNo++)
	{
		multiProgress.AddChild(1.0f);
	}

	bool success = true;

	static Btl::Vector<Btl::AutoPointer<Util::C_String> > perfStrings;

	for(S32 processNo=0; processNo<processCount; processNo++)
	{
		C_Process* pProcess = processList[processNo];
		// run source process
		//EXECUTER_INFO( "%s\n", m_pSourceProcess->GetName().Chars() );

		LOGINFO( g_log, "=== Begin Process: %s", pProcess->GetName().Chars() );
		perfStrings.PushBack(new Util::C_String(pProcess->GetName()));
		PERF_EVENT(perfStrings.Back()->Chars());
		MEMORY_TRACKER_PUSH_SUB(perfStrings.Back()->Chars());
		const Time::C_Time startTime = Time::GetPerformanceCounter();

		const bool processSuccess =  pProcess->ExecuteProcess( multiProgress[processNo] );

		const Time::C_Time endTime = Time::GetPerformanceCounter();
		const Time::C_Time duration = endTime - startTime;

		MEMORY_TRACKER_POP_SUB(perfStrings.Back()->Chars());

		LOGINFO( g_log, "--- End Process: %s - Took %s", pProcess->GetName().Chars(), Time::FormatDurationString(duration).Chars() );

		if(!processSuccess)
		{
			LOGWARN( g_log, "An error occurred while performing '%s'", pProcess->GetName().Chars() );
			success = false;
			break;
		}
	}
	
	for(S32 processNo=0; processNo<processCount; processNo++)
	{
		delete multiProgress[processNo];
	}

	return success;
}

} // namespace ProcessGraph

} // namespace Tools