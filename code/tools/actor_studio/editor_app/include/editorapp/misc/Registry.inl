#include <wx/msw/registry.h>

namespace Editor
{
namespace Registry
{

inline bool HasValue(const wxString& keyPath, const wxString& key)
{
	wxRegKey regPath(keyPath);
	if (!regPath.Exists() || !regPath.HasValue(key))
		return false;

	return true;
}
	
template <typename T>
T Read(const wxString& keyPath, const wxString& key, const T& defaultVal)
{
	wxRegKey regPath(keyPath);
	if (!regPath.Exists())
		return defaultVal;

	wxMemoryBuffer buffer;
	buffer.SetBufSize(sizeof(T));
	
	if (!regPath.HasValue(key) || !regPath.QueryValue(key, buffer))
		return defaultVal;

	assert(sizeof(T) == buffer.GetDataLen());
	T returnVal;
	returnVal = *reinterpret_cast<T*>(buffer.GetData());
	return returnVal;
}

template <>
inline wxString Read(const wxString& keyPath, const wxString& key, const wxString& defaultVal)
{
	wxRegKey regPath(keyPath);
	if (!regPath.Exists())
		return defaultVal;

	wxString returnVal;
	if (!regPath.HasValue(key) || !regPath.QueryValue(key, returnVal))
		return defaultVal;

	return returnVal;
}

template <typename T>
bool Write(const wxString& keyPath, const wxString& key, const T& value)
{
	wxRegKey regPath(keyPath);
	if (!regPath.Exists() && !regPath.Create())
		return false;

	wxMemoryBuffer buffer;
	memcpy(buffer.GetWriteBuf(sizeof(T)), &value, sizeof(T));
	buffer.UngetWriteBuf(sizeof(T));

	if (!regPath.SetValue(key, buffer))
		return false;

	return true;
}

template <>
inline bool Write(const wxString& keyPath, const wxString& key, const wxString& value)
{
	wxRegKey regPath(keyPath);
	if (!regPath.Exists() && !regPath.Create())
		return false;

	if (!regPath.SetValue(key, value))
		return false;

	return true;
}

}
}