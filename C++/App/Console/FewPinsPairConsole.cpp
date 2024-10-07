// FewPinsPairConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#include "wclBluetooth.h"

using namespace std;
using namespace wclBluetooth;

// Change this MAC to your one!!!
const __int64 DEVICE_MAC = 0xB48655ED4B8C;
tstring PINS[] = { _T("0000"), _T("1234"), _T("1111"), _T("9999") };
const int PINS_COUNT = 4;

class CTestApp
{
private:
	int PinIndex;
	int PairingResult;
	HANDLE PairingCompletedEvent;

	void Manager_OnPinRequest(void* Sender, CwclBluetoothRadio* const Radio,
		const __int64 Address, tstring& Pin)
	{
		UNREFERENCED_PARAMETER(Sender);
		UNREFERENCED_PARAMETER(Radio);

		Pin = _T("");
		if (Address == DEVICE_MAC)
		{
			Pin = PINS[PinIndex];
			wcout << L"Trying to pair using PIN: " << Pin.c_str() << endl;
		}
		else
			wcout << L"Unknwon device " << hex << Address << dec << L" requuests PIN pairing. Ignore" << endl;
	}

	void Manager_OnAuthenticationCompleted(void* Sender, CwclBluetoothRadio* const Radio,
		const __int64 Address, const int Error)
	{
		UNREFERENCED_PARAMETER(Sender);
		UNREFERENCED_PARAMETER(Radio);

		if (Address != DEVICE_MAC)
			wcout << L"Pairing with unknown device " << hex << Address << L" completed with result: 0x" << Error << dec << endl;
		else
		{
			PairingResult = Error;

			if (Error == WCL_E_SUCCESS)
				wcout << L"Device has been paired" << endl;
			else
				wcout << L"Pairing with device failed: 0x" << hex << Error << dec << endl;

			SetEvent(PairingCompletedEvent);
		}
	}

public:
	void Run()
	{
		// Switch to "console synchronization mode".
		CwclBluetoothManager* Manager = new CwclBluetoothManager();
		Manager->MessageProcessing = mpAsync;
		__hook(&CwclBluetoothManager::OnPinRequest, Manager, &CTestApp::Manager_OnPinRequest);
		__hook(&CwclBluetoothManager::OnAuthenticationCompleted, Manager, &CTestApp::Manager_OnAuthenticationCompleted);
		
		int Res = Manager->Open();
		if (Res != WCL_E_SUCCESS)
			wcout << L"Unale to open Bluetooth Manager: 0x" << hex << Res << dec << endl;
		else
		{
			CwclBluetoothRadio* Radio = nullptr;
			if (Manager->Count == 0)
				wcout << L"No Bluetooth hardware found" << endl;
			else
			{
				// Try to find working radio.
				for (unsigned int i = 0; i < Manager->Count; i++)
				{
					if (Manager->Radios[i]->Available)
					{
						Radio = Manager->Radios[i];
						break;
					}
				}
				
				if (Radio == nullptr)
					wcout << L"No available Bluetooth radio" << endl;
				else
				{
					wcout << L"Found " << Radio->ApiName.c_str() << L" Bluetooth radio" << endl;
					
					// Create pairing completion event.
					PairingCompletedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
					// Reset pairing result.
					PairingResult = WCL_E_SUCCESS;
					
					// Start pairing.
					wcout << L"Start pairing" << endl;
					for (PinIndex = 0; PinIndex < PINS_COUNT; PinIndex++)
					{
						Res = Radio->RemotePair(DEVICE_MAC);
						if (Res != WCL_E_SUCCESS)
						{
							if (Res == WCL_E_BLUETOOTH_ALREADY_PAIRED)
								wcout << L"Device is already paired" << endl;
							else
								wcout << L"Start pairing failed: 0x" << hex << Res << dec << endl;
							break;
						}
						else
						{
							// Wait when pairing completed.
							WaitForSingleObject(PairingCompletedEvent, INFINITE);
							// Analize pairing result.
							if (PairingResult == WCL_E_SUCCESS)
							{
								wcout << L"Paired successfully" << endl;
								break;
							}
							else
							{
								wcout << L"Pairing failed: 0x" << hex << PairingResult << dec << endl;
								wcout << L"Try next PIN" << endl;
								Sleep(5000);
							}
						}
					}
					
					if (PinIndex == PINS_COUNT)
						wcout << L"Pairing done" << endl;
					
					// Do not forget to close and release pairing completed event.
					CloseHandle(PairingCompletedEvent);
				}
			}
			
			// Do not forget to close Bluetooth Manager.
			Manager->Close();
			__unhook(Manager);
			delete Manager;
		}
	}
};


int main()
{
	CTestApp* App = new CTestApp();
	App->Run();

    return 0;
}
