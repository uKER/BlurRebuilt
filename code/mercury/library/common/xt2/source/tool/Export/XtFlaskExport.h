#ifndef XT_TOOL_XTFLASKEXPORT_H
#define XT_TOOL_XTFLASKEXPORT_H

#ifdef _MSC_VER
#pragma once
#endif

struct XtPackBuildContext;

typedef ExportType* (*XtRecordTypeCallback)(XtPackBuildContext*, XtId);

struct XtPackBuildContext
{
	XtTargetProfile*			targetProfile;
	XtUt::Array<Xt::DomRecord*>	records;
	XtUt::List<Xt::DomRecord>	tmpRecords;
	XtRecordTypeCallback		callback;
	void*						context;

public:
	inline XtPackBuildContext() :
		targetProfile(0),
		callback(0),
		context(0)
	{
	}
};

struct FlaskExportContext
{
	ExportSchema*				schema;
	XtPackBuildContext*			pb;
	XtUt::List<ExportRecord>	records;

public:
	inline FlaskExportContext() :
		schema(0),
		pb(0)
	{
	}
};

struct CompileFlaskContext
{
	XtUt::Array<FlaskType>		types;
	XtUt::Array<FlaskBase>		bases;
	XtUt::Array<FlaskField>		fields;
	XtUt::Block					strings;
	XtUt::Array<FlaskRef>		refs;
	XtUt::Array<FlaskRecord>	records;
	XtUt::Array<FlaskComponent>	components;
};

struct XtFeRecordRef
{
	size_t		index;
	XtId		id;
	ExportType*	type;

public:
	inline XtFeRecordRef() :
		index(0),
		id(0),
		type(0)
	{
	}
};

struct XtFeRefList
{
	XtUt::Array<XtFeRecordRef> refs;
};

struct XtFeTypeRef
{
	size_t		index;
	ExportType*	type;
	bool		stub;

public:
	inline XtFeTypeRef() :
		index(0),
		type(0),
		stub(false)
	{
	}
};

struct XtFeTypeList
{
	XtUt::Array<XtFeTypeRef>	types;
	XtUt::Array<XtFeTypeRef*>	map;
};

#endif // XT_TOOL_XTFLASKEXPORT_H