#pragma once


// Global Events 
extern HANDLE g_hTrayIconMicrophoneMuted;
extern HANDLE g_hTrayIconMicrophoneUnmuted;

DWORD WINAPI MicrophoneHandleThread( LPVOID );