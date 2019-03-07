using System;
using System.Threading;

using wclCommon;
using wclBluetooth;

namespace FewPinsConsole
{
    class Program
    {
        // Change this MAC to your one!!!
        private const Int64 DEVICE_MAC = 0xB48655ED4B8C;
        private static String[] PINS = { "0000", "1234", "1111", "9999" };

        private static AutoResetEvent PairingCompletedEvent;
        private static Int32 PairingResult;
        private static int PinIndex;

        static void Main(string[] args)
        {
            // Switch to "console synchronization mode".
            wclMessageBroadcaster.SetSyncMethod(wclMessageSynchronizationKind.skNone);

            wclBluetoothManager Manager = new wclBluetoothManager();
            Manager.OnPinRequest += new wclBluetoothPinRequestEvent(Manager_OnPinRequest);
            Manager.OnAuthenticationCompleted += new wclBluetoothDeviceResultEvent(Manager_OnAuthenticationCompleted);

            Int32 Res = Manager.Open();
            if (Res != wclErrors.WCL_E_SUCCESS)
                Console.WriteLine("Unale to open Bluetooth Manager: 0x" + Res.ToString("X8"));
            else
            {
                wclBluetoothRadio Radio = null;
                if (Manager.Count == 0)
                    Console.WriteLine("No Bluetooth hardware found");
                else
                {
                    // Try to find working radio.
                    for (Int32 i = 0; i < Manager.Count; i++)
                    {
                        if (Manager[i].Available)
                        {
                            Radio = Manager[i];
                            break;
                        }
                    }

                    if (Radio == null)
                        Console.WriteLine("No available Bluetooth radio");
                    else
                    {
                        Console.WriteLine("Found " + Radio.ApiName + " Bluetooth radio");

                        // Create pairing completion event.
                        PairingCompletedEvent = new AutoResetEvent(false);
                        // Reset pairing result.
                        PairingResult = wclErrors.WCL_E_SUCCESS;

                        // Start pairing.
                        Console.WriteLine("Start pairing");
                        for (PinIndex = 0; PinIndex < PINS.Length; PinIndex++)
                        {
                            Res = Radio.RemotePair(DEVICE_MAC);
                            if (Res != wclErrors.WCL_E_SUCCESS)
                            {
                                if (Res == wclBluetoothErrors.WCL_E_BLUETOOTH_ALREADY_PAIRED)
                                    Console.WriteLine("Device is already paired");
                                else
                                    Console.WriteLine("Start pairing failed: 0x" + Res.ToString("X8"));
                                break;
                            }
                            else
                            {
                                // Wait when pairing completed.
                                PairingCompletedEvent.WaitOne();
                                // Analize pairing result.
                                if (PairingResult == wclErrors.WCL_E_SUCCESS)
                                {
                                    Console.WriteLine("Paired successfully");
                                    break;
                                }
                                else
                                {
                                    Console.WriteLine("Pairing failed: 0x" + PairingResult.ToString("X8"));
                                    Console.WriteLine("Try next PIN");
                                    Thread.Sleep(5000);
                                }
                            }
                        }

                        if (PinIndex == PINS.Length)
                            Console.WriteLine("Pairing done");

                        // Do not forget to close and release pairing completed event.
                        PairingCompletedEvent.Close();
                        PairingCompletedEvent = null;

                        // Do not forget to clean up radio object.
                        Radio = null;
                    }
                }

                // Do not forget to close Bluetooth Manager.
                Manager.Close();
            }

            Manager = null;
        }

        private static void Manager_OnAuthenticationCompleted(object Sender, wclBluetoothRadio Radio, long Address, int Error)
        {
            if (Address != DEVICE_MAC)
                Console.WriteLine("Pairing with unknown device " + Address.ToString("X12") + " completed with result: 0x" + Error.ToString("X8"));
            else
            {
                PairingResult = Error;

                if (Error == wclErrors.WCL_E_SUCCESS)
                    Console.WriteLine("Device has been paired");
                else
                    Console.WriteLine("Pairing with device failed: 0x" + Error.ToString("X8"));

                PairingCompletedEvent.Set();
            }
        }

        private static void Manager_OnPinRequest(object Sender, wclBluetoothRadio Radio, long Address, out string Pin)
        {
            Pin = "";
            if (Address == DEVICE_MAC)
            {
                Pin = PINS[PinIndex];
                Console.WriteLine("Trying to pair using PIN: " + Pin);
            }
            else
                Console.WriteLine("Unknwon device " + Address.ToString("X12") + " requuests PIN pairing. Ignore");
        }
    }
}
