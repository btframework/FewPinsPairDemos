//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include "wclBluetooth.hpp"
//---------------------------------------------------------------------------
class TfmMain : public TForm
{
__published:	// IDE-managed Components
	TButton *btStart;
	TButton *btStop;
	TListBox *lbLog;
	TTimer *tiWait;
	TwclBluetoothManager *BluetoothManager;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BluetoothManagerAuthenticationCompleted(TObject *Sender, TwclBluetoothRadio * const Radio,
          const __int64 Address, const int Error);
	void __fastcall BluetoothManagerPinRequest(TObject *Sender, TwclBluetoothRadio * const Radio,
          const __int64 Address, UnicodeString Pin);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall btStartClick(TObject *Sender);
	void __fastcall tiWaitTimer(TObject *Sender);
	void __fastcall btStopClick(TObject *Sender);
private:	// User declarations
	TwclBluetoothRadio* FRadio;
	int FPinIndex;
	void __fastcall StartPairing();
	void __fastcall StopPairing();
public:		// User declarations
	__fastcall TfmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfmMain *fmMain;
//---------------------------------------------------------------------------
#endif
