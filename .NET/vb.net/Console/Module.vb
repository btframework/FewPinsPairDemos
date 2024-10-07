Module Module1
    ' Change this MAC to your one!!!
    Private Const DEVICE_MAC As Int64 = &HB48655ED4B8C
    Private PINS() As String = {"0000", "1234", "1111", "9999"}

    Private PairingCompletedEvent As AutoResetEvent
    Private PairingResult As Int32
    Private PinIndex As Integer

    Sub Main()
        Dim Manager As wclBluetoothManager = New wclBluetoothManager()
        AddHandler Manager.OnPinRequest, AddressOf Manager_OnPinRequest
        AddHandler Manager.OnAuthenticationCompleted, AddressOf Manager_OnAuthenticationCompleted

        Manager.MessageProcessing = wclMessageProcessingMethod.mpAsync
        Dim Res As Int32 = Manager.Open()
        If Res <> wclErrors.WCL_E_SUCCESS Then
            Console.WriteLine("Unale to open Bluetooth Manager: 0x" + Res.ToString("X8"))
        Else
            Dim Radio As wclBluetoothRadio = Nothing
            If Manager.Count = 0 Then
                Console.WriteLine("No Bluetooth hardware found")
            Else
                ' Try to find working radio.
                For i As Int32 = 0 To Manager.Count - 1
                    If Manager(i).Available Then
                        Radio = Manager(i)
                        Exit For
                    End If
                Next i

                If Radio Is Nothing Then
                    Console.WriteLine("No available Bluetooth radio")
                Else
                    Console.WriteLine("Found " + Radio.ApiName + " Bluetooth radio")

                    ' Create pairing completion event.
                    PairingCompletedEvent = New AutoResetEvent(False)
                    ' Reset pairing result.
                    PairingResult = wclErrors.WCL_E_SUCCESS

                    ' Start pairing.
                    Console.WriteLine("Start pairing")
                    For PinIndex = 0 To PINS.Length - 1
                        Res = Radio.RemotePair(DEVICE_MAC)
                        If Res <> wclErrors.WCL_E_SUCCESS Then
                            If Res = wclBluetoothErrors.WCL_E_BLUETOOTH_ALREADY_PAIRED Then
                                Console.WriteLine("Device is already paired")
                            Else
                                Console.WriteLine("Start pairing failed: 0x" + Res.ToString("X8"))
                                Exit For
                            End If
                        Else
                            ' Wait when pairing completed.
                            PairingCompletedEvent.WaitOne()
                            ' Analize pairing result.
                            If PairingResult = wclErrors.WCL_E_SUCCESS Then
                                Console.WriteLine("Paired successfully")
                                Exit For
                            Else
                                Console.WriteLine("Pairing failed: 0x" + PairingResult.ToString("X8"))
                                Console.WriteLine("Try next PIN")
                                Thread.Sleep(5000)
                            End If
                        End If
                    Next

                    If PinIndex = PINS.Length Then Console.WriteLine("Pairing done")

                    ' Do Not forget to close And release pairing completed event.
                    PairingCompletedEvent.Close()
                    PairingCompletedEvent = Nothing

                    ' Do Not forget to clean up radio object.
                    Radio = Nothing
                End If
            End If

            ' Do Not forget to close Bluetooth Manager.
            Manager.Close()
        End If

        Manager = Nothing
    End Sub

    Private Sub Manager_OnAuthenticationCompleted(Sender As Object, Radio As wclBluetoothRadio, Address As Int64, [Error] As Int32)
        If Address <> DEVICE_MAC Then
            Console.WriteLine("Pairing with unknown device " + Address.ToString("X12") + " completed with result: 0x" + [Error].ToString("X8"))
        Else
            PairingResult = [Error]

            If [Error] = wclErrors.WCL_E_SUCCESS Then
                Console.WriteLine("Device has been paired")
            Else
                Console.WriteLine("Pairing with device failed: 0x" + [Error].ToString("X8"))
            End If

            PairingCompletedEvent.Set()
        End If
    End Sub

    Private Sub Manager_OnPinRequest(Sender As Object, Radio As wclBluetoothRadio, Address As Int64, ByRef Pin As String)
        Pin = ""
        If Address = DEVICE_MAC Then
            Pin = PINS(PinIndex)
            Console.WriteLine("Trying to pair using PIN: " + Pin)
        Else
            Console.WriteLine("Unknwon device " + Address.ToString("X12") + " requuests PIN pairing. Ignore")
        End If
    End Sub

End Module
