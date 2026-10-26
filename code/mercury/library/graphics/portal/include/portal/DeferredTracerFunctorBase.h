#ifndef PORTAL_DEFERREDTRACERFUNCTORBASE_H
#define PORTAL_DEFERREDTRACERFUNCTORBASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>


namespace Portal
{
	// Call this forward:
	class C_TraceResults;

	//! Deferred tracer call-back functor base class.
	class I_DeferredTracerFunctorBase
	{
	public:
		//! Pure virtual operater, implemented in derived classes.
		//@param rkResults The results of the deferred ray trace.
		virtual void operator () (const C_TraceResults& rkResults) = 0;
	};
}

#endif // PORTAL_DEFERREDTRACERFUNCTORBASE_H
