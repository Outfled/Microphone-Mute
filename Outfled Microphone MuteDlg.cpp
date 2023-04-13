
// OutfledMicrophoneMuteDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Outfled Microphone Mute.h"
#include "Outfled Microphone MuteDlg.h"
#include "afxdialogex.h"
#include "appreg.h"
#include "microphone_handler.h"
#include "apptray.h"
#include <strsafe.h>
#include "microphone_thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Display Error
inline void DisplayAppError( LPCWSTR lpszMessage )
{
	MessageBox( NULL, lpszMessage, L"Error", MB_OK );
}
inline void DisplayAppError( DWORD dwError )
{
	WCHAR szErrorBuf[100];

	LPWSTR lpszErrorBuffer = NULL;
	if ( 0 == FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPWSTR)&lpszErrorBuffer,
		0,
		NULL
	) )
	{
		StringCbPrintf( szErrorBuf, sizeof( szErrorBuf ), L"An unknown error occurred: %d", szErrorBuf );

		DisplayAppError( szErrorBuf );
		return;
	}
	else
	{
		StringCbPrintf( szErrorBuf, sizeof( szErrorBuf ), L"An unknown error occurred: %s", lpszErrorBuffer );
	}

	DisplayAppError( lpszErrorBuffer );
	if ( lpszErrorBuffer ) {
		LocalFree( lpszErrorBuffer );
	}
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() {

}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COutfledMicrophoneMuteDlg dialog



COutfledMicrophoneMuteDlg::COutfledMicrophoneMuteDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OUTFLEDMICROPHONEMUTE_DIALOG, pParent)
{
	// Set the default options until the registry is read
	m_bNotificationsEnabled = 1;
	m_bSoundEnabled			= 0; 
	m_bTrayEnabled			= 0;
	m_dwHotkeys				= 0;
	m_wModifiers			= 0;
	m_bMuted				= 0;
	m_hAppTrayThread		= NULL;
	ZeroMemory( &m_szMicrophoneDev, sizeof( m_szMicrophoneDev ) );

	/* Load the muted icon (default) */
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_MUTED);

	/* Get the default microphone name from the registry */
	LRESULT lResult = GetAppRegistryValue( APP_REG_VALUE_DEFAULT_DEVICE, m_szMicrophoneDev, sizeof( m_szMicrophoneDev ) );
	if ( lResult == ERROR_SUCCESS && wcslen( m_szMicrophoneDev ) > 0 )
	{
		/* Check if the default mic. device is current unmuted */
		HRESULT hResult = GetMicrophoneMuted( m_szMicrophoneDev, &m_bMuted );
		if ( SUCCEEDED( hResult ) && m_bMuted == 0 )
		{
			/* Update the icon to the unmuted icon */
			m_hIcon = AfxGetApp()->LoadIconW( IDI_ICON_UNMUTED );
		}

		/* Check if the default mic. device doesn't exist */
		if ( hResult == E_INVALIDARG )
		{
			ZeroMemory( m_szMicrophoneDev, sizeof( m_szMicrophoneDev ) );
			SetAppRegistryValue( APP_REG_VALUE_DEFAULT_DEVICE, m_szMicrophoneDev, sizeof( m_szMicrophoneDev ) );
		}
	}
}

void COutfledMicrophoneMuteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COutfledMicrophoneMuteDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE( IDC_COMBO_DEVICES, &COutfledMicrophoneMuteDlg::OnCbnSelchangeComboDevices )
	ON_BN_CLICKED( IDC_CHECK_NOTIFICATIONS, &COutfledMicrophoneMuteDlg::OnBnClickedCheckNotifications )
	ON_BN_CLICKED( IDC_CHECK_PLAYSOUND, &COutfledMicrophoneMuteDlg::OnBnClickedCheckPlaysound )
	ON_BN_CLICKED( IDC_CHECK_STARTUPPROGRAM, &COutfledMicrophoneMuteDlg::OnBnClickedCheckStartupprogram )
	ON_BN_CLICKED( IDC_CHECK_CLOSETOTRAY, &COutfledMicrophoneMuteDlg::OnBnClickedCheckClosetotray )
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// COutfledMicrophoneMuteDlg message handlers

BOOL COutfledMicrophoneMuteDlg::OnInitDialog()
{
	CComboBox	*pDeviceDropBox;
	CHotKeyCtrl *pHotKeyCtl;
	UINT		nDevCount;
	HREFTYPE	hResult;
	DWORD		dwStatus;
	HANDLE		hThread;

	pDeviceDropBox	= (CComboBox *)GetDlgItem( IDC_COMBO_DEVICES );
	pHotKeyCtl		= (CHotKeyCtrl *)GetDlgItem( IDC_HOTKEY_SHORTCUTKEYS );

	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	/* Get the number of microphone devices */
	hResult = GetMicrophoneDevCount( &nDevCount );
	if ( hResult != S_OK )
	{
		_com_error cError( hResult );
		DisplayAppError( cError.ErrorMessage() );

		PostQuitMessage( 0 );
	}

	/* Populate the device names dropdown box */
	for ( UINT iDev = 0; iDev < nDevCount; ++iDev )
	{
		WCHAR szDevName[256];

		/* Get the current device name */
		hResult = EnumerateMicrophoneDevs( iDev, szDevName, ARRAYSIZE( szDevName ) );
		if ( SUCCEEDED( hResult ) )
		{
			pDeviceDropBox->AddString( szDevName );
		}
	}

	/* If the default micropone exists, it must be selected in the dropdown box */
	if ( wcslen( m_szMicrophoneDev ) > 0 )
	{
		for ( UINT iDev = 0; iDev < nDevCount; ++iDev )
		{
			CString szDevice;
			
			pDeviceDropBox->GetLBText( iDev, szDevice );
			if ( 0 == wcscmp( szDevice, m_szMicrophoneDev ) )
			{
				pDeviceDropBox->SetCurSel( iDev );
			}
		}
	}

	/* Set the default hotkey */
	GetAppRegistryValue( APP_REG_VALUE_DEFAULT_HOTKEYS, &m_dwHotkeys, sizeof( DWORD ) );
	GetAppRegistryValue( APP_REG_VALUE_HK_MODIFIERS, &m_wModifiers, sizeof( DWORD ) );
	if ( m_dwHotkeys != 0 )
	{
		pHotKeyCtl->SetHotKey( m_dwHotkeys, m_wModifiers );
	}

	/* Set the check box values */
	GetAppRegistryValue( APP_REG_VALUE_NOTIFS_ENABLED, &dwStatus, sizeof( DWORD ) );
	( (CButton *)GetDlgItem( IDC_CHECK_NOTIFICATIONS ) )->SetCheck( dwStatus );

	m_bNotificationsEnabled = (BOOL)dwStatus;

	GetAppRegistryValue( APP_REG_VALUE_SOUND_ENABLED, &dwStatus, sizeof( DWORD ) );
	( (CButton *)GetDlgItem( IDC_CHECK_PLAYSOUND ) )->SetCheck( dwStatus );

	m_bSoundEnabled = (BOOL)dwStatus;

	GetAppRegistryValue( APP_REG_VALUE_STARTUP_PROGRAM, &dwStatus, sizeof( DWORD ) );
	( (CButton *)GetDlgItem( IDC_CHECK_STARTUPPROGRAM ) )->SetCheck( dwStatus );

	/* Always set the 'Close to Tray' checkbox to true */
	( (CButton *)GetDlgItem( IDC_CHECK_CLOSETOTRAY ) )->SetCheck( TRUE );
	m_bTrayEnabled = TRUE;

	/* Create the microphone handler thread */
	hThread = CreateThread( NULL, 0, MicrophoneHandleThread, this, 0, NULL );
	if ( hThread == NULL )
	{
		DisplayAppError( GetLastError() );
		ExitProcess( 0 );
	}

	// Create the tray manager thread
	m_hAppTrayThread = CreateThread( NULL, 0, AppTrayIconThread, this, 0, NULL );
	if ( !m_hAppTrayThread )
	{
		DisplayAppError( GetLastError() );
		ExitProcess( 0 );
	}

	SetForegroundWindow();
	::SetFocus( NULL );

	// return TRUE  unless you set the focus to a control
	return TRUE; 
}

void COutfledMicrophoneMuteDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	// Window is closing
	else if ( nID == SC_CLOSE )
	{
		static bool bFirstTime = TRUE;

		CHotKeyCtrl *pHotKey = ( (CHotKeyCtrl *)GetDlgItem( IDC_HOTKEY_SHORTCUTKEYS ) );

		// Get the current hotkeys
		pHotKey->GetHotKey( (WORD &)m_dwHotkeys, m_wModifiers );

		// Update the registry default hotkeys
		SetAppRegistryValue( APP_REG_VALUE_DEFAULT_HOTKEYS, &m_dwHotkeys, sizeof( DWORD ) );
		SetAppRegistryValue( APP_REG_VALUE_HK_MODIFIERS, &m_wModifiers, sizeof( WORD ) );

		// Hide the window
		if ( m_bTrayEnabled )
		{
			// Send the notification
			if ( bFirstTime )
			{
				bFirstTime = FALSE;
				SendTrayNotification( L"Minimized to Tray" );
			}

			// Hide the window
			ShowWindow( SW_HIDE );
		}
		
		/* 'Close to Tray' is not checked; exit the application */
		else
		{
			CDialogEx::OnSysCommand( nID, lParam );
		}
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COutfledMicrophoneMuteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COutfledMicrophoneMuteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// The system calls this function to obtain the brush to use for the dialog background
HBRUSH COutfledMicrophoneMuteDlg::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	return (HBRUSH)GetStockObject( WHITE_BRUSH );
}

// The system calls this function whenever a value from the devices combo box (dropdown box) is selected
void COutfledMicrophoneMuteDlg::OnCbnSelchangeComboDevices()
{
	BOOL bMuted;

	CComboBox *pDevListComboBox = ( (CComboBox*)GetDlgItem( IDC_COMBO_DEVICES ) );

	// Remove focus from the control
	::SetFocus( NULL );

	// Check if no text is selected
	if ( pDevListComboBox->GetCurSel() == CB_ERR )
	{
		SetAppRegistryValue( APP_REG_VALUE_DEFAULT_DEVICE, L"", sizeof( L"" ) );
		return;
	}

	// Get the device name
	pDevListComboBox->GetLBText( pDevListComboBox->GetCurSel(), m_szMicrophoneDev );

	// Update the default microphone name in the registry
	SetAppRegistryValue( APP_REG_VALUE_DEFAULT_DEVICE, m_szMicrophoneDev, ( wcslen( m_szMicrophoneDev ) + 1 ) * sizeof( WCHAR ) );

	// Update the app icon
	if ( S_OK == GetMicrophoneMuted( m_szMicrophoneDev, &bMuted ) )
	{
		if ( m_bMuted != bMuted )
		{
			m_bMuted = bMuted;

			m_hIcon = AfxGetApp()->LoadIcon( ( bMuted ) ? IDI_ICON_MUTED : IDI_ICON_UNMUTED );
			SetIcon( m_hIcon, TRUE );	// Taskbar icon
			SetIcon( m_hIcon, FALSE );	// Dlg window icon

			UpdateWindow();		
		}
	}
}


// The system calls this function whenever the 'Send Notifications' checkbox is clicked
void COutfledMicrophoneMuteDlg::OnBnClickedCheckNotifications()
{
	DWORD dwStatus;

	CButton *pNotificationsCheckBox = (CButton *)GetDlgItem( IDC_CHECK_NOTIFICATIONS );
	switch ( pNotificationsCheckBox->GetCheck() )
	{
	case 1:
		dwStatus				= REG_STATUS_ENABLED;
		m_bNotificationsEnabled = TRUE;

		break;
	case 0:
		dwStatus				= REG_STATUS_DISABLED;
		m_bNotificationsEnabled = FALSE;
		break;
	}

	// Update the registry
	SetAppRegistryValue( APP_REG_VALUE_NOTIFS_ENABLED, &dwStatus, sizeof( DWORD ) );
}

// The system calls this function whenever the 'Play Sound' checkbox is clicked
void COutfledMicrophoneMuteDlg::OnBnClickedCheckPlaysound()
{
	DWORD dwStatus;

	CButton *pPlaySoundCheckbox = (CButton *)GetDlgItem( IDC_CHECK_PLAYSOUND );
	switch ( pPlaySoundCheckbox->GetCheck() )
	{
	case 1:
		dwStatus		= REG_STATUS_ENABLED;
		m_bSoundEnabled = TRUE;

		break;
	case 0:
		dwStatus		= REG_STATUS_DISABLED;
		m_bSoundEnabled = FALSE;
		break;
	}

	// Update the registry
	SetAppRegistryValue( APP_REG_VALUE_SOUND_ENABLED, &dwStatus, sizeof( DWORD ) );
}


// The system calls this function whenever the 'Startup Program' checkbox is clicked
void COutfledMicrophoneMuteDlg::OnBnClickedCheckStartupprogram()
{
	DWORD dwStatus;

	CButton *pStartupProgCheckbox = (CButton *)GetDlgItem( IDC_CHECK_STARTUPPROGRAM );
	switch ( pStartupProgCheckbox->GetCheck() )
	{
	case 1:
		dwStatus = REG_STATUS_ENABLED;
		break;
	case 0:
		dwStatus = REG_STATUS_DISABLED;
		break;
	}

	// Update the registry
	SetAppRegistryValue( APP_REG_VALUE_STARTUP_PROGRAM, &dwStatus, sizeof( DWORD ) );
	SetAppStartupProgram( dwStatus );
}

// The system calls this function whenever the 'Close to Tray' checkbox is clicked
void COutfledMicrophoneMuteDlg::OnBnClickedCheckClosetotray()
{
	DWORD dwStatus;

	CButton *pPlaySoundCheckbox = (CButton *)GetDlgItem( IDC_CHECK_CLOSETOTRAY );
	switch ( pPlaySoundCheckbox->GetCheck() )
	{
	case 1:
		dwStatus		= REG_STATUS_ENABLED;
		m_bTrayEnabled	= TRUE;

		break;
	case 0:
		dwStatus		= REG_STATUS_DISABLED;
		m_bTrayEnabled	= FALSE;
		break;
	}

	// Update the registry
	SetAppRegistryValue( APP_REG_VALUE_SOUND_ENABLED, &dwStatus, sizeof( DWORD ) );
}
