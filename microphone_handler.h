#pragma once

#include <Windows.h>


template<class T>
inline void SafeRelease( T **ppObject )
{
	if ( ppObject && *ppObject ) {
		( *ppObject )->Release();
		*ppObject = NULL;
	}
}

HRESULT GetMicrophoneDevCount(
	UINT *pcDevices
);

HRESULT EnumerateMicrophoneDevs(
	UINT	nIndex,
	LPWSTR	lpszBuffer,
	SIZE_T	cchBuffer
);

HRESULT GetMicrophoneVolume(
	LPCWSTR lpszDevName,
	FLOAT	*pfVolumeLevel
);

HRESULT GetMicrophoneMuted(
	LPCWSTR lpszDevName,
	BOOL	*pbMuted
);

HRESULT SetMicrophoneVolume(
	LPCWSTR lpszDevName,
	FLOAT	fVolumeLevel
);

HRESULT SetMicrophoneMuted(
	LPCWSTR lpszDevName,
	BOOL	bMuted
);