
// FewPinsPairMfcDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "wclBluetooth.h"

using namespace wclBluetooth;


// CFewPinsPairMfcDlg dialog
class CFewPinsPairMfcDlg : public CDialogEx
{
// Construction
public:
	CFewPinsPairMfcDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FEWPINSPAIRMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CListBox lbLog;

	CwclBluetoothManager* Manager;
	CwclBluetoothRadio* Radio;
	int PinIndex;
	bool TimerRunning;

	void StartTimer();
	void StopTimer();

	void StartPairing();
	void StopPairing();

	void Manager_OnAuthenticationCompleted(void* Sender, CwclBluetoothRadio* const Radio,
		const __int64 Address, const int Error);
	void Manager_OnPinRequest(void* Sender, CwclBluetoothRadio* const Radio,
		const __int64 Address, tstring& Pin);
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStop();
};
