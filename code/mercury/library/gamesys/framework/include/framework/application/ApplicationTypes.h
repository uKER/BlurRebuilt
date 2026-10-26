#ifndef FRAMEWORK_APPLICATION_TYPES_H
#define FRAMEWORK_APPLICATION_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3

typedef U32				HRESULT;

#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)

#define E_FAIL			_HRESULT_TYPEDEF_(0x80004005L)

#define S_OK			((HRESULT)0x00000000L)
#define S_FALSE			((HRESULT)0x00000001L)

#undef FAILED
#define FAILED(hr)		((HRESULT)(hr) < 0)

#undef SUCCEEDED
#define SUCCEEDED(hr)	((HRESULT)(hr) >= 0)

#endif // PLATFORM_PS3

#endif // FRAMEWORK_APPLICATION_TYPES_H
