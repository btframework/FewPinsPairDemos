#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include <tchar.h>

#include <stdio.h>
#include <iostream.h>
#include <conio.h>

#include "wclBluetooth.hpp"
#include "wclMessaging.hpp"
#include "wclBluetoothErrors.hpp"
#include "wclErrors.hpp"

using namespace std;

// Change this MAC to your one!!!
const __int64 DEVICE_MAC = 0xB48655ED4B8Cll;
String PINS[] = { "0000", "1234", "1111", "9999" };
const int PINS_COUNT = 4;

class TTestApp
{
private:
	int PinIndex;
	int PairingResult;
	HANDLE PairingCompletedEvent;

	void __fastcall Manager_OnPinRequest(TObject* Sender, TwclBluetoothRadio* const Radio,
		const __int64 Address, String& Pin)
	{
		Pin = _T("");
		if (Address == DEVICE_MAC)
		{
			Pin = PINS[PinIndex];
			wcout << L"Trying to pair using PIN: " << Pin.c_str() << endl;
		}
		else
			wcout << L"Unknwon device " << hex << Address << dec << L" requuests PIN pairing. Ignore" << endl;
	}

	void __fastcall Manager_OnAuthenticationCompleted(TObject* Sender,
		TwclBluetoothRadio* const Radio, const __int64 Address, const int Error)
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
		TwclMessageBroadcaster::SetSyncMethod(skThread);

		TwclBluetoothManager* Manager = new TwclBluetoothManager(NULL);
		Manager->OnPinRequest = Manager_OnPinRequest;
		Manager->OnAuthenticationCompleted = Manager_OnAuthenticationCompleted;

		int Res = Manager->Open();
		if (Res != WCL_E_SUCCESS)
			wcout << L"Unale to open Bluetooth Manager: 0x" << hex << Res << dec << endl;
		else
		{
			TwclBluetoothRadio* Radio = NULL;
			if (Manager->Count == 0)
				wcout << L"No Bluetooth hardware found" << endl;
			else
			{
				// Try to find working radio.
				for (int i = 0; i < Manager->Count; i++)
				{
					if (Manager->Radios[i]->Available)
					{
						Radio = Manager->Radios[i];
						break;
					}
				}

				if (Radio == NULL)
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
			Manager->Free();
		}
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	TTestApp* App = new TTestApp();
	App->Run();

	return 0;
}

