#include "pch.h"
#include "appreg.h"
#include <strsafe.h>

#define HKEY_APP_BASE_PATH			L"SOFTWARE\\Outfled\\Microphone Mute"
#define HKEY_STARTUP_PROGRAMS_PATH	L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define HKEY_STARTUP_VALUE_NAME		L"Outfled Microphone Mute"


LRESULT GetAppRegistryValue( LPCWSTR lpszValueName, LPVOID lpResult, DWORD cbSize )
{
	LRESULT lResult;
	HKEY	hBaseKey;
	DWORD	dwDisposition;
	DWORD	cbValue;

	/* Open/create the app's registry key */
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER,
		HKEY_APP_BASE_PATH, 
		0,
		NULL,
		0,
		KEY_ALL_ACCESS, 
		NULL,
		&hBaseKey,
		&dwDisposition
	);
	if ( lResult != ERROR_SUCCESS )
	{
		return lResult;
	}

	/* Check the key disposition. If the key was created then the default values must be set */
	if ( dwDisposition == REG_CREATED_NEW_KEY )
	{
		DWORD dwDefault = REG_STATUS_DISABLED;

		RegSetValueEx( hBaseKey,
			APP_REG_VALUE_DEFAULT_DEVICE,
			0,
			REG_SZ,
			(BYTE *)L"",
			sizeof( L"" )
		);
		RegSetValueEx( hBaseKey,
			APP_REG_VALUE_DEFAULT_HOTKEYS,
			0,
			REG_DWORD,
			(BYTE*)&dwDefault,
			sizeof(DWORD)
		);
		RegSetValueEx( hBaseKey,
			APP_REG_VALUE_HK_MODIFIERS,
			0,
			REG_DWORD,
			(BYTE *)&dwDefault,
			sizeof( DWORD )
		);
		RegSetValueEx( hBaseKey,
			APP_REG_VALUE_SOUND_ENABLED,
			0,
			REG_DWORD,
			(BYTE *)&dwDefault,
			sizeof( DWORD )
		);
		RegSetValueEx( hBaseKey,
			APP_REG_VALUE_NOTIFS_ENABLED,
			0,
			REG_DWORD,
			(BYTE *)&dwDefault,
			sizeof( DWORD )
		);
		RegSetValueEx( hBaseKey,
			APP_REG_VALUE_STARTUP_PROGRAM,
			0,
			REG_DWORD,
			(BYTE *)&dwDefault,
			sizeof( DWORD )
		);
	}

	/* Get the key value data */
	lResult = RegQueryValueEx( hBaseKey, lpszValueName, NULL, NULL, (LPBYTE)lpResult, &( cbValue = cbSize ) );

	RegCloseKey( hBaseKey );
	return lResult;
}

LRESULT SetAppRegistryValue( LPCWSTR lpszValueName, LPCVOID lpData, DWORD cbSize )
{
	LRESULT lResult;
	HKEY	hBaseKey;

	/* Open/create the app's registry key */
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER,
		HKEY_APP_BASE_PATH,
		0,
		NULL,
		0,
		KEY_ALL_ACCESS,
		NULL,
		&hBaseKey,
		0
	);
	if ( lResult != ERROR_SUCCESS )
	{
		return lResult;
	}

	/* Set the key value */
	if (0 == wcscmp(lpszValueName, APP_REG_VALUE_DEFAULT_DEVICE )){
		RegSetValueEx( hBaseKey, lpszValueName, 0, REG_SZ, (BYTE *)lpData, cbSize );
	}
	else {
		RegSetValueEx( hBaseKey, lpszValueName, 0, REG_DWORD, (BYTE *)lpData, cbSize );
	}

	RegCloseKey( hBaseKey );
	return lResult;
}

LRESULT SetAppStartupProgram( DWORD dwStatus )
{
	LRESULT lResult;
	HKEY	hBaseKey;
	WCHAR	szAppPath[MAX_PATH + 1];

	/* Get the current app path */
	GetModuleFileName( NULL, szAppPath, ARRAYSIZE( szAppPath ) );

	/* Open the windows startup programs registry key */
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER, HKEY_STARTUP_PROGRAMS_PATH, 0, KEY_ALL_ACCESS, &hBaseKey );
	if ( lResult != ERROR_SUCCESS )
	{
		return lResult;
	}

	/* Set the value */
	switch ( dwStatus )
	{
	case REG_STATUS_ENABLED:
	{
		WCHAR szPathAndArgs[512];
		StringCbPrintf( szPathAndArgs, sizeof( szPathAndArgs ), L"\"%s\" --no-wnd", szAppPath );

		lResult = RegSetValueEx( hBaseKey, HKEY_STARTUP_VALUE_NAME, 0, REG_SZ, (LPBYTE)szPathAndArgs, ( wcslen( szPathAndArgs ) + 1 ) * sizeof( WCHAR ) );
		break;
	}
	case REG_STATUS_DISABLED:
		lResult = RegDeleteValue( hBaseKey, HKEY_STARTUP_VALUE_NAME );
		break;
	}

	RegCloseKey( hBaseKey );
	return lResult;
}