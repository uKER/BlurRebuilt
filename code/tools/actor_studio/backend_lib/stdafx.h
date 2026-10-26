#ifdef _MSC_VER_
#pragma once
#endif

#ifndef EDITOR_LIB_WIP_STDAFX_H
#define EDITOR_LIB_WIP_STDAFX_H

//MFC, used only for serialisation helpers.
#define WINVER			0x0501
#define _WIN32_WINNT	0x0501

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// for artery
#include <winsock2.h>
#include <wincrypt.h>

#include <string>

// shared container classes
#include <core/core.h>
#include <core/containers.h>
#include <core/maths.h>
#include <core/utmaths.h>
#include <core/utility/utility.h>
#include <core/utility/singleton.h>
#include <core/utility/smartptr.h>
#include <core/utility/Delegate.h>
#include <btl/containers/vector.h>
#include <btl/containers/openhashmap.h>
#include <btl/containers/openhashset.h>
#include <btl/containers/list.h>
#include <btl/utility/string.h>
#include <Btl/Utility/Hashops.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <System/Misc/StringUtil.h>
#include <System/Misc/BtlStringUtil.h>

#include <Misc_Shared/Variant/Variant.h>
#include <Misc_Shared/Variant/KeyValue.h>
#include <Misc_Shared/Factory/Factory.h>

#pragma warning(disable:4355)
#include <xt/xttool.h>

#include <EditorLib/Misc/NodeId.h>
#include <EditorLib/Misc/Definitions.h>
#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>

#include <EditorLib/Xt/XtDefinitions.h>

#include <Tools_Shared/Shim/Shim.h>

#endif
