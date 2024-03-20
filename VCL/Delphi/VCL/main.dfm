object fmMain: TfmMain
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'Few PINs WinForm Test App'
  ClientHeight = 309
  ClientWidth = 645
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object btStart: TButton
    Left = 8
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Start'
    TabOrder = 0
    OnClick = btStartClick
  end
  object btStop: TButton
    Left = 89
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Stop'
    TabOrder = 1
    OnClick = btStopClick
  end
  object lbLog: TListBox
    Left = 8
    Top = 39
    Width = 629
    Height = 262
    ItemHeight = 13
    TabOrder = 2
  end
  object tiWait: TTimer
    Enabled = False
    Interval = 5000
    OnTimer = tiWaitTimer
    Left = 304
    Top = 24
  end
  object BluetoothManager: TwclBluetoothManager
    OnAuthenticationCompleted = BluetoothManagerAuthenticationCompleted
    OnPinRequest = BluetoothManagerPinRequest
    Left = 376
    Top = 8
  end
end
