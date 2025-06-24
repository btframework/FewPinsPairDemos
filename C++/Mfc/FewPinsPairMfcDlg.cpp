
// FewPinsPairMfcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FewPinsPairMfc.h"
#include "FewPinsPairMfcDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Change this MAC to your one!!!
const __int64 DEVICE_MAC = 0xB48655ED4B8C;
tstring PINS[] = { _T("0000"), _T("1234"), _T("1111"), _T("9999") };
const int PINS_COUNT = 4;

CString IntToHex(const int i)
{
	CString s;
	s.Format(_T("%.8X"), i);
	return s;
}

CString IntToHex(const unsigned long i)
{
	CString s;
	s.Format(_T("%.8X"), i);
	return s;
}

CString IntToHex(const __int64 i)
{
	CString s;
	s.Format(_T("%.4X%.8X"), static_cast<INT32>((i >> 32) & 0x00000FFFF),
		static_cast<INT32>(i) & 0xFFFFFFFF);
	return s;
}


// CFewPinsPairMfcDlg dialog



CFewPinsPairMfcDlg::CFewPinsPairMfcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FEWPINSPAIRMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFewPinsPairMfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, lbLog);
}

BEGIN_MESSAGE_MAP(CFewPinsPairMfcDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_START, &CFewPinsPairMfcDlg::OnBnClickedButtonStart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CFewPinsPairMfcDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CFewPinsPairMfcDlg message handlers

BOOL CFewPinsPairMfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	Manager = new CwclBluetoothManager();
	__hook(&CwclBluetoothManager::OnPinRequest, Manager, &CFewPinsPairMfcDlg::Manager_OnPinRequest);
	__hook(&CwclBluetoothManager::OnAuthenticationCompleted, Manager, &CFewPinsPairMfcDlg::Manager_OnAuthenticationCompleted);

	Radio = nullptr;
	PinIndex = 0;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFewPinsPairMfcDlg::OnPaint()
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
HCURSOR CFewPinsPairMfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFewPinsPairMfcDlg::StartTimer()
{
	if (TimerRunning)
	{
		SetTimer(100, 5000, NULL);
		TimerRunning = true;
	}
}

void CFewPinsPairMfcDlg::StopTimer()
{
	if (TimerRunning)
	{
		KillTimer(100);
		TimerRunning = false;
	}
}

void CFewPinsPairMfcDlg::StartPairing()
{
	if (Radio != nullptr)
		lbLog.AddString(_T("Pairing already running"));
	else
	{
		lbLog.ResetContent();

		int Res = Manager->Open();
		if (Res != WCL_E_SUCCESS)
			lbLog.AddString(_T("Unable to open Bluetooth Manager: 0x") + IntToHex(Res));
		else
		{
			if (Manager->Count == 0)
				lbLog.AddString(_T("Bluetooth hardware not found"));
			else
			{
				for (size_t i = 0; i < Manager->Count; i++)
				{
					if (Manager->Radios[i]->Available)
					{
						Radio = Manager->Radios[i];
						break;
					}
				}

				if (Radio == nullptr)
					lbLog.AddString(_T("No available Bluetooth radio found"));
				else
				{
					lbLog.AddString(_T("Found ") + CString(Radio->ApiName.c_str()) + _T(" Bluetooth radio"));
					PinIndex = 0;
					StartTimer();
					lbLog.AddString(_T("Start pairing"));
				}
			}

			if (Radio == nullptr)
				Manager->Close();
		}
	}
}

void CFewPinsPairMfcDlg::StopPairing()
{
	if (Radio == nullptr)
		lbLog.AddString(_T("Pairing is not running"));
	else
	{
		StopTimer();

		Radio = nullptr;
		Manager->Close();
	}
}

void CFewPinsPairMfcDlg::Manager_OnAuthenticationCompleted(void* Sender, CwclBluetoothRadio* const Radio,
	__int64 Address, int Error)
{
	UNREFERENCED_PARAMETER(Sender);
	UNREFERENCED_PARAMETER(Radio);
	
	if (Address != DEVICE_MAC)
		lbLog.AddString(_T("Pairing with unknown device ") + IntToHex(Address) + _T(" completed with result: 0x") + IntToHex(Error));
	else
	{
		if (Error == WCL_E_SUCCESS)
		{
			lbLog.AddString(_T("Device has been paired"));
			StopPairing();
		}
		else
		{
			lbLog.AddString(_T("Pairing with device failed: 0x") + IntToHex(Error));
			if (Radio != nullptr)
			{
				PinIndex++;
				if (PinIndex == PINS_COUNT)
				{
					lbLog.AddString(_T("No more PINs"));
					StopPairing();
				}
				else
				{
					lbLog.AddString(_T("Try next PIN"));
					StartTimer();
				}
			}
		}
	}
}

void CFewPinsPairMfcDlg::Manager_OnPinRequest(void* Sender, CwclBluetoothRadio* const Radio,
	const __int64 Address, tstring& Pin)
{
	UNREFERENCED_PARAMETER(Sender);
	UNREFERENCED_PARAMETER(Radio);

	Pin = _T("");
	if (Address == DEVICE_MAC)
	{
		Pin = PINS[PinIndex];
		lbLog.AddString(_T("Trying to pair using PIN: ") + CString(Pin.c_str()));
	}
	else
		lbLog.AddString(_T("Unknwon device ") + IntToHex(Address) + _T(" requuests PIN pairing. Ignore"));
}

void CFewPinsPairMfcDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	StopPairing();

	__unhook(Manager);
	delete Manager;
}

void CFewPinsPairMfcDlg::OnBnClickedButtonStart()
{
	StartPairing();
}

void CFewPinsPairMfcDlg::OnTimer(UINT_PTR nIDEvent)
{
	StopTimer();

	if (Radio != nullptr)
	{
		int Res = Radio->RemotePair(DEVICE_MAC);
		if (Res != WCL_E_SUCCESS)
		{
			if (Res == WCL_E_BLUETOOTH_ALREADY_PAIRED)
				lbLog.AddString(_T("Device already paired"));
			else
				lbLog.AddString(_T("Unable to start pairing with device: 0x") + IntToHex(Res));
			StopPairing();
		}
		else
			lbLog.AddString(_T("Try to pair with device"));
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CFewPinsPairMfcDlg::OnBnClickedButtonStop()
{
	StopPairing();
}
