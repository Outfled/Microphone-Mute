#pragma once


//
// Names of Registry Values for Holding the Previous App Instance Data
//

#define APP_REG_VALUE_DEFAULT_DEVICE		L""
#define APP_REG_VALUE_DEFAULT_HOTKEYS		L"HotKeys"
#define APP_REG_VALUE_HK_MODIFIERS			L"Modifiers"
#define APP_REG_VALUE_SOUND_ENABLED			L"SoundEffects"
#define APP_REG_VALUE_NOTIFS_ENABLED		L"Notifications"

#define APP_REG_VALUE_STARTUP_PROGRAM		L"StartWithWindows"

#define REG_STATUS_ENABLED					0x00000001
#define REG_STATUS_DISABLED					0x00000000

LRESULT GetAppRegistryValue( LPCWSTR lpszValueName, LPVOID lpResult, DWORD cbSize );
LRESULT SetAppRegistryValue( LPCWSTR lpszValueName, LPCVOID lpData, DWORD cbSize );
LRESULT SetAppStartupProgram( DWORD dwStatus );
