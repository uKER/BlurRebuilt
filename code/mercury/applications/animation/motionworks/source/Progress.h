#ifndef PROGRESS_H
#define PROGRESS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/misc/Progress.h>

class I_Progress: public Progress::I_ProgressBar
{
public:
	I_Progress( Progress::I_ProgressBar* parent = NULL ):
		Progress::I_ProgressBar(parent)
	{}
	
	virtual ~I_Progress() {}
	
	virtual void SetTitle( const char* title )=0;
	virtual void ErrorMessage( const char* msg )=0;
};

typedef bool (*LongTaskCallback)(I_Progress*, void* context);

bool RunLongTaskRaw(void* parentWindow, LongTaskCallback callback, void* context);

template <typename T>
bool RunLongTask(void* parentWindow, bool (*func)(I_Progress*, T* context), T* context)
{
	return RunLongTaskRaw(parentWindow, (LongTaskCallback) func, context);
}

#endif
