#pragma once


DWORD WINAPI AppTrayIconThread( LPVOID );

BOOL SendTrayNotification( LPCWSTR );
BOOL SetTrayIconImage( HICON );
BOOL SetTrayIconTitle( LPCWSTR );