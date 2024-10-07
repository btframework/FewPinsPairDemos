program FewPinsPairConsoleDelphi;

{$APPTYPE CONSOLE}

{$R *.res}

uses
  System.SysUtils, wclBluetooth, wclErrors, Windows, wclMessaging,
  wclBluetoothErrors;

// Change this MAC to your one!!!
const
  DEVICE_MAC = $B48655ED4B8C;
  PINS: array [0..3] of string = ('0000', '1234', '1111', '9999');
  PINS_COUNT = 4;

type
  TTestApp = class
  private
    FPinIndex: Integer;
    FPairingResult: Integer;
    FPairingCompletedEvent: THandle;

    procedure Manager_OnPinRequest(Sender: TObject;
      const Radio: TwclBluetoothRadio; const Address: Int64; out Pin: string);
    procedure Manager_OnAuthenticationCompleted(Sender: TObject;
      const Radio: TwclBluetoothRadio; const Address: Int64;
      const Error: Integer);

  public
    procedure Run;
end;

procedure TTestApp.Manager_OnPinRequest(Sender: TObject;
  const Radio: TwclBluetoothRadio; const Address: Int64; out Pin: string);
begin
  Pin := '';
  if Address = DEVICE_MAC then begin
    Pin := PINS[FPinIndex];
    Writeln('Trying to pair using PIN: ' + Pin);

  end else begin
    Writeln('Unknwon device ' + IntToHex(Address, 12) +
      ' requuests PIN pairing. Ignore');
  end;
end;

procedure TTestApp.Manager_OnAuthenticationCompleted(Sender: TObject;
  const Radio: TwclBluetoothRadio; const Address: Int64; const Error: Integer);
begin
  if Address <> DEVICE_MAC then begin
    Writeln('Pairing with unknown device ' + IntToHex(Address, 12) +
      ' completed with result: 0x' + IntToHex(Error, 8));
  end else begin
    FPairingResult := Error;

    if Error = WCL_E_SUCCESS then
      Writeln('Device has been paired')
    else
      Writeln('Pairing with device failed: 0x' + IntToHex(Error, 12));

    SetEvent(FPairingCompletedEvent);
  end;
end;

procedure TTestApp.Run;
var
  Manager: TwclBluetoothManager;
  Res: Integer;
  Radio: TwclBluetoothRadio;
  i: Integer;
begin
  // Switch to "console synchronization mode".
  Manager := TwclBluetoothManager.Create(nil);
  Manager.MessageProcessing := mpAsync;
  Manager.OnPinRequest := Manager_OnPinRequest;
  Manager.OnAuthenticationCompleted := Manager_OnAuthenticationCompleted;

  Res := Manager.Open;
  if Res <> WCL_E_SUCCESS then
    Writeln('Unale to open Bluetooth Manager: 0x' + IntToHex(Res, 8))

  else begin
    Radio := nil;
    if Manager.Count = 0 then
      Writeln('No Bluetooth hardware found')

    else begin
      // Try to find working radio.
      for i := 0 to Manager.Count - 1 do begin
        if Manager[i].Available then begin
          Radio := Manager[i];
          Break;
        end;
      end;

      if Radio = nil then
        Writeln('No available Bluetooth radio')

      else begin
        Writeln('Found ' + Radio.ApiName + ' Bluetooth radio');

        // Create pairing completion event.
        FPairingCompletedEvent := CreateEvent(nil, False, False, nil);
        // Reset pairing result.
        FPairingResult := WCL_E_SUCCESS;

        // Start pairing.
        Writeln('Start pairing');
        FPinIndex := 0;
        while FPinIndex < PINS_COUNT do begin
          Res := Radio.RemotePair(DEVICE_MAC);
          if Res <> WCL_E_SUCCESS then begin
            if Res = WCL_E_BLUETOOTH_ALREADY_PAIRED then
              Writeln('Device is already paired')
            else
              Writeln('Start pairing failed: 0x' + IntToHex(Res, 8));
            Break;

          end else begin
            // Wait when pairing completed.
            WaitForSingleObject(FPairingCompletedEvent, INFINITE);
            // Analize pairing result.
            if FPairingResult = WCL_E_SUCCESS then begin
              Writeln('Paired successfully');
              Break;

            end else begin
              Writeln('Pairing failed: 0x' + IntToHex(FPairingResult, 8));
              Writeln('Try next PIN');
              Sleep(5000);
            end;
          end;
          Inc(FPinIndex);
        end;

        if FPinIndex = PINS_COUNT then
          Writeln('Pairing done');

        // Do not forget to close and release pairing completed event.
        CloseHandle(FPairingCompletedEvent);
      end;
    end;

    // Do not forget to close Bluetooth Manager.
    Manager.Close;
    Manager.Free;
  end;
end;

begin
  try
    with TTestApp.Create do
      Run;
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
end.
