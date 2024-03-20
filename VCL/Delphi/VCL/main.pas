unit main;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants,
  System.Classes, Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs,
  wclBluetooth, Vcl.ExtCtrls, Vcl.StdCtrls;

type
  TfmMain = class(TForm)
    btStart: TButton;
    btStop: TButton;
    lbLog: TListBox;
    tiWait: TTimer;
    BluetoothManager: TwclBluetoothManager;
    procedure FormCreate(Sender: TObject);
    procedure BluetoothManagerAuthenticationCompleted(Sender: TObject;
      const Radio: TwclBluetoothRadio; const Address: Int64;
      const Error: Integer);
    procedure BluetoothManagerPinRequest(Sender: TObject;
      const Radio: TwclBluetoothRadio; const Address: Int64; out Pin: string);
    procedure FormDestroy(Sender: TObject);
    procedure btStartClick(Sender: TObject);
    procedure tiWaitTimer(Sender: TObject);
    procedure btStopClick(Sender: TObject);

  private
    FRadio: TwclBluetoothRadio;
    FPinIndex: Integer;
    procedure StartPairing;
    procedure StopPairing;
  end;

var
  fmMain: TfmMain;

implementation

uses
  wclErrors, wclBluetoothErrors;

{$R *.dfm}

// Change this MAC to your one!!!
const
  DEVICE_MAC = $B48655ED4B8C;
  PINS: array [0..3] of string = ('0000', '1234', '1111', '9999');
  PINS_COUNT = 4;

{ TfmMain }

procedure TfmMain.BluetoothManagerAuthenticationCompleted(Sender: TObject;
  const Radio: TwclBluetoothRadio; const Address: Int64; const Error: Integer);
begin
  if Address <> DEVICE_MAC then begin
    lbLog.Items.Add('Pairing with unknown device ' +
      IntToHex(Address, 12) + ' completed with result: 0x' +
      IntToHex(Error, 8));

  end else begin
    if Error = WCL_E_SUCCESS then begin
			lbLog.Items.Add('Device has been paired');
			StopPairing;

    end else begin
      lbLog.Items.Add('Pairing with device failed: 0x' + IntToHex(Error, 8));
      if FRadio <> nil then begin
        Inc(FPinIndex);
        if FPinIndex = PINS_COUNT then begin
					lbLog.Items.Add('No more PINs');
					StopPairing;

        end else begin
					lbLog.Items.Add('Try next PIN');
					tiWait.Enabled := True;
        end;
      end;
    end;
  end;
end;

procedure TfmMain.BluetoothManagerPinRequest(Sender: TObject;
  const Radio: TwclBluetoothRadio; const Address: Int64; out Pin: string);
begin
  Pin := '';
	if Address = DEVICE_MAC then begin
		Pin := PINS[FPinIndex];
		lbLog.Items.Add('Trying to pair using PIN: ' + Pin);

  end else begin
		lbLog.Items.Add('Unknwon device ' + IntToHex(Address, 12) +
      ' requuests PIN pairing. Ignore');
	end;
end;

procedure TfmMain.btStartClick(Sender: TObject);
begin
  StartPairing;
end;

procedure TfmMain.btStopClick(Sender: TObject);
begin
  StopPairing;
end;

procedure TfmMain.FormCreate(Sender: TObject);
begin
  FRadio := nil;
	FPinIndex := 0;
end;

procedure TfmMain.FormDestroy(Sender: TObject);
begin
  StopPairing;
end;

procedure TfmMain.StartPairing;
var
  Res: Integer;
  i: Integer;
begin
  if FRadio <> nil then
		lbLog.Items.Add('Pairing already running')

	else begin
    lbLog.Items.Clear;

    Res := BluetoothManager.Open;
    if Res <> WCL_E_SUCCESS then begin
      lbLog.Items.Add('Unable to open Bluetooth Manager: 0x' +
        IntToHex(Res, 8));

    end else begin
      if BluetoothManager.Count = 0 then
        lbLog.Items.Add('Bluetooth hardware not found')

      else begin
        for i := 0 to BluetoothManager.Count -1 do begin
          if BluetoothManager[i].Available then begin
            FRadio := BluetoothManager[i];
            Break;
          end;
        end;

        if FRadio = nil then
          lbLog.Items.Add('No available Bluetooth radio found')
        else begin
          lbLog.Items.Add('Found ' + FRadio.ApiName + ' Bluetooth radio');
          FPinIndex := 0;
          tiWait.Enabled := True;
          lbLog.Items.Add('Start pairing');
        end;
      end;

      if FRadio = nil then
        BluetoothManager.Close;
    end;
  end;
end;

procedure TfmMain.StopPairing;
begin
  if FRadio = nil then
    lbLog.Items.Add('Pairing is not running')

  else begin
    tiWait.Enabled := False;

    FRadio := nil;
    BluetoothManager.Close;
  end;
end;

procedure TfmMain.tiWaitTimer(Sender: TObject);
var
  Res: Integer;
begin
  tiWait.Enabled := False;

	if FRadio <> nil then begin
    Res := FRadio.RemotePair(DEVICE_MAC);
    if Res <> WCL_E_SUCCESS then begin
      if Res = WCL_E_BLUETOOTH_ALREADY_PAIRED then
        lbLog.Items.Add('Device already paired')

      else begin
        lbLog.Items.Add('Unable to start pairing with device: 0x' +
          IntToHex(Res, 8));
      end;
			StopPairing;

		end else
			lbLog.Items.Add('Try to pair with device');
	end;
end;

end.
