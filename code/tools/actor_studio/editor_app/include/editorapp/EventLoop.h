#ifndef EDITOR_EVENTLOOP_H
#define EDITOR_EVENTLOOP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{

	class C_EventLoop : public wxEventLoop
	{
	public:
		//trivial ctor
		C_EventLoop();
		
		//dtor
		virtual ~C_EventLoop();

	public:
		//*** wxEventLoop (msw) overrides ***///

		virtual bool Pending() const;
		virtual bool Dispatch();

		virtual bool PreProcessMessage(WXMSG *msg);

		// process a single message
		virtual void ProcessMessage(WXMSG *msg);
	};
}

#endif