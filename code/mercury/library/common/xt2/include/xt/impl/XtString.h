#ifndef XT_IMPL_XTSTRING_H
#define XT_IMPL_XTSTRING_H

#ifdef _MSC_VER
#pragma once
#endif

typedef Sz8 XtString;

extern const Sz8	kXtNullString;
extern const Sz16	kXtNullStringW;

//
// XtString functions
//

XtString XtsEncodeString(const char* text);
XtString XtsDecodeString(const char* text, int length);
bool XtsIsWhitespace(char c);
bool XtsIsIdentifierStart(char c);
bool XtsIsIdentifierBody(char c);
bool XtsIsNameBody(char c);
bool XtsIsIdentifier(const char* text);
bool XtsIsName(const char* text);
bool XtsSplit(const char* text, char c, XtString& before, XtString& after);
XtString XtsStripHead(const char* text, char c);
XtString XtsStripTail(const char* text, char c);
XtString XtsStripBoth(const char* text, char c);
bool XtsSplitFileName(const char* fullFilename, XtString& path, XtString& file, XtString& extension);
bool XtsSplitObjectFileName(const char* fullFilename, XtString& path, XtString& file, XtString& type);
bool XtsSplitObjectName(const char* fullFilename, XtString& path, XtString& type);
bool XtsAppend(XtString& text, const char* toAppend, int length);
bool XtsIsValidFileName(const XtString& fileName);
int XtsDecodeHexDigit(char c);
Sz16 XtsDecodeUnicodeText(XtString string);

#endif // XT_IMPL_XTSTRING_H