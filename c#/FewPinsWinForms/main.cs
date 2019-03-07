using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Threading;

using wclCommon;
using wclBluetooth;

namespace FewPinsWinForms
{
    public partial class fmMain : Form
    {
        // Change this MAC to your one!!!
        private const Int64 DEVICE_MAC = 0xB48655ED4B8C;
        private static String[] PINS = { "0000", "1234", "1111", "9999" };

        private wclBluetoothManager Manager;
        private wclBluetoothRadio Radio;
        private Int32 PinIndex;

        private void StartPairing()
        {
            if (Radio != null)
                lbLog.Items.Add("Pairing already running");
            else
            {
                lbLog.Items.Clear();

                Int32 Res = Manager.Open();
                if (Res != wclErrors.WCL_E_SUCCESS)
                    lbLog.Items.Add("Unable to open Bluetooth Manager: 0x" + Res.ToString("X8"));
                else
                {
                    if (Manager.Count == 0)
                        lbLog.Items.Add("Bluetooth hardware not found");
                    else
                    {
                        for (Int32 i = 0; i < Manager.Count; i++)
                        {
                            if (Manager[i].Available)
                            {
                                Radio = Manager[i];
                                break;
                            }
                        }

                        if (Radio == null)
                            lbLog.Items.Add("No available Bluetooth radio found");
                        else
                        {
                            lbLog.Items.Add("Found " + Radio.ApiName + " Bluetooth radio");
                            PinIndex = 0;
                            tiWait.Start();
                            lbLog.Items.Add("Start pairing");
                        }
                    }

                    if (Radio == null)
                        Manager.Close();
                }
            }
        }

        private void StopPairing()
        {
            if (Radio == null)
                lbLog.Items.Add("Pairing is not running");
            else
            {
                tiWait.Stop();

                Radio = null;
                Manager.Close();
            }
        }

        public fmMain()
        {
            InitializeComponent();
        }

        private void fmMain_Load(object sender, EventArgs e)
        {
            Manager = new wclBluetoothManager();
            Manager.OnPinRequest += new wclBluetoothPinRequestEvent(Manager_OnPinRequest);
            Manager.OnAuthenticationCompleted += new wclBluetoothDeviceResultEvent(Manager_OnAuthenticationCompleted);

            Radio = null;
            PinIndex = 0;
        }

        private void Manager_OnAuthenticationCompleted(object Sender, wclBluetoothRadio Radio, long Address, int Error)
        {
            if (Address != DEVICE_MAC)
                lbLog.Items.Add("Pairing with unknown device " + Address.ToString("X12") + " completed with result: 0x" + Error.ToString("X8"));
            else
            {
                if (Error == wclErrors.WCL_E_SUCCESS)
                {
                    lbLog.Items.Add("Device has been paired");
                    StopPairing();
                }
                else
                {
                    lbLog.Items.Add("Pairing with device failed: 0x" + Error.ToString("X8"));
                    if (Radio != null)
                    {
                        PinIndex++;
                        if (PinIndex == PINS.Length)
                        {
                            lbLog.Items.Add("No more PINs");
                            StopPairing();
                        }
                        else
                        {
                            lbLog.Items.Add("Try next PIN");
                            tiWait.Start();
                        }
                    }
                }
            }
        }

        private void Manager_OnPinRequest(object Sender, wclBluetoothRadio Radio, long Address, out string Pin)
        {
            Pin = "";
            if (Address == DEVICE_MAC)
            {
                Pin = PINS[PinIndex];
                lbLog.Items.Add("Trying to pair using PIN: " + Pin);
            }
            else
                lbLog.Items.Add("Unknwon device " + Address.ToString("X12") + " requuests PIN pairing. Ignore");
        }

        private void fmMain_FormClosed(object sender, FormClosedEventArgs e)
        {
            StopPairing();
        }

        private void btStart_Click(object sender, EventArgs e)
        {
            StartPairing();
        }

        private void tiWait_Tick(object sender, EventArgs e)
        {
            tiWait.Stop();

            if (Radio != null)
            {
                Int32 Res = Radio.RemotePair(DEVICE_MAC);
                if (Res != wclErrors.WCL_E_SUCCESS)
                {
                    if (Res == wclBluetoothErrors.WCL_E_BLUETOOTH_ALREADY_PAIRED)
                        lbLog.Items.Add("Device already paired");
                    else
                        lbLog.Items.Add("Unable to start pairing with device: 0x" + Res.ToString("X8"));
                    StopPairing();
                }
                else
                    lbLog.Items.Add("Try to pair with device");
            }
        }

        private void btStop_Click(object sender, EventArgs e)
        {
            StopPairing();
        }
    }
}