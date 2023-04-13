#include "pch.h"
#include "microphone_handler.h"
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <strsafe.h>

#pragma comment ( lib, "Winmm.lib" ) 

//
// Microphone Device Properties for GetMicrophoneDevProperty()/SetMicrophoneDevProperty()
//

#define MICROPHONE_DEVPROP_VOLUME		0x00000001
#define MICROPHONE_DEVPROP_ISMUTE		0x00000002


CONST CLSID CLSID_MMDeviceEnumerator	= __uuidof( MMDeviceEnumerator );
CONST IID	IID_IMMDeviceEnumerator		= __uuidof( IMMDeviceEnumerator );
CONST IID	IID_IAudioEndpointVolume	= __uuidof( IAudioEndpointVolume );

static HRESULT GetMicrophoneDevCollection( IMMDeviceEnumerator **ppEnum, IMMDeviceCollection **ppCollection )
{
	*ppEnum			= NULL;
	*ppCollection	= NULL;

	HRESULT hResult = CoCreateInstance( CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void **)ppEnum );
	if (FAILED( hResult ))
	{
		return hResult;
	}

	hResult = (*ppEnum)->EnumAudioEndpoints( eCapture, DEVICE_STATE_ACTIVE, ppCollection );
	if (FAILED( hResult ))
	{
		SafeRelease( ppEnum );
		return hResult;
	}

	return hResult;
}

static HRESULT GetMicrophoneDevProperty(
	LPCWSTR lpszDevName,
	DWORD	dwPropType,
	LPVOID	lpPropValue
)
{
	HRESULT					hResult;
	IMMDeviceEnumerator		*pEnumerator;
	IMMDeviceCollection		*pCollection;
	IMMDevice				*pDevice;
	IAudioEndpointVolume	*pAudioEndpoint;

	pDevice	= NULL;

	/* Initialize COM */
	CoInitializeEx( NULL, COINITBASE_MULTITHREADED );

	/* Get microphone device collection & enumerator */
	hResult = GetMicrophoneDevCollection( &pEnumerator, &pCollection );
	if ( hResult != S_OK )
	{
		CoUninitialize();
		return hResult;
	}

	/* Enumerate the devices & find the selected device */
	for ( UINT nIndex = 0; hResult == S_OK; ++nIndex )
	{
		WCHAR szDevName[256] = { 0 };

		/* Get the current device name */
		hResult = EnumerateMicrophoneDevs( nIndex, szDevName, ARRAYSIZE( szDevName ) );
		if ( hResult == S_OK )
		{
			/* Compare names to see if the current device matches the input device */
			if ( 0 == _wcsicmp( szDevName, lpszDevName ) )
			{
				/* Grab the device object */
				hResult = pCollection->Item( nIndex, &pDevice );
				break;
			}
		}
	}

	/* Check if the device was not found */
	if ( hResult != S_OK || pDevice == NULL )
	{
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );

		CoUninitialize();
		return ( hResult == S_OK ) ? E_INVALIDARG : hResult;
	}

	/* Get the IAudioEnpointVolume for the device */
	hResult = pDevice->Activate( IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void **)&pAudioEndpoint );
	if ( hResult != S_OK )
	{
		SafeRelease( &pDevice );
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );

		CoUninitialize();
		return hResult;
	}

	/* Get the property value */
	switch ( dwPropType )
	{
	case MICROPHONE_DEVPROP_VOLUME:
	{
		FLOAT fVolume;

		/* Get the current volume level */
		hResult = pAudioEndpoint->GetChannelVolumeLevelScalar( 0, &fVolume );
		if ( SUCCEEDED( hResult ) )
		{
			*( (FLOAT *)lpPropValue ) = fVolume;
		}
		break;
	}
	case MICROPHONE_DEVPROP_ISMUTE:
	{
		BOOL bMuted;

		/* Check if the device is muted */
		hResult = pAudioEndpoint->GetMute( &bMuted );
		if ( SUCCEEDED( hResult ) ) {
			*( (LPBOOL)lpPropValue ) = bMuted;
		}

		break;
	}
	}

	/* Cleanup */
	SafeRelease( &pAudioEndpoint );
	SafeRelease( &pDevice );
	SafeRelease( &pEnumerator );
	SafeRelease( &pCollection );

	CoUninitialize();
	return hResult;
}

static HRESULT SetMicrophoneDevProperty(
	LPCWSTR lpszDevName,
	DWORD	dwPropType,
	LPVOID	lpPropValue
)
{
	HRESULT					hResult;
	IMMDeviceEnumerator		*pEnumerator;
	IMMDeviceCollection		*pCollection;
	IMMDevice				*pDevice;
	IAudioEndpointVolume	*pAudioEndpoint;

	pDevice	= NULL;

	/* Initialize COM */
	CoInitializeEx( NULL, COINITBASE_MULTITHREADED );

	/* Get microphone device collection & enumerator */
	hResult = GetMicrophoneDevCollection( &pEnumerator, &pCollection );
	if ( hResult != S_OK )
	{
		CoUninitialize();
		return hResult;
	}

	/* Enumerate the devices & find the selected device */
	for ( UINT nIndex = 0; hResult == S_OK; ++nIndex )
	{
		WCHAR szDevName[256] = { 0 };

		/* Get the current device name */
		hResult = EnumerateMicrophoneDevs( nIndex, szDevName, ARRAYSIZE( szDevName ) );
		if ( hResult == S_OK )
		{
			/* Compare names to see if the current device matches the input device */
			if ( 0 == _wcsicmp( szDevName, lpszDevName ) )
			{
				/* Grab the device object */
				hResult = pCollection->Item( nIndex, &pDevice );
				break;
			}
		}
	}

	/* Check if the device was not found */
	if ( hResult != S_OK || pDevice == NULL )
	{
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );

		CoUninitialize();
		return ( hResult == S_OK ) ? E_INVALIDARG : hResult;
	}

	/* Get the IAudioEnpointVolume for the device */
	hResult = pDevice->Activate( IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void **)&pAudioEndpoint );
	if ( hResult != S_OK )
	{
		SafeRelease( &pDevice );
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );

		CoUninitialize();
		return hResult;
	}

	/* Get the property value */
	switch ( dwPropType )
	{
	case MICROPHONE_DEVPROP_VOLUME:
		/* Set the current volume level */
		hResult = pAudioEndpoint->SetChannelVolumeLevelScalar( 0, *(FLOAT *)lpPropValue, NULL );
		break;
	case MICROPHONE_DEVPROP_ISMUTE:
		/* Set the device muted */
		hResult = pAudioEndpoint->SetMute( *(BOOL *)lpPropValue, NULL );
		break;
	}

	/* Cleanup */
	SafeRelease( &pAudioEndpoint );
	SafeRelease( &pDevice );
	SafeRelease( &pEnumerator );
	SafeRelease( &pCollection );

	CoUninitialize();
	return hResult;
}


HRESULT GetMicrophoneDevCount(
	UINT *pcDevices
)
{
	HRESULT				hResult;
	IMMDeviceEnumerator *pEnumerator;
	IMMDeviceCollection *pCollection;

	*pcDevices = 0;

	/* Initialize COM */
	CoInitializeEx( NULL, COINITBASE_MULTITHREADED );

	/* Get microphone device collection & enumerator */
	hResult = GetMicrophoneDevCollection( &pEnumerator, &pCollection );
	if ( hResult != S_OK )
	{
		CoUninitialize();
		return hResult;
	}

	/* Get the number of microphone devices in the collection */
	hResult = pCollection->GetCount( pcDevices );

	/* Release objects */
	SafeRelease( &pEnumerator );
	SafeRelease( &pCollection );

	CoUninitialize();
	return hResult;
}

HRESULT EnumerateMicrophoneDevs(
	UINT	nIndex,
	LPWSTR	lpszBuffer,
	SIZE_T	cchBuffer
)
{
	HRESULT				hResult;
	IMMDeviceEnumerator *pEnumerator;
	IMMDeviceCollection *pCollection;
	IMMDevice			*pDevice;
	IPropertyStore		*pPropStore;
	PROPVARIANT			vDeviceName;

	ZeroMemory( lpszBuffer, cchBuffer * sizeof( WCHAR ) );

	/* Initialize COM */
	CoInitializeEx( NULL, COINITBASE_MULTITHREADED );

	/* Get microphone device collection & enumerator */
	hResult = GetMicrophoneDevCollection( &pEnumerator, &pCollection );
	if ( hResult != S_OK )
	{
		CoUninitialize();
		return hResult;
	}

	/* Get the index device */
	hResult = pCollection->Item( nIndex, &pDevice );
	if ( hResult != S_OK )
	{
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );
		CoUninitialize();
		return hResult;
	}

	/* Open the property store for the device */
	hResult = pDevice->OpenPropertyStore( STGM_READ, &pPropStore );
	if ( hResult != S_OK )
	{
		SafeRelease( &pDevice );
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );

		CoUninitialize();
		return hResult;
	}

	/* Get the device name property */
	hResult = pPropStore->GetValue( PKEY_Device_FriendlyName, &vDeviceName );
	if ( hResult != S_OK )
	{
		SafeRelease( &pPropStore );
		SafeRelease( &pDevice );
		SafeRelease( &pEnumerator );
		SafeRelease( &pCollection );

		CoUninitialize();
		return hResult;
	}

	/* Copy the name to the buffer */
	StringCchCopy( lpszBuffer, cchBuffer, vDeviceName.pwszVal );

	/* Cleanup */
	SafeRelease( &pPropStore );
	SafeRelease( &pDevice );
	SafeRelease( &pEnumerator );
	SafeRelease( &pCollection );

	CoUninitialize();
	return hResult;
}

HRESULT GetMicrophoneVolume(
	LPCWSTR lpszDevName,
	FLOAT	*pfVolumeLevel
)
{
	return GetMicrophoneDevProperty( lpszDevName, MICROPHONE_DEVPROP_VOLUME, pfVolumeLevel );
}

HRESULT GetMicrophoneMuted(
	LPCWSTR lpszDevName,
	BOOL	*pbMuted
)
{
	return GetMicrophoneDevProperty( lpszDevName, MICROPHONE_DEVPROP_ISMUTE, pbMuted );
}

HRESULT SetMicrophoneVolume(
	LPCWSTR lpszDevName,
	FLOAT	fVolumeLevel
)
{
	return SetMicrophoneDevProperty( lpszDevName, MICROPHONE_DEVPROP_VOLUME, &fVolumeLevel );
}

HRESULT SetMicrophoneMuted(
	LPCWSTR lpszDevName,
	BOOL	bMuted
)
{
	return SetMicrophoneDevProperty( lpszDevName, MICROPHONE_DEVPROP_ISMUTE, &bMuted );
}