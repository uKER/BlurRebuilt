#ifndef XT_IMPL_XTLOCFILE_H
#define XT_IMPL_XTLOCFILE_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_PS3
const XtU32 kXtLocTag = 'XTLO';
#else
const XtU32 kXtLocTag = 'X' << 24 | 'T' << 16 | 'L' << 8 | 'O';
#endif
struct XtLocBlock
{
	XtU32 offset;
	XtU32 size;
};

struct XtLocHeader
{
	XtU32		tag;
	XtU32		version;
	XtLocBlock	toc;
};

struct XtLocToc
{
	XtU32 langOffset;
	XtU32 langCount;
	XtU32 lookupOffset;
};

struct XtLocLang
{
	char		name[16];
	XtLocBlock	data;
};

struct XtLocTable
{
	XtU32 count;
	XtU32 offset;
};

struct XtLocEntry
{
	XtU32 id;
	XtU32 offset;
};

#endif // XT_IMPL_XTLOCFILE_H
