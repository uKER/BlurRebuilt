namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Text;
	using Bizarre.Test;

    public enum PS3AutomationButtonFlags
    {
        DPadUp = 1,
        DPadDown = 2,
        DPadLeft = 4,
        DPadRight = 8,
        StartButton = 16,
        BackButton = 32,
        LeftThumbButton = 64,
        RightThumbButton = 128,
        LeftShoulderButton = 256,
        RightShoulderButton = 512,
        Xbox360_Button = 1024,
        Bind_Button = 2048,
        A_Button = 4096,
        B_Button = 8192,
        X_Button = 16384,
        Y_Button = 32768,
    }

    public struct PS3_AUTOMATION_GAMEPAD
    {
        public PS3AutomationButtonFlags Buttons;
        public int LeftThumbX;
        public int LeftThumbY;
        public uint LeftTrigger;
        public int RightThumbX;
        public int RightThumbY;
        public uint RightTrigger;
    }

	public enum PS3VirtualPadSticks
	{
		LeftThumbX,
		LeftThumbY,
		RightThumbX,
		RightThumbY
	}

    public class AutomationKitPS3 : AutomationKit
    {
        /// <summary>
        /// Cached Process ID on the PS3 - Don't use this to directly get the value, use the ProcessId property
        /// </summary>
        private uint processId = 0;

        private PS3_AUTOMATION_GAMEPAD[] automationGamepad = new PS3_AUTOMATION_GAMEPAD[4];

        public int hTarget; //holds the target id of the PS3
        public string targetName;
        private AutomationConnection connection;

        private ScreenShotInfo screenshotInfo;

        public AutomationKitPS3(string name)
            : base(name, AutomationKitType.PS3)
        {
            targetName = name;
            
            Bizarre.Test.PS3Wrapper.InitTargetComms();
        }

        private uint ProcessId
        {
            get
            {
                if (this.processId != 0)
                {
                    return this.processId;
                }
                else
                {
                    uint[] processList = PS3Wrapper.ProcessList(hTarget, PS3Wrapper.ProcessListBufferSize(hTarget));

                    for (int i = 0; i < processList.Length; i++)
                    {
                        if (processList[i] != 0)
                        {
                            this.processId = processList[i];
                            
                            return this.processId;
                        }
                    }
                }

                // If it gets here then there's probably nothing runnong on the kit
                return 0;
            }
        }

        public void SetConnection(AutomationConnection conn)
        {
            Bizarre.Test.PS3Wrapper.InitTargetComms();
            connection = conn;
        }

        public void PostArteryConnection()
        {
            CommandResult result = connection.SendCommand("PS3GetScreenShotInfo");

            string[] lines = result.SingleLine.Split(':');

            screenshotInfo.width = uint.Parse(lines[0]);
            screenshotInfo.height = uint.Parse(lines[1]);
            screenshotInfo.pitch = uint.Parse(lines[2]);
            screenshotInfo.colour = byte.Parse(lines[3]);
            screenshotInfo.breakpoint = ulong.Parse(lines[4]);
            screenshotInfo.address = ulong.Parse(lines[5]);
        }

        public unsafe void EventCallback(int hTarget, uint eventType, uint eventParam, SNRESULT result, uint dataLength, byte* eventData, void* userData)
        {
            System.Diagnostics.Debug.WriteLine("PS3:EventCallback");
            System.Console.WriteLine("PS3:EventCallback");
        }

        public unsafe void TTYCallback(int hTarget, uint eventType, uint eventParam, SNRESULT result, uint dataLength, byte* eventData, void* userData)
        {
            System.Diagnostics.Debug.WriteLine("PS3:TTYCallback");
            System.Console.WriteLine("PS3:TTYCallback");
        }

        protected TMAPI_HandleEventCallback callback = null;
        protected TMAPI_HandleEventCallback callback2 = null;

        public override bool Connect()
        {
            this.hTarget = -1;

            this.connected = false;

            try
            {
                bool found = Bizarre.Test.PS3Wrapper.GetTargetFromName(out hTarget, this.targetName);

                if (found)
                {
                    // Try and connect to the kit
                    if (Bizarre.Test.PS3Wrapper.Connect(hTarget))
                    {
                        unsafe
                        {
                            callback2 = new TMAPI_HandleEventCallback(EventCallback);
                            PS3Wrapper.RegisterTargetEventHandler(hTarget, callback2, IntPtr.Zero);

                            callback = new TMAPI_HandleEventCallback(TTYCallback);
                            PS3Wrapper.RegisterTTYEventHandler(hTarget, SNPS3_TTY.SNPS3_TTY_ALL_STREAMS, callback, IntPtr.Zero);
                        }

                        this.connected = true;
                    }
                }

                //else
                //{
                //    ulong numTargets = Bizarre.Test.PS3Wrapper.GetNumTargets();

                //    if (numTargets == 1)
                //    {
                //        //connect to that one target
                //        bool found = Bizarre.Test.PS3Wrapper.GetTargetFromName(out hTarget, targetName);

                //        if (found)
                //            Bizarre.Test.PS3Wrapper.Connect(hTarget);
                //    }
                //    else if (numTargets == 0)
                //    {
                //        System.Diagnostics.Debug.WriteLine("FAIL: No targets are set in the PS3 Target Manager");
                //        return false;
                //    }
                //    else
                //    {
                //        //select the first connected kit we come across
                //        int i;
                //        for (i = 0; i < (int)numTargets; i++)
                //        {
                //            if (Bizarre.Test.PS3Wrapper.GetConnectStatus(i) == ECONNECTSTATUS.CS_CONNECTED)
                //            {
                //                hTarget = i;
                //                break;
                //            }
                //        }

                //        //if no kits are already connected, prompt the user to pick one
                //        if (i == (int)numTargets)
                //        {
                //            hTarget = (int)Bizarre.Test.PS3Wrapper.PickTarget(); //a selection box owned by the desktop

                //            //order Target Manager to connect to the supplied target
                //            Bizarre.Test.PS3Wrapper.Connect(hTarget);

                //        }
                //    }
                //}
            }
            catch (Exception ex)
            {
                this.WriteLine("AutomationKitPS3: Connect: Exception: " + ex.Message);
                return false;
            }

            return this.connected;
        }

        public override void Disconnect()
        {
            //connection.SendCommand("DisconnectPS3Controllers");
        }

        public override void Reset()
        {
        }

        public override void Reboot()
        {
            Bizarre.Test.PS3Wrapper.ResetEx(hTarget,
                                    (ulong)BootParams.SNPS3TM_BOOTP_DEBUG_MODE,
                                    (ulong)BootParams.SNPS3_BOOT_DEFAULT_MASK,
                                    (ulong)ResetType.SNPS3TM_RESETP_SOFT_RESET,
                                    (ulong)ResetType.SNPS3TM_RESETP_ALL_MASK,
                                    (ulong)SystemParams.SNPS3TM_SYSTEMP_TARGET_MODEL_60GB,
                                    (ulong)SystemParams.SNPS3TM_SYSTEMP_TARGET_MODEL);
        }

        private string PS3GetProcessName()
        {
            uint bufferSize = PS3Wrapper.ProcessInfoBufferSize(hTarget, this.ProcessId);

            SNPS3PROCESSINFO pi = PS3Wrapper.ProcessInfo(hTarget, this.ProcessId, bufferSize);

            if (pi.Hdr.szPath != null)
            {
                for (int ic = 0; ic < pi.Hdr.szPath.Length; ic++)
                {
                    if (pi.Hdr.szPath[ic] == '\0')
                    {
                        string programName = new string(pi.Hdr.szPath, 0, ic);

                        // Remove /app_home/ if the path starts with this
                        if (programName.StartsWith("/app_home/"))
                        {
                            programName = programName.Substring(10);
                        }

                        programName.Replace("\\\\", "\\");

                        return programName;
                    }
                }
            }

            return "?";
        }

        public override string Running()
        {
            try
            {
                return PS3GetProcessName();
            }
            catch
            {
                return "?";
            }
        }

        public override bool IsRunning(WorkManager.BuildConfig buildConfig)
        {
            this.exePath = buildConfig.PlatformPS3;

            System.Diagnostics.Debug.Assert(connected == true, "Check we've got a connection to the kit");

            if (Automation.WorkManager.Verbose)
            {
                this.WriteLine("AutomationKitPS3:IsRunning - " + exePath);
            }

            try
            {
                string programName = PS3GetProcessName();

                if (!programName.ToLower().Contains(this.exePath.ToLower()))
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.WriteLine("AutomationKitPS3:IsRunning - ExeMatchFail - " + programName);
                    }

                    return false;
                }

                return true;
            }
            catch (Exception ex)
            {
                this.WriteLine("AutomationKitPS3:IsRunning - Exception - " + ex.Message);
            }

            return false;
        }

        public override bool IsAlive()
        {
            throw new NotImplementedException();
        }

        public override uint IPAdddress()
        {
            SNPS3GamePortIPAddressData info = PS3Wrapper.GetGamePortDebugIPAddrData(hTarget, null);
            return info.uIPAddress;
        }

        public override void TakeScreenshot(string filename)
        {
            // Ensure the directory we want to put the screenshot in exists
            string directory = System.IO.Path.Combine(WorkManager.ReportFolder, System.IO.Path.GetDirectoryName(filename));
            string fullPath = System.IO.Path.Combine(WorkManager.ReportFolder, filename);

            if (!System.IO.Directory.Exists(directory))
            {
                System.IO.Directory.CreateDirectory(directory);
            }

            string filenameTemp = fullPath + ".bmp";

            while (PS3Wrapper.Kick() == SNRESULT.SN_S_OK) ;

            ECONNECTSTATUS status = Bizarre.Test.PS3Wrapper.GetConnectStatus(hTarget);

            SNPS3VRAMInfo vramInfo = new SNPS3VRAMInfo()
            {
                uWidth = screenshotInfo.width,
                uHeight = screenshotInfo.height,
                colour = screenshotInfo.colour,
                uPitch = screenshotInfo.pitch,
                uTopAddressPointer = screenshotInfo.address,
                uBPAddress = screenshotInfo.breakpoint
            };

            bool takenOk = Bizarre.Test.PS3Wrapper.VRAMCapture(hTarget, this.ProcessId, vramInfo, filenameTemp.ToCharArray());

            System.Diagnostics.Debug.Assert(takenOk == true, "Did we take the picture ok?");

            // DevIL isn't thread-safe, so we make sure it's only one at a time using it
            lock (devilLock)
            {
                System.Drawing.Bitmap bmp = DevIL.DevIL.LoadBitmap(filenameTemp);

                DevIL.DevIL.SaveBitmap(fullPath, bmp);

                System.IO.File.Delete(filenameTemp);
            }
        }

        public override void Launch(WorkManager.BuildConfig buildConfig)
        {
            this.exePath = buildConfig.PlatformPS3;

            if (Automation.WorkManager.Verbose)
            {
                this.WriteLine("AutomationKitPS3:Launch - " + this.exePath);
            }

            try
            {
                // Launch the new exe - on the PS3 if the self is already running it won't launch, so we need
                // to explicitly reset the kit to make launching work
                if (PS3GetProcessName().ToLower() == buildConfig.PlatformPS3.ToLower())
                {
                    Reboot();
                }

                //Bizarre.Test.PS3Wrapper.EnableVRAMCapture(hTarget);

                SNPS3TargetInfo targetInfo = new SNPS3TargetInfo();
                targetInfo.nFlags = TargetInfo.SN_TI_TARGETID | TargetInfo.SN_TI_FILESERVEDIR;
                targetInfo.hTarget = this.hTarget;

                // Set the File Serving folder to either one specified in the config file, or based off the location of the self
                if (buildConfig.PlatformPS3_Folder != null)
                {
                    targetInfo.pszFSDir = buildConfig.PlatformPS3_Folder;
                }
                else
                {
                    targetInfo.pszFSDir = System.IO.Path.GetDirectoryName(exePath);
                }

                PS3Wrapper.SetTargetInfo(targetInfo);

                // Make sure core dumps are enabled on loading the self
                //PS3Wrapper.SetExtraLoadFlags(this.hTarget, ExtraLoadFlags.SNPS3_XLF_ENABLE_CORE_DUMP, ExtraLoadFlags.SNPS3_XLF_ENABLE_CORE_DUMP);

                ExtraLoadFlags currentExtraLoadFlags = PS3Wrapper.GetExtraLoadFlags(this.hTarget);

                // Launch the process
                this.processId = PS3Wrapper.ProcessLoad(hTarget, (uint)0x3e7, exePath.ToCharArray(), 0, new char[0, 0], 0, new char[0, 0], (uint)LoadParams.SNPS3_LOAD_FLAG_WAIT_BD_MOUNTED);

                while (PS3Wrapper.Kick() == SNRESULT.SN_S_OK);

                // Did we launch ok? Maybe we're on the system screen and need to reboot
                if (this.processId == 0)
                {
                    Reboot();
                    this.processId = PS3Wrapper.ProcessLoad(hTarget, (uint)0x3e7, exePath.ToCharArray(), 0, new char[0, 0], 0, new char[0, 0], (uint)LoadParams.SNPS3_LOAD_FLAG_WAIT_BD_MOUNTED);
                }

                if (this.processId == 0)
                {
                    throw new Exception("AutomationPS3: Launch: Couldn't launch self");
                }

                // Give the application 4 seconds from the point we can get an IP address from the console after a reboot
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(4));

                ExceptionEvent.Reset();
            }
            catch (Exception ex)
            {
                this.WriteLine("Error: " + ex.Message);
            }
        }

        public override void SaveCrashDump(string filename, bool takePicture)
        {
            // HACK: Taken out as PS3 core dumps are TOO BIG!!!
            //Bizarre.Test.PS3Wrapper.SetCoreDumpFlags(this.hTarget, CoreDumpFlags.SNPS3TM_CORE_DUMP_TO_APP_HOME);
            //Bizarre.Test.PS3Wrapper.TriggerCoreDump(this.hTarget, this.ProcessId, 0, 0, 0);
        }

        public override void ClearPadState(uint controllerPort)
        {
            automationGamepad[controllerPort] = new PS3_AUTOMATION_GAMEPAD();

            connection.SendCommand("CreateVirtualPS3Pad", 1);
            connection.SendCommand("PS3ResetControls");
        }

        public override void SelectButtonPress(uint controllerPort, AutomationKitButton button)
        {
            connection.SendCommand("PS3button", button);
        }

        public override void SelectLeftStick(uint controllerPort, double x, double y)
        {
            connection.SendCommand("PS3Axis", PS3VirtualPadSticks.LeftThumbX, x);
            connection.SendCommand("PS3Axis", PS3VirtualPadSticks.LeftThumbY, y);
        }

        public override void SelectRightStick(uint controllerPort, double x, double y)
        {
            connection.SendCommand("PS3Axis", PS3VirtualPadSticks.RightThumbX, x);
            connection.SendCommand("PS3Axis", PS3VirtualPadSticks.RightThumbY, y);
        }

        public override bool SetPadState(uint controllerPort, bool oneFrame)
        {
            SetGamepadState(controllerPort, ref automationGamepad[controllerPort], oneFrame);

            return true;
        }

        private void SetGamepadState(uint controllerPort, ref PS3_AUTOMATION_GAMEPAD state, bool oneFrame)
        {
            // TODO: Need to write
        }

        private struct ScreenShotInfo
        {
            public uint width;
            public uint height;
            public uint pitch;
            public byte colour;
            public ulong breakpoint;
            public ulong address;
        }
    }
}
