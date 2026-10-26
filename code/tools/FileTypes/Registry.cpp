#include "stdafx.h"

#include <shlwapi.h>

namespace Registry
{
//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegKey::RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
//			printf("Key not found.\n");
            return TRUE;
        } 
        else {
//			printf("Error opening key.\n");
			return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do {

            lstrcpy (lpEnd, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegKey::RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[2 * MAX_PATH];

    lstrcpy (szDelKey, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

RegKey::RegKey(HKEY parent_key, char* subkeyName)
{
    SECURITY_ATTRIBUTES security;
    
    security.nLength = sizeof( security );
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = TRUE;
    
    HKEY hSubKey = 0;
    DWORD disposition = 0;

	m_Result = RegCreateKeyEx(
			parent_key, 
			subkeyName, 
			0, 
			NULL, 
			REG_OPTION_NON_VOLATILE, 
			KEY_ALL_ACCESS,
			&security,
			&m_Key,
			&disposition
			);
	
}

void RegKey::SetStringValue(char* valueName, char* valueString)
{
	if ( m_Result == ERROR_SUCCESS )
	{
		// REG_CREATED_NEW_KEY = 1, REG_OPENED_EXISTING_KEY = 2
		RegSetValueEx(
			m_Key,								// handle to key
			valueName,							// value name (NULL = default)
			0,									// reserved
			REG_SZ,								// value type
			(const BYTE *)valueString,			// value data
			(DWORD)(strlen(valueString) + 1)	// size of value data
		);
	}
}

void RegKey::SetDwordValue(char* valueName, DWORD value)
{
	if ( m_Result == ERROR_SUCCESS )
	{
		// REG_CREATED_NEW_KEY = 1, REG_OPENED_EXISTING_KEY = 2
	    RegSetValueEx(
			m_Key,					// handle to key
			valueName,				// value name (NULL = default)
			0,						// reserved
			REG_DWORD,				// value type
			(const BYTE *)&value,	// value data
			sizeof(DWORD)			// size of value data
		);
	}
}

void RegKey::DeleteValue(char* valueName)
{
	if( m_Result == ERROR_SUCCESS )
	{
		RegDeleteValue( m_Key, valueName );
	}
}

void RegKey::DeleteSubkey(char *subkeyName, bool recusiveDelete )
{
	if( m_Result == ERROR_SUCCESS )
	{
		if(recusiveDelete)
		{
			RegDelnode( m_Key, subkeyName );
		}
		else
		{
			RegDeleteKey( m_Key, subkeyName );
		}
	}
}

RegKey::~RegKey()
{
	if(m_Result == ERROR_SUCCESS)
	{
		RegCloseKey( m_Key );
	}
}

} // namespace Registry