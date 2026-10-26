#ifndef XT_XTTOOL_H
#define XT_XTTOOL_H

#ifdef _MSC_VER
#pragma once
#endif

void DebugLn(const char* text, ...);

#include "Xt/Impl/XtCore.h"
#include "Xt/Impl/XtReflection.h"

void XtLogInit(const char* logPath, const char* appName);
void XtLogV(const char* area, const char* msg, va_list vl);
void XtLog(const char* area, const char* msg, ...);

struct XtFileTime // TODO: Move this
{
public:
	XtFileTime()
	{
		ft.dwLowDateTime	= 0;
		ft.dwHighDateTime	= 0;
	}

public:
	operator FILETIME&()				{ return ft;	}
	operator const FILETIME&() const	{ return ft;	}
	operator FILETIME*()				{ return &ft;	}
	operator const FILETIME*() const	{ return &ft;	}
	FILETIME* operator->()				{ return &ft;	}
	const FILETIME* operator->() const	{ return &ft;	}

public:
	bool operator==(const FILETIME& rhs) const
	{
		return (ft.dwLowDateTime == rhs.dwLowDateTime) && (ft.dwHighDateTime == rhs.dwHighDateTime);
	}

	bool operator!=(const FILETIME& rhs) const
	{
		return (ft.dwLowDateTime != rhs.dwLowDateTime) || (ft.dwHighDateTime != rhs.dwHighDateTime);
	}

public:
	FILETIME ft;
};

#include "Xt/Impl/Sz.h"
#include "Xt/Impl/XtString.h"
#include "Xt/Impl/XtSimpleFile.h"
#include "Xt/Impl/XtStream.h"
#include "Xt/Impl/XtDom.h"
#include "Xt/Impl/XtDomHelpers.h"
#include "Xt/Impl/XtMerge.h"
#include "Xt/Impl/Larva.h"

// TODO: Move these

enum XtSeverity
{
	kXtSevInfo,
	kXtSevWarning,
	kXtSevError,
	kXtSevTrace
};

struct XtTargetProfile
{
	XtEndian endian;
};

struct XtPackBuildParams
{
public:
	XtPackBuildParams();
	virtual ~XtPackBuildParams();

public:
	virtual void Output(XtSeverity severity, const char* text);
};

XtSchema* XtLoadSchema(const char* filename);
bool XtPackBuild(XtTargetProfile* targetProfile, Xt::DomDb* db, Xt::DomPack* pack, XtUt::Block* data, XtPackBuildParams* params);
bool XtPackBuild(XtTargetProfile* targetProfile, Xt::DomDb* db, Xt::DomRecord** records, size_t count, XtUt::Block* data, XtPackBuildParams* params);

struct XtBuildParams
{
	Xt::DomDb*	db;

	bool		doData;
	bool		doLocalisation;
	bool		doDeploy;

	bool		overrideStage;
	XtString	stageName;
	XtString	stagePath;

	bool						overridePacks;
	XtUt::Array<Xt::DomPack*>	packs;

	bool					overridePlatforms;
	XtUt::Array<XtString>	platforms;

public:
	XtBuildParams();
	virtual ~XtBuildParams();

public:
	virtual void Output(XtSeverity severity, const char* text);
	virtual void Progress(float fraction);
	virtual bool IsCancelled();
};

void XtBuild(XtBuildParams* params);
int XtGetDbVersion();

struct XtCustomExportContext
{
	Xt::DomDb*		db;
	LtStruct*		containerType;
	Xt::DomBlock*	containerDom;
	LarvaField*		field;
	LarvaAttr*		attr;

public:
	XtCustomExportContext(Xt::DomDb* db_, LtStruct* containerType_, Xt::DomBlock* containerDom_, LarvaField* field_, LarvaAttr* attr_) :
		db(db_),
		containerType(containerType_),
		containerDom(containerDom_),
		field(field_),
		attr(attr_)
	{
	}
};

bool XtCustomExport_ShaderConst(XtCustomExportContext* ctx);

#endif // XT_XTTOOL_H