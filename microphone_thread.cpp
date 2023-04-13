#include "pch.h"
#include "microphone_thread.h"
#include "Outfled Microphone MuteDlg.h"
#include "apptray.h"
#include "microphone_handler.h"
#include <strsafe.h>
#include "resource.h"
#include <Mmsystem.h>

#define CheckKeyPressedStatus(vKeyCode)		( GetKeyState(vKeyCode) & 0x8000 )

//
// Sound Effect File Paths
//
#define SE_FILE_MUTED						L"Sound\\muted.wav"
#define SE_FILE_UNMUTED						L"Sound\\unmuted.wav"


static inline VOID OnMicrophoneMute( COutfledMicrophoneMuteDlg *pWndDlg )
{
	if ( pWndDlg->m_bSoundEnabled ) {
		PlaySound( SE_FILE_MUTED, NULL, SND_FILENAME | SND_NODEFAULT );
	}
	if ( pWndDlg->m_bNotificationsEnabled ) {
		SendTrayNotification( L"Muted" );
	}

	SetTrayIconTitle( L"Outfled Microphone Mute (Muted)" );
	SetTrayIconImage( AfxGetApp()->LoadIcon( IDI_ICON_MUTED ));

	pWndDlg->SetIcon( AfxGetApp()->LoadIcon( IDI_ICON_MUTED ), TRUE );
	pWndDlg->SetIcon( AfxGetApp()->LoadIcon( IDI_ICON_MUTED ), FALSE );

	pWndDlg->UpdateWindow();
}
static inline VOID OnMicrophoneUnmute( COutfledMicrophoneMuteDlg *pWndDlg )
{
	if ( pWndDlg->m_bSoundEnabled ) {
		PlaySound( SE_FILE_UNMUTED, NULL, SND_FILENAME | SND_NODEFAULT );
	}
	if ( pWndDlg->m_bNotificationsEnabled ) {
		SendTrayNotification( L"Un-muted" );
	}


	SetTrayIconTitle( L"Outfled Microphone Mute (Un-Muted)" );
	SetTrayIconImage( AfxGetApp()->LoadIcon( IDI_ICON_UNMUTED ) );

	pWndDlg->SetIcon( AfxGetApp()->LoadIcon( IDI_ICON_UNMUTED ), TRUE );
	pWndDlg->SetIcon( AfxGetApp()->LoadIcon( IDI_ICON_UNMUTED ), FALSE );

	pWndDlg->UpdateWindow();
}

HANDLE g_hTrayIconMicrophoneMuted;
HANDLE g_hTrayIconMicrophoneUnmuted;

DWORD WINAPI MicrophoneHandleThread( LPVOID lpData )
{
	COutfledMicrophoneMuteDlg *pWndDlg = (COutfledMicrophoneMuteDlg *)lpData;

	// Create global events
	g_hTrayIconMicrophoneMuted 	= CreateEvent( NULL, TRUE, FALSE, NULL );
	if ( !g_hTrayIconMicrophoneMuted ) {
		DisplayAppError( GetLastError() );
		ExitProcess( 0 );
	}

	g_hTrayIconMicrophoneUnmuted = CreateEvent( NULL, TRUE, FALSE, NULL );
	if ( !g_hTrayIconMicrophoneUnmuted ) {
		DisplayAppError( GetLastError() );
		ExitProcess( 0 );
	}

	/* Main loop */
	while ( 1 )
	{
		DWORD dwMuteEventStatus;
		DWORD dwUnmuteEventStatus;

		// Add a pause to prevent high CPU usage
		Sleep( 30 );

		// No microphone device selected
		if ( 0 == wcslen( pWndDlg->m_szMicrophoneDev ) )
		{
			continue;
		}

		/* Check the global signals */
		dwMuteEventStatus	= WaitForSingleObject( g_hTrayIconMicrophoneMuted, 0 );
		dwUnmuteEventStatus = WaitForSingleObject( g_hTrayIconMicrophoneUnmuted, 0 );

		/* Check if the toggle key has been pressed or */
		if ( CheckKeyPressedStatus( pWndDlg->m_dwHotkeys ) )
		{
			BOOL bModifiers = TRUE;

			/* Check the modifiers */
			if ( pWndDlg->m_wModifiers & HOTKEYF_ALT )
			{
				if ( 0 == CheckKeyPressedStatus( VK_MENU ) ) {
					bModifiers = FALSE;
				}
			}
			if ( pWndDlg->m_wModifiers & HOTKEYF_CONTROL )
			{
				if ( 0 == CheckKeyPressedStatus( VK_CONTROL ) ) {
					bModifiers = FALSE;
				}
			}
			if ( pWndDlg->m_wModifiers & HOTKEYF_SHIFT )
			{
				if ( 0 == CheckKeyPressedStatus( VK_SHIFT ) ) {
					bModifiers = FALSE;
				}
			}

			if ( bModifiers == TRUE )
			{
				BOOL	bMuted;
				HRESULT hResult;

				/* Get the microphone mute status */
				hResult = GetMicrophoneMuted( pWndDlg->m_szMicrophoneDev, &bMuted );
				if ( hResult == S_OK )
				{
					/* Mute/unmute microphone */
					hResult = SetMicrophoneMuted( pWndDlg->m_szMicrophoneDev, !bMuted );
					if ( hResult == S_OK )
					{
						pWndDlg->m_bMuted = !bMuted;
						if ( pWndDlg->m_bMuted ) {
							OnMicrophoneMute( pWndDlg );
						}
						else {
							OnMicrophoneUnmute( pWndDlg );
						}
					}
				}

				if ( hResult != S_OK )
				{
					_com_error cError( hResult );
					DisplayAppError( cError.ErrorMessage() );
				}

				ResetEvent( g_hTrayIconMicrophoneMuted );
				ResetEvent( g_hTrayIconMicrophoneUnmuted );

				/* Wait for toggle keys to not be pressed */
				while ( CheckKeyPressedStatus( pWndDlg->m_dwHotkeys ) ) {
					Sleep( 1 );
				}

				continue;
			}
		}

		/* Check if the 'Mute' button was clicked from the tray icon menu */ 
		if ( dwMuteEventStatus == WAIT_OBJECT_0 )
		{
			HRESULT hResult;

			/* Mute the microphone */
			hResult = SetMicrophoneMuted( pWndDlg->m_szMicrophoneDev, 1 );
			if ( SUCCEEDED( hResult ) )
			{
				pWndDlg->m_bMuted = TRUE;
				OnMicrophoneMute( pWndDlg );
			}
			else
			{
				_com_error cError( hResult );
				DisplayAppError( cError.ErrorMessage() );
			}

			ResetEvent( g_hTrayIconMicrophoneMuted );
			ResetEvent( g_hTrayIconMicrophoneUnmuted );
		}

		/* Check if the 'Unmute' button was clicked from the tray icon menu */
		if ( dwUnmuteEventStatus == WAIT_OBJECT_0 )
		{
			HRESULT hResult;

			/* Mute the microphone */
			hResult = SetMicrophoneMuted( pWndDlg->m_szMicrophoneDev, 0 );
			if ( SUCCEEDED( hResult ) )
			{
				pWndDlg->m_bMuted = FALSE;
				OnMicrophoneUnmute( pWndDlg );
			}
			else
			{
				_com_error cError( hResult );
				DisplayAppError( cError.ErrorMessage() );
			}

			ResetEvent( g_hTrayIconMicrophoneMuted );
			ResetEvent( g_hTrayIconMicrophoneUnmuted );
		}
	}
}