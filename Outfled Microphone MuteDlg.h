// OutfledMicrophoneMuteDlg.h : header file
//

#pragma once


// COutfledMicrophoneMuteDlg dialog
class COutfledMicrophoneMuteDlg : public CDialogEx
{
// Construction
public:
	COutfledMicrophoneMuteDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OUTFLEDMICROPHONEMUTE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	HANDLE	m_hAppTrayThread;

public:
	BOOL	m_bNotificationsEnabled;
	BOOL	m_bSoundEnabled;
	BOOL	m_bTrayEnabled;
	DWORD	m_dwHotkeys;
	WORD	m_wModifiers;
	WCHAR	m_szMicrophoneDev[256];
	BOOL	m_bMuted;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboDevices();
	afx_msg void OnBnClickedCheckNotifications();
	afx_msg void OnBnClickedCheckPlaysound();
	afx_msg void OnBnClickedCheckStartupprogram();
	afx_msg void OnBnClickedCheckClosetotray();

};

// Display Error
inline void DisplayAppError( LPCWSTR lpszMessage );
inline void DisplayAppError( DWORD dwError );