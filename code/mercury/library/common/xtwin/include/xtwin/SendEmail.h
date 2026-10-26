#ifndef SENDEMAIL_H
#define SENDEMAIL_H

#ifdef _MSC_VER
#pragma once
#endif

void SendEmail(const XtString& subject, const XtString& body);

XtString GetExeBuildTime();

bool RecycleFile(const XtString& path);
bool SetClipboardText(HWND hwnd, const XtString& text);
bool GetClipboardText(HWND hwnd, XtString& text);

Gdiplus::Bitmap* CacheBitmap(XtString filename);
void FreeBitmapCache();

#endif // SENDEMAIL_H