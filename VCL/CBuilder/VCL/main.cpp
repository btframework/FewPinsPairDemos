//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "wclBluetooth"
#pragma resource "*.dfm"
TfmMain *fmMain;
//---------------------------------------------------------------------------
// Change this MAC to your one!!!
const __int64 DEVICE_MAC = 0xB48655ED4B8Cll;
String PINS[] = { "0000", "1234", "1111", "9999" };
const int PINS_COUNT = 4;
//---------------------------------------------------------------------------
__fastcall TfmMain::TfmMain(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::StartPairing()
{
	if (FRadio != NULL)
		lbLog->Items->Add("Pairing already running");
	else
	{
		lbLog->Items->Clear();

		int Res = BluetoothManager->Open();
		if (Res != WCL_E_SUCCESS)
		{
			lbLog->Items->Add("Unable to open Bluetooth Manager: 0x" +
				IntToHex(Res, 8));
		}
		else
		{
			if (BluetoothManager->Count == 0)
				lbLog->Items->Add("Bluetooth hardware not found");
			else
			{
				for (int i = 0; i < BluetoothManager->Count; i++)
				{
				if (BluetoothManager->Radios[i]->Available)
				{
					FRadio = BluetoothManager->Radios[i];
					break;
				}
			}

			if (FRadio == NULL)
				lbLog->Items->Add("No available Bluetooth radio found");
			else
			{
				lbLog->Items->Add("Found " + FRadio->ApiName +
					" Bluetooth radio");
				FPinIndex = 0;
				tiWait->Enabled = true;
				lbLog->Items->Add("Start pairing");
			}
		}

		if (FRadio == NULL)
			BluetoothManager->Close();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::StopPairing()
{
	if (FRadio == NULL)
		lbLog->Items->Add("Pairing is not running");
	else
	{
		tiWait->Enabled = false;

		FRadio = NULL;
		BluetoothManager->Close();
	}
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::FormCreate(TObject *Sender)
{
	FRadio = NULL;
	FPinIndex = 0;
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::BluetoothManagerAuthenticationCompleted(
	TObject *Sender, TwclBluetoothRadio * const Radio, const __int64 Address,
	const int Error)

{
	if (Address != DEVICE_MAC)
	{
		lbLog->Items->Add("Pairing with unknown device " +
			IntToHex(Address, 12) + " completed with result: 0x" +
			IntToHex(Error, 8));
	}
	else
	{
		if (Error == WCL_E_SUCCESS)
		{
			lbLog->Items->Add("Device has been paired");
			StopPairing();
		}
		else
		{
			lbLog->Items->Add("Pairing with device failed: 0x" +
				IntToHex(Error, 8));
			if (FRadio != NULL)
			{
				FPinIndex++;
				if (FPinIndex == PINS_COUNT)
				{
					lbLog->Items->Add("No more PINs");
					StopPairing();
				}
				else
				{
					lbLog->Items->Add("Try next PIN");
					tiWait->Enabled = true;
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::BluetoothManagerPinRequest(TObject *Sender,
	TwclBluetoothRadio * const Radio, const __int64 Address, UnicodeString Pin)
{
	Pin = "";
	if (Address == DEVICE_MAC)
	{
		Pin = PINS[FPinIndex];
		lbLog->Items->Add("Trying to pair using PIN: " + Pin);
	}
	else
	{
		lbLog->Items->Add("Unknwon device " + IntToHex(Address, 12) +
			" requuests PIN pairing. Ignore");
	}
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::FormDestroy(TObject *Sender)
{
	StopPairing();
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::btStartClick(TObject *Sender)
{
	StartPairing();
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::tiWaitTimer(TObject *Sender)
{
	tiWait->Enabled = false;

	if (FRadio != NULL)
	{
		int Res = FRadio->RemotePair(DEVICE_MAC);
		if (Res != WCL_E_SUCCESS)
		{
			if (Res == WCL_E_BLUETOOTH_ALREADY_PAIRED)
				lbLog->Items->Add("Device already paired");
			else
			{
				lbLog->Items->Add("Unable to start pairing with device: 0x" +
					IntToHex(Res, 8));
			}
			StopPairing();
		}
		else
			lbLog->Items->Add("Try to pair with device");
	}
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::btStopClick(TObject *Sender)
{
	StopPairing();
}
//---------------------------------------------------------------------------

