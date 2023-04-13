#include "pch.h"
#include "apptray.h"
#include "Outfled Microphone MuteDlg.h"
#include "microphone_thread.h"
#include <strsafe.h>


#define TRAY_WINDOW_CLASS_NAME	L"__hidden__"
#define TRAY_WINDOW_NAME		L"Outfled Microphone Mute"
#define TRAY_HOVER_MESSAGE		TRAY_WINDOW_NAME
#define TRAY_ICON_CALLBACK		WM_APP + 1
#define TRAY_MENU_ID_OPEN_APP	100 + 1
#define TRAY_MENU_ID_EXIT_APP	100 + 2
#define TRAY_MENU_ID_MUTE_MIC	100 + 3
#define TRAY_MENU_ID_UNMUTE_MIC	100 + 4

static HWND							g_hTrayWindow;
static WNDCLASSEX					g_WindowClass;
static HMODULE						g_hInstance;
static COutfledMicrophoneMuteDlg	*g_pAppWindow;
static PNOTIFYICONDATA				g_pIconData;

static LRESULT TrayIconProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	/* Create tray icon */
	if ( uMsg == WM_CREATE )
	{
		g_pIconData						= (PNOTIFYICONDATA)LocalAlloc( LPTR, sizeof( NOTIFYICONDATA ) );
		g_pIconData->cbSize				= sizeof( NOTIFYICONDATA );
		g_pIconData->hWnd				= hWnd;
		g_pIconData->uID				= 0;
		g_pIconData->uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_INFO;
		g_pIconData->uCallbackMessage	= TRAY_ICON_CALLBACK;
		g_pIconData->uVersion			= NOTIFYICON_VERSION_4;
		g_pIconData->dwInfoFlags		= NIIF_INFO;
		g_pIconData->hIcon				= g_pAppWindow->GetIcon( TRUE );

		/* Notification title */
		StringCbCopy( g_pIconData->szInfoTitle, sizeof( g_pIconData->szInfoTitle ), L"Outfled Microphone Mute" );

		/* Add icon to shell */
		Shell_NotifyIcon( NIM_ADD, g_pIconData );
		Shell_NotifyIconW( NIM_SETVERSION, g_pIconData );

		/* Set the hover message */
		if ( g_pAppWindow->m_bMuted ) {
			SetTrayIconTitle( L"Outfled Microphone Mute (Muted)" );
		}
		else {
			SetTrayIconTitle( L"Outfled Microphone Mute (Un-muted)" );
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	/* Tray icon message */
	if ( uMsg == TRAY_ICON_CALLBACK )
	{
		/* Left click on icon */
		if ( LOWORD( lParam ) == WM_LBUTTONUP )
		{
			/* Show the main app window */
			g_pAppWindow->ShowWindow( SW_SHOW );
			SetForegroundWindow( g_pAppWindow->GetSafeHwnd() );
		}

		/* Popup menu */
		else if ( LOWORD( lParam ) == WM_CONTEXTMENU )
		{
			POINT			Cursor{};
			HMENU			PopupMenu;
			MENUITEMINFO	ItemInfo;
			UINT			iResult;

			ItemInfo.cbSize = sizeof( ItemInfo );
			ItemInfo.fMask	= MIIM_FTYPE;
			ItemInfo.fType	= MFT_SEPARATOR;
			PopupMenu		= CreatePopupMenu();

			/* Create popup menu */
			InsertMenu( PopupMenu,
				0,
				( g_pAppWindow->IsWindowVisible() ) ? MF_GRAYED : 0 | MF_STRING | MF_BYCOMMAND,
				TRAY_MENU_ID_OPEN_APP,
				L"Show Window"
			);
			InsertMenuItem( PopupMenu, 1, TRUE, &ItemInfo );
			InsertMenu( PopupMenu,
				2,
				( g_pAppWindow->m_bMuted ) ? MF_GRAYED | MF_GRAYED : MF_BYCOMMAND,
				TRAY_MENU_ID_MUTE_MIC,
				L"Mute Microphone"
			);
			InsertMenuItem( PopupMenu, 3, TRUE, &ItemInfo );
			InsertMenu( PopupMenu,
				4,
				( g_pAppWindow->m_bMuted ) ? MF_BYCOMMAND | MF_BYCOMMAND : MF_GRAYED,
				TRAY_MENU_ID_UNMUTE_MIC,
				L"Unmute Microphone"
			);
			InsertMenuItem( PopupMenu, 5, TRUE, &ItemInfo );
			InsertMenu( PopupMenu,
				6,
				MF_BYCOMMAND,
				TRAY_MENU_ID_EXIT_APP,
				L"Exit"
			);


			/* Set the foreground window to the tray window */
			SetForegroundWindow( g_hTrayWindow );

			/* Get the mouse cursor position */
			GetCursorPos( &Cursor );

			/* Display the popup menu & get the clicked item id */
			iResult = TrackPopupMenu( PopupMenu,
				TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_VERNEGANIMATION | TPM_VERPOSANIMATION | TPM_RETURNCMD,
				Cursor.x,
				Cursor.y,
				0,
				g_hTrayWindow,
				NULL
			);
			switch ( iResult )
			{
			case TRAY_MENU_ID_OPEN_APP:
				g_pAppWindow->ShowWindow( SW_SHOW );
				break;

			case TRAY_MENU_ID_EXIT_APP:
				Shell_NotifyIcon( NIM_DELETE, g_pIconData );
				PostQuitMessage( 0 );

				return 0;

			case TRAY_MENU_ID_MUTE_MIC:
				SetEvent( g_hTrayIconMicrophoneMuted );
				break;

			case TRAY_MENU_ID_UNMUTE_MIC:
				SetEvent( g_hTrayIconMicrophoneUnmuted );
				break;

			default:
				break;
			}
		}
	}

	/* Destroy tray icon */
	if ( uMsg == WM_DESTROY )
	{
		Shell_NotifyIcon( NIM_DELETE, g_pIconData );
		PostQuitMessage( 0 );

		return 0;
	}

	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

DWORD WINAPI AppTrayIconThread( LPVOID lpData )
{
	MSG	WindowMsg;

	g_pAppWindow	= (COutfledMicrophoneMuteDlg *)lpData;
	g_hTrayWindow	= NULL;
	g_hInstance		= GetModuleHandle( NULL );
	g_pIconData		= NULL;
	g_WindowClass	= { 0 };

	/* Create the tray window class */
	g_WindowClass.cbSize		= sizeof( WNDCLASSEX );
	g_WindowClass.style			= CS_HREDRAW | CS_VREDRAW;
	g_WindowClass.lpfnWndProc	= TrayIconProcedure;
	g_WindowClass.hInstance		= g_hInstance;
	g_WindowClass.lpszClassName = TRAY_WINDOW_CLASS_NAME;
	if ( !RegisterClassEx( &g_WindowClass ) )
	{
		ExitThread( GetLastError() );
	}

	/* Create the tray shell icon window (hidden) */
	g_hTrayWindow = CreateWindow(TRAY_WINDOW_CLASS_NAME,
		TRAY_WINDOW_NAME,
		WS_OVERLAPPED,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		NULL,
		NULL,
		g_hInstance,
		NULL
	);
	if ( g_hTrayWindow == NULL )
	{
		DWORD dwError = GetLastError();

		UnregisterClass( TRAY_WINDOW_CLASS_NAME, g_hInstance );
		DisplayAppError( dwError );

		ExitProcess( 0 );
	}

	/* Get all tray icon messages */
	while ( GetMessage( &WindowMsg, NULL, 0, 0 ) )
	{
		Sleep( 0 );

		TranslateMessage( &WindowMsg );
		DispatchMessageW( &WindowMsg );

		if ( g_pAppWindow->m_bTrayEnabled == FALSE )
		{
			PostQuitMessage( 0 );
			break;
		}
	}

	/* Cleanup */
	UnregisterClassW( TRAY_WINDOW_CLASS_NAME, g_hInstance );
	LocalFree( g_pIconData );

	ExitProcess( 0 );
	ExitThread( 0 );
}

BOOL SendTrayNotification( LPCWSTR lpszMessage )
{
	if ( g_pIconData )
	{
		StringCbCopy( g_pIconData->szInfo, sizeof( g_pIconData->szInfo ), lpszMessage );
		return Shell_NotifyIcon( NIM_MODIFY, g_pIconData );
	}

	return FALSE;
}

BOOL SetTrayIconImage( HICON hIcon )
{
	if ( g_pIconData )
	{
		/* Clear any previous notification string */
		ZeroMemory( g_pIconData->szInfo, sizeof( g_pIconData ) );

		g_pIconData->hIcon = hIcon;
		return Shell_NotifyIcon( NIM_MODIFY, g_pIconData );
	}

	return FALSE;
}

BOOL SetTrayIconTitle( LPCWSTR lpszTitle )
{
	if ( g_pIconData )
	{
		/* Clear any previous notification string */
		ZeroMemory( g_pIconData->szInfo, sizeof( g_pIconData ) );

		StringCbCopy( g_pIconData->szTip, sizeof( g_pIconData->szTip ), lpszTitle );
		return Shell_NotifyIcon( NIM_MODIFY, g_pIconData );
	}

	return FALSE;
}