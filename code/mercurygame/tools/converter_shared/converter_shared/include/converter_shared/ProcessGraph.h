#ifndef CONVERTER_SHARED_PROCESS_GRAPH_H
#define CONVERTER_SHARED_PROCESS_GRAPH_H

#include <system/misc/Progress.h>
#include <btl/containers/Vector.h>

#ifdef _MSCVER
#pragma once
#endif

//! @defgroup ProcessGraph ProcessGraph Tools Processing Structure
//! The ProcessGraph module provides a framework to structure tools process and data flow. 
//!
//! \par Overview
//! .
//!

namespace Tools
{

//!
//! @addtogroup ProcessGraph
//! @{

//! The ProcessGraph namespace contains the code for the ProcessGraph module.
namespace ProcessGraph
{

class C_Data;
class C_Process;

typedef Btl::Vector<C_Process*> C_ProcessList;

//! The high level data objects used within a tool should be derived from this C_Data object.
class C_Data
{
friend class C_Process;

public:
	C_Data();

	virtual ~C_Data();

	//! This returns the C_Data's integer type indentifier. This is assigned by the derived class.
	//virtual const S32 GetType() const = 0;

	//virtual const bool IsType( const S32 type ) const = 0;

	//! This returns a textual description of the C_Data type. This is used for logging and debugging.
	virtual const Util::C_String GetName() const = 0;

	//! If the data is not built already, this will build any input data the source process relies on and then call the source process.
	bool Build( const S32 desiredState, const Util::C_String& name, Progress::I_ProgressBar* pProgress );

	//! Returns the processed state of the data.
	S32 GetProcessedState();

	//! Manually set the processed state of this data
	void SetProcessedState( S32 state );

	void DumpGraph();

	void DumpSourceProcesses();

	void IncRefCount();
	void DecRefCount();

	//called when refcount gets to zero
	virtual void Cleanup();

private:
	void DumpGraph( const Util::C_String& printPrefix, const Util::C_String& subNodePrefix );

	void AddSourceProcess(C_Process* pProcess);

	const S32 GetSourceProcessCount() const;

	C_Process* GetSourceProcess( const S32 index ) const;

	C_Process* FindSourceProcess( const S32 desiredState ) const;

	void ClearVisited();

	bool CreateProcessList( const S32 desiredState, C_ProcessList& processList );

private:
	S32 m_ProcessedState;

	// used internally by CreateProcessList
	S32 m_VisitedState;

	Btl::Vector<C_Process*> m_pSourceProcessList;

	S32 m_refCount;
};

//! The high level processes used within a tool should be derived from this C_Process object.
//! Then the AddInput and AddOutput should be used to create the process dependencies.
class C_Process
{
friend class C_Data;

	class C_DataItem
	{
		public:
			C_DataItem( C_Data* pData, const S32 type, const Util::C_String& name )
				: m_pData( pData )
				, m_Type( type )
				, m_Name( name )
			{
			}

			S32 GetType() const { return m_Type; }
			const Util::C_String& GetName() const { return m_Name; }
		public:
			C_Data*			m_pData;
			S32				m_Type;
			Util::C_String	m_Name;
	};

public:
	C_Process();

	virtual ~C_Process();

	//! Function called to execute process
	bool ExecuteProcess( Progress::I_ProgressBar* pProgress );

	//! This returns a textual description of the C_Process type. This is used for logging and debugging.
	virtual const Util::C_String GetName() const = 0;

	//! Assigns a new input data dependency for this process.
	void AddInput(C_Data* pNewInput, const S32 newType, const Util::C_String& newName);

	//! Assigns a new output data dependency for this process. Note, each C_Data object can only have one source C_Process.
	void AddOutput(C_Data* pNewOutput, const S32 newType, const Util::C_String& newName);

	//! Returns the input data object with the specified type or NULL if the type is not found in the input list.
	const C_Data* GetInput( const S32 type );

	//! Returns the output data object with the specified type or NULL if the type is not found in the output list.
	C_Data* GetOutput( const S32 type );

	void Dump();

private:
	//! Virtual funtion to implement the process
	virtual bool Process( Progress::I_ProgressBar* pProgress ) = 0;

	void SetVisitedStates();


	Ut::Vector<C_DataItem*> m_InputList;

	Ut::Vector<C_DataItem*> m_OutputList;
};

bool ExecuteProcessList( const Util::C_String& name, const C_ProcessList& processList, Progress::I_ProgressBar* pProgress );

} // namespace ProcessGraph

//! @}

} // namespace Tools

#endif
