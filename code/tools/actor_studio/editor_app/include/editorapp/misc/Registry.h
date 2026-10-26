#ifndef EDITOR_REGISTRY_H
#define EDITOR_REGISTRY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{

namespace Registry
{
	bool HasValue(const wxString& keyPath, const wxString& key);
	
	template <typename T>
	T Read(const wxString& keyPath, const wxString& key, const T& defaultVal);

	template <>
	wxString Read(const wxString& keyPath, const wxString& key, const wxString& defaultVal);

	template <typename T>
	bool Write(const wxString& keyPath, const wxString& key, const T& value);

	template <>
	bool Write(const wxString& keyPath, const wxString& key, const wxString& value);
}
}

#include "Registry.inl"

#endif
