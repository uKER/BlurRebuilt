#ifndef UTIL_RESOURCE_RESOURCE_H
#define UTIL_RESOURCE_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <System/DebugLog.h>

#include <btl/containers/vector.h>
#include <btl/Containers/list.h>

#include <resource/ResourceTypes.h>
#include <resource/TypeRegistry.h>
#include <resource/Handle.h>
#include <resource/Listener.h>
#include <resource/ResourceBase.h>
#include <resource/Manager.h>

//! @defgroup Resource Resource Module
//! The Resource module manages the loading of game resources from disk.
//!
//! \par Overview
//! The resource system manages the loading of resources. Its main functions are 
//!		- to provide an asynchronous loading system for game assets.
//!		- to prevent loading the same resource twice.
//!		- to allow the order of loaded files to be sorted alphabetically.
//!		- to help simplify the management of resource loading and deletion.
//!		- to provide a system for reloading of resources. - this makes most sense to be driven by the actors themselves so an actor can reload its resource data and this can be triggered by an artist somehow.
//!
//! For some examples of syn
//!
//! See ..\\..\\readme.txt

//!
//! @addtogroup Resource
//! @{

//! The Resource namespace contains the code for the Resource module.
namespace Resource
{

extern Debug::Logger &g_Log;

} // namespace Resource

//! @}

#include <resource/Handle.inl>
#include <resource/Manager.inl>

#endif

