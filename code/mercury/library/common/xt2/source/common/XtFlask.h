#ifndef XTFLASK_H
#define XTFLASK_H

#ifdef _MSC_VER
#pragma once
#endif

const XtU32		kXtFlaskTag		= 'FLSK';
const size_t	kXtFlaskAlign	= 4;

struct FlaskBlob
{
	XtU32 offset;
	XtU32 count;
};

struct FlaskHeader
{
	XtU32 tag;
	XtU32 version;

	FlaskBlob types;
	FlaskBlob bases;
	FlaskBlob fields;
	FlaskBlob strings;
	FlaskBlob refs;
	FlaskBlob records;
	FlaskBlob components;
	FlaskBlob data;
};

struct FlaskType
{
	XtU16	name;
	XtU16	fieldCount;
	XtU16	baseCount;
	XtU16	behaviour;
	XtU16	atom;
	XtU16	size;
};

struct FlaskBase
{
	XtU16 type;
	XtU16 offset;
};

struct FlaskField
{
	XtU16	name;
	XtU16	type;
	XtU16	offset;
	XtU8	behaviour;
	XtU8	array;
};

struct FlaskRef
{
	XtId	id;
	XtU16	type;
	XtU16	record;
};

struct FlaskRecord
{
	XtU32 ref;
	XtU32 componentCount;
	XtU32 dataBytes;
	XtU32 stringBytes;
};

struct FlaskComponent
{
	XtU16 type;
	XtU16 count;
	XtU16 behaviour;
};

struct FlaskArray
{
	XtU16 component;
	XtU16 element;
	XtU32 length;
};

struct FlaskPtr
{
	XtU16 component;
	XtU16 element;
};

struct FlaskHandle
{
	XtId id;
};

void XtFlaskDecodeStrings(XtU8* __restrict dst, const XtU8* __restrict src, int count);
void XtFlaskEncodeStrings(XtU8* __restrict data, int count);

#endif // XTFLASK_H
