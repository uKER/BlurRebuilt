#pragma once

namespace Registry
{

class RegKey
{
public:
				RegKey(HKEY parent_key, char* subkey_name);
				~RegKey();
	HKEY		GetKey() {return m_Key;}

	void		SetStringValue(char* valueName, char* value_string);
	void		SetDwordValue(char* valueName, DWORD value);
	void		DeleteValue(char* valueName);
	void		DeleteSubkey(char *subkeyName, bool recursiveDelete );

private:
	BOOL		RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey);
	BOOL		RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey);

public:

	int			m_Result;
	HKEY		m_Key;
};

} // namespace Registry

