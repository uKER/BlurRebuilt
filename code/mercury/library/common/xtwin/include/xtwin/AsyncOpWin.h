#ifndef ASYNCOPWIN_H
#define ASYNCOPWIN_H

#ifdef _MSC_VER
#pragma once
#endif

enum AsyncOpIcon
{
	kAoiNone,
	kAoiInfo,
	kAoiWarning,
	kAoiError
};

typedef bool (*AsyncOpCallback)(class AsyncOp*, void*);

class AsyncOp
{
public:
	void NewMessage(AsyncOpIcon icon, const XtString& text);
	void AppendMessage(const XtString& text);

public:
	void SetTitle(const char* title);
	void SetProgress(float fraction);
	void SetStayOpen();

public:
	HWND Window();
	bool IsCancelled();

private:
	AsyncOp(class AsyncOpImpl* impl);
	~AsyncOp();

private:
	class AsyncOpImpl* _impl;

friend class AsyncOpImpl;
};

bool AsyncInvokeEx(HWND parentWindow, AsyncOpCallback callback, void* context, int stackSize, bool noWindow);

template<typename Type>
bool AsyncInvoke(HWND parentWindow, bool (*callback)(class AsyncOp*, Type*), Type* context, int stackSize = 0, bool noWindow = false)
{
	return AsyncInvokeEx(parentWindow, (AsyncOpCallback)callback, context, stackSize, noWindow);
}

#endif // ASYNCOPWIN_H