#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <System/DebugLog.h>
//#include <System/Debug/Debug.h>

#include <system\DebugLog.h>
#include <System/Debug/Log/Subscribers/SubscriberCommon.h>

//-----------------------------------------------------------------------------
// C_ErrorHandler
//-----------------------------------------------------------------------------
class C_ErrorHandler : public Debug::SubscriberCommon
{
public:
				C_ErrorHandler() {}
 
				~C_ErrorHandler() {}
 
	void		Log(Debug::LogEvent *pEvent); 

	void        LogString(const Util::C_String &string) {}
};

C_ErrorHandler* g_pErrorHandler;

//-----------------------------------------------------------------------------
// XT annotated structs
//-----------------------------------------------------------------------------

struct AudioEntityDef;

const XtI32 INVALID_INDEX = 0xffffffff;

struct DbEntityDef
{
};

//=============================================================================
// AudioObject
//=============================================================================
struct AudioObject
{
	XtId	id;								// not annotated. 
	XtSz8	filename;
};

//=============================================================================
// Object and Instance
//=============================================================================

struct Object
{
	Object() : index(INVALID_INDEX) {}

	XtI32					index;			// not xt annotated. for internal use only
	XtHandle<AudioObject>	object;
};

struct Instance
{
	Instance() : index(INVALID_INDEX) {}

	XtI32		index;						// not xt annotated. for internal use only
	XtSz8		name;
	Object*		object;
};

//=============================================================================
// Internal Connections
//=============================================================================

struct InternalConnector
{
	InternalConnector() : index(INVALID_INDEX) {}

	XtI32		index;						// not xt annotated. for internal use only
	Instance*	instance;
	XtSz8		component;
};

struct InternalConnectorIn : InternalConnector
{
};

struct InternalConnectorOut : InternalConnector
{
};

struct InternalConnection
{
	InternalConnectorOut*	from;
	InternalConnectorIn*	to;
};

//=============================================================================
// External Connections
//=============================================================================

struct ExternalConnector
{
	XtSz8	instanceFrom;
	XtSz8	componentFrom;
};

struct IncomingConnection
{
	ExternalConnector		from;
	InternalConnectorIn*	to;
};

struct OutgoingConnection
{
	InternalConnectorOut*	from;
	ExternalConnector		to;
};

//=============================================================================
// Functions
//=============================================================================

struct ObjFunction
{
	XtArray<Object> objCreates;
};

struct InstFunction
{
	Ut::Vector<InternalConnector*>	connectors;		// not xt annotated. created by hand

	XtArray<Instance>				instances;
	XtArray<InternalConnectorOut>	connectorsOut;
	XtArray<InternalConnectorIn>	connectorsIn;
	XtArray<InternalConnection>		connections;
};

struct ConnectFunction
{
	XtHandle<AudioEntityDef>	connectEnv;
	XtArray<IncomingConnection> incoming;
	XtArray<OutgoingConnection> outgoing;
};

struct External
{
	XtSz8				referenceString;
	InternalConnector*	connector;
};

struct AudioEntityGroup
{
	XtI32	id;						// not xt annotated. for internal use only
	XtSz8	name;
	XtU32	index;
};

//=============================================================================
// AudioEntityDef Record
//=============================================================================

struct AudioEntityDef
{
public:
	void						SetupArrayIndices();

public:
	XtHandle<AudioEntityGroup>	group;
	XtI32						id;						// not xt annotated. for internal use only
	XtSz8						name;
	ObjFunction					objCreate;
	XtArray<ConnectFunction>	connectFunctions;
	XtArray<External>			externs;
	InstFunction				instFunction;	
};

struct RecordList
{
	Ut::Vector <AudioEntityDef*> scripts;
	Ut::Vector <AudioEntityGroup*> groups;
	Ut::Vector <AudioObject*> audioObjects;
};
#endif // MAIN_H
