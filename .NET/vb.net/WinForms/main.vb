Public Class fmMain
    ' Change this MAC to your one!!!
    Private Const DEVICE_MAC As Int64 = &HB48655ED4B8C
    Private PINS() As String = {"0000", "1234", "1111", "9999"}

    Private WithEvents Manager As wclBluetoothManager
    Private Radio As wclBluetoothRadio
    Private PinIndex As Int32

    Private Sub StartPairing()
        If Radio IsNot Nothing Then
            lbLog.Items.Add("Pairing already running")
        Else
            lbLog.Items.Clear()

            Dim Res As Int32 = Manager.Open()
            If Res <> wclErrors.WCL_E_SUCCESS Then
                lbLog.Items.Add("Unable to open Bluetooth Manager: 0x" + Res.ToString("X8"))
            Else
                If Manager.Count = 0 Then
                    lbLog.Items.Add("Bluetooth hardware not found")
                Else
                    For i As Int32 = 0 To Manager.Count - 1
                        If Manager(i).Available Then
                            Radio = Manager(i)
                            Exit For
                        End If
                    Next i

                    If Radio Is Nothing Then
                        lbLog.Items.Add("No available Bluetooth radio found")
                    Else
                        lbLog.Items.Add("Found " + Radio.ApiName + " Bluetooth radio")
                        PinIndex = 0
                        tiWait.Start()
                        lbLog.Items.Add("Start pairing")
                    End If
                End If

                If Radio Is Nothing Then Manager.Close()
            End If
        End If
    End Sub

    Private Sub StopPairing()
        If Radio Is Nothing Then
            lbLog.Items.Add("Pairing is not running")
        Else
            tiWait.Stop()

            Radio = Nothing
            Manager.Close()
        End If
    End Sub

    Private Sub fmMain_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Manager = New wclBluetoothManager()

        Radio = Nothing
        PinIndex = 0
    End Sub

    Private Sub Manager_OnAuthenticationCompleted(Sender As Object, Radio As wclBluetoothRadio, Address As Long, [Error] As Integer) Handles Manager.OnAuthenticationCompleted
        If Address <> DEVICE_MAC Then
            lbLog.Items.Add("Pairing with unknown device " + Address.ToString("X12") + " completed with result: 0x" + [Error].ToString("X8"))
        Else
            If [Error] = wclErrors.WCL_E_SUCCESS Then
                lbLog.Items.Add("Device has been paired")
                StopPairing()
            Else
                lbLog.Items.Add("Pairing with device failed: 0x" + [Error].ToString("X8"))
                If Radio IsNot Nothing Then
                    PinIndex = PinIndex + 1
                    If PinIndex = PINS.Length Then
                        lbLog.Items.Add("No more PINs")
                        StopPairing()
                    Else
                        lbLog.Items.Add("Try next PIN")
                        tiWait.Start()
                    End If
                End If
            End If
        End If
    End Sub

    Private Sub Manager_OnPinRequest(Sender As Object, Radio As wclBluetoothRadio, Address As Long, ByRef Pin As String) Handles Manager.OnPinRequest
        Pin = ""
        If Address = DEVICE_MAC Then
            Pin = PINS(PinIndex)
            lbLog.Items.Add("Trying to pair using PIN: " + Pin)
        Else
            lbLog.Items.Add("Unknwon device " + Address.ToString("X12") + " requuests PIN pairing. Ignore")
        End If
    End Sub

    Private Sub fmMain_FormClosed(sender As Object, e As FormClosedEventArgs) Handles MyBase.FormClosed
        StopPairing()
    End Sub

    Private Sub btStart_Click(sender As Object, e As EventArgs) Handles btStart.Click
        StartPairing()
    End Sub

    Private Sub tiWait_Tick(sender As Object, e As EventArgs) Handles tiWait.Tick
        tiWait.Stop()

        If Radio IsNot Nothing Then
            Dim Res As Int32 = Radio.RemotePair(DEVICE_MAC)
            If Res <> wclErrors.WCL_E_SUCCESS Then
                If Res = wclBluetoothErrors.WCL_E_BLUETOOTH_ALREADY_PAIRED Then
                    lbLog.Items.Add("Device already paired")
                Else
                    lbLog.Items.Add("Unable to start pairing with device: 0x" + Res.ToString("X8"))
                End If
                StopPairing()
            Else
                lbLog.Items.Add("Try to pair with device")
            End If
        End If
    End Sub

    Private Sub btStop_Click(sender As Object, e As EventArgs) Handles btStop.Click
        StopPairing()
    End Sub
End Class
