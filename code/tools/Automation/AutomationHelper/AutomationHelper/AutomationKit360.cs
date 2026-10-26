namespace Automation
{
    using System;
    using System.Collections.Generic;
    //using System.Linq;
    using System.Net;
    using System.Text;
    using System.Threading;
    using XDevkit;

    public class AutomationKit360 : AutomationKit
    {
        XboxManager xboxManager = new XboxManager();
        XboxConsole xboxConsole;

        private XBOX_AUTOMATION_GAMEPAD[] automationGamepad;

        private string GetStringFromConsoleMemory(uint addr, uint length)
        {
            try
            {
                uint bytesRead;

                byte[] tempArray = new byte[length];
                xboxConsole.DebugTarget.GetMemory(addr, length, tempArray, out bytesRead);

                return Encoding.UTF8.GetString(tempArray, 0, (int)length);
            }
            catch (Exception ex)
            {
                this.WriteLine(string.Format("GetStringFromConsoleMemory: addr(0x{0:X8}) len({1}): Exception - {2}", addr, length, ex.Message));

                return "?";
            }
        }

        private void HandleAssertException(XDevkit.XBOX_EVENT_INFO info)
        {
            string condition = GetStringFromConsoleMemory(info.Parameters[1], info.Parameters[0]);
            string file = GetStringFromConsoleMemory(info.Parameters[3], info.Parameters[2]);
            string function = GetStringFromConsoleMemory(info.Parameters[5], info.Parameters[4]);
            string message = GetStringFromConsoleMemory(info.Parameters[7], info.Parameters[6]);

            this.AssertFired(message, condition, function, file, info.Parameters[8]);
        }

        private string ExceptionCode(XDevkit.XBOX_EVENT_INFO info)
        {
            switch (info.Code)
            {
                // Access violation
                case 0xc0000005:
                    return string.Format("Unhandled exception at 0x{0:X8} in {1}: 0xC0000005: Access violation {2} location 0x{3:X8}",
                        info.Address,
                        this.ExePath,
                        (info.Parameters[0] == 0) ? "reading" : "writing",
                        info.Parameters[1]);

                // Data type misalignment
                case 0x80000002:
                    return string.Format("Unhandled exception at 0x{0:X8} in {1}: 0x80000002: Datatype misalignment.",
                        info.Address,
                        this.ExePath);

                // Stack overflow
                case 0xC00000FD:
                    return string.Format("Unhandled exception at 0x{0:X8} in {1}: 0xC00000FD: Stack overflow.",
                        info.Address,
                        this.ExePath);

                // SetThreadName
                case 0x406D1388:
                    return string.Format("SetThreadName exception at 0x{0:X8}", info.Address);

                default:
                    return string.Format("Unhandled exception at 0x{0:X8} in {1}: 0x{2:X8}",
                        info.Address,
                        this.ExePath,
                        info.Code);
            }
        }

        public override void SaveCrashDump(string filename, bool takePicture)
        {
            string dumpLoc = System.IO.Path.Combine(WorkManager.ReportFolder, "dumps");

            if (!System.IO.Directory.Exists(dumpLoc))
            {
                System.IO.Directory.CreateDirectory(dumpLoc);
            }

            string dumpFile = System.IO.Path.Combine(dumpLoc, filename);

            // Ensure filename is unique
            if (System.IO.File.Exists(dumpFile + ".dmp"))
            {
                string prevDumpFolder = System.IO.Path.Combine(dumpLoc, "previous");

                if (!System.IO.Directory.Exists(prevDumpFolder))
                {
                    System.IO.Directory.CreateDirectory(prevDumpFolder);
                }

                string prevDumpFile = System.IO.Path.Combine(prevDumpFolder, filename);

                if (System.IO.File.Exists(prevDumpFile + ".dmp"))
                {
                    int i = 1;

                    while (System.IO.File.Exists(prevDumpFile + "." + i + ".dmp"))
                        i++;

                    prevDumpFile = prevDumpFile + "." + i;
                }

                if (System.IO.File.Exists(dumpFile + ".dmp"))
                {
                    System.IO.File.Move(dumpFile + ".dmp", prevDumpFile + ".dmp");
                }

                if (System.IO.File.Exists(dumpFile + ".jpg"))
                {
                    System.IO.File.Move(dumpFile + ".jpg", prevDumpFile + ".jpg");
                }
            }

            this.xboxConsole.DebugTarget.ConnectAsDebugger("Automation", XboxDebugConnectFlags.Force);
            this.xboxConsole.DebugTarget.WriteDump(dumpFile + ".dmp", XboxDumpFlags.Normal);
            this.xboxConsole.DebugTarget.DisconnectAsDebugger();

            if (takePicture)
                TakeScreenshot(dumpFile.Substring(WorkManager.ReportFolder.Length + 1) + ".jpg");
        }

        private void MyStdNotifyHandler(XboxDebugEventType eventType, IXboxEventInfo eventInfo)
        {
            if (eventInfo.Info.Flags == XboxExceptionFlags.FirstChance)
                return;

            //AutomationHelper.DebugOutput.Write("MyStdNotifyHandler: " + eventType.ToString());

            switch (eventType)
            {
                case XboxDebugEventType.DebugString:
                    lock (this.debugOutputLogFileLock)
                    {
                        if (this.debugOutputLogFile != null)
                        {
                            string[] lines = eventInfo.Info.Message.TrimEnd(new Char[] { '\n', '\r' }).Split(new Char[] { '\n' });

                            foreach (var l in lines)
                            {
                                if (l.StartsWith("WARNING: "))
                                {
                                    this.warnings++;
                                }
                                else if (l.StartsWith("ERROR: "))
                                {
                                    this.errors++;
                                }
                                else if (l.StartsWith("FATAL ERROR: "))
                                {
                                    this.fatalErrors++;
                                }

                                this.debugOutputLogFile.WriteLine(l);
                            }
                        }
                    }
                    break;

                case XboxDebugEventType.Exception:

                    // Has our custom Assert fired?
                    if (eventInfo.Info.Code == 0x0000a557)
                    {
                        HandleAssertException(eventInfo.Info);
                    }
                    else
                    {
                        string errMsg = "MyStdNotifyHandler: ";

                        if (eventInfo.Info.Flags != 0)
                            errMsg += eventInfo.Info.Flags.ToString() + ": ";

                        this.WriteLine(errMsg + ExceptionCode(eventInfo.Info));

                        if (eventInfo.Info.Flags != XboxExceptionFlags.FirstChance)
                        {
                            ExceptionEvent.Set();
                        }
                    }

                    break;

                case XboxDebugEventType.AssertionFailed:
                case XboxDebugEventType.AssertionFailedEx:
                    System.Diagnostics.Debug.Assert(false, "Not Supported");
                    break;

                //case XboxDebugEventType.ExecutionBreak:
                //    break;
            }
        }

        public AutomationKit360(string name)
            : base(name, AutomationKitType.Xbox360)
        {
            automationGamepad = new XBOX_AUTOMATION_GAMEPAD[4];
        }

        public override bool Connect()
        {
            xboxConsole = xboxManager.OpenConsole(this.Name);

            xboxConsole.Shared = true;

            try
            {
                xboxConsole.OnStdNotify += new XboxEvents_OnStdNotifyEventHandler(MyStdNotifyHandler);

                xboxConsole.DumpMode = XboxDumpMode.Disabled;

                BindController(0);
                ConnectController(0);

                connected = true;

                return true;
            }
            catch (System.Runtime.InteropServices.COMException ex)
            {
                this.WriteLine(string.Format("AutomationKit360: Connect: Exception: {0} ({1})", xboxManager.TranslateError(ex.ErrorCode), ex.Message));

                if (ex.ErrorCode == unchecked((int)0x82da0100))
                {
                    this.WriteLine("This kit probably requires a hard reboot");
                }
            }
            catch (Exception ex)
            {
                this.WriteLine("AutomationKit360: Connect: Exception: " + ex.Message);
            }

            return false;
        }

        public override void Disconnect()
        {
            for (uint port = 0; port < 4; port++)
            {
                try
                {
                    UnbindController(port);
                }
                catch
                {
                }
            }
        }

        public override void Reset()
        {
        }

        public override bool IsAlive()
        {
            try
            {
                xboxConsole.FindConsole(0, 0);

                return true;
            }
            catch
            {
            }

            return false;
        }

        public override string Running()
        {
            try
            {
                return xboxConsole.DebugTarget.RunningProcessInfo.ProgramName;
            }
            catch
            {
                return "?";
            }
        }

        public override bool IsRunning(WorkManager.BuildConfig buildConfig)
        {
            this.exePath = buildConfig.Platform360;

            System.Diagnostics.Debug.Assert(connected == true, "Check we've got a connection to the kit");

            if (Automation.WorkManager.Verbose)
            {
                this.WriteLine("AutomationKit360:IsRunning - " + exePath);
            }

            try
            {
                string programName = xboxConsole.DebugTarget.RunningProcessInfo.ProgramName;

                if (this.exePath.ToLower().CompareTo(programName.ToLower()) != 0)
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.WriteLine("AutomationKit360:IsRunning - ExeMatchFail - " + programName);
                    }

                    return false;
                }

                // Check to see if it's crashed
                foreach (IXboxThread xbt in xboxConsole.DebugTarget.Threads)
                {
                    // Is any thread stopped?
                    if (xbt.StopEventInfo.IsThreadStopped != 0)
                    {
                        if (Automation.WorkManager.Verbose)
                        {
                            this.WriteLine("AutomationKit360:IsRunning - ThreadStopped - " + xbt.ThreadInfo.Name);
                        }

                        return false;
                    }
                }

                return true;
            }
            catch (System.Runtime.InteropServices.COMException ex)
            {
                this.WriteLine(string.Format("AutomationKit360:IsRunning: Exception: {0} ({1})", xboxManager.TranslateError(ex.ErrorCode), ex.Message));
                return false;
            }
            catch (Exception ex)
            {
                this.WriteLine("AutomationKit360:IsRunning: Exception - " + ex.Message);
            }

            return false;
        }

        public override uint IPAdddress()
        {
            return xboxConsole.IPAddressTitle;
        }

         /// <summary>
        /// Take a screenshot from the console. Due to varying back buffer formats we do this in DDS format then convert to whatever was asked for (TGA, JPG, etc)
        /// </summary>
        /// <param name="filename">The name of the target file. This also controls the output format</param>
        public override void TakeScreenshot(string filename)
        {
            // Ensure the directory we want to put the screenshot in exists
            string directory = System.IO.Path.Combine(WorkManager.ReportFolder, System.IO.Path.GetDirectoryName(filename));
            string fullPath = System.IO.Path.Combine(WorkManager.ReportFolder, filename);
            
            if (!System.IO.Directory.Exists(directory))
            {
                System.IO.Directory.CreateDirectory(directory);
            }

            string filenameDDS = fullPath + ".dds";

            xboxConsole.ScreenShot(filenameDDS);

            // DevIL isn't thread-safe, so we make sure it's only one at a time using it
            lock (devilLock)
            {
                System.Drawing.Bitmap bmp = DevIL.DevIL.LoadBitmap(filenameDDS);

                DevIL.DevIL.SaveBitmap(fullPath, bmp);

                System.IO.File.Delete(filenameDDS);
            }
        }

        public override void Reboot()
        {
            try
            {
                xboxConsole.Reboot(null, null, null, XboxRebootFlags.Cold);

                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(5));
            }
            catch
            {
            }
        }

        public override void Launch(WorkManager.BuildConfig buildConfig)
        {
            this.exePath = buildConfig.Platform360;

            if (Automation.WorkManager.Verbose)
            {
                this.WriteLine("AutomationKit360:Launch - " + this.exePath);
            }

            try
            {
                // Launch the new exe
                xboxConsole.Reboot(this.exePath, System.IO.Path.GetDirectoryName(this.exePath), null, XboxRebootFlags.Title);

                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(5));

                for (int tryCount = 0; tryCount < 30; tryCount++)
                {
                    try
                    {
                        if (Automation.WorkManager.Verbose)
                        {
                            this.WriteLine("AutomationKit360:Launch - FindConsole");
                        }

                        // Wait up to 30 seconds for the console to be found
                        xboxConsole.FindConsole(2, 1000);

                        uint ip = xboxConsole.IPAddressTitle;

                        if (Automation.WorkManager.Verbose)
                        {
                            this.WriteLine("AutomationKit360:Launch - FindConsole - Found");
                        }

                        break;
                    }
                    catch (System.Runtime.InteropServices.COMException ex)
                    {
                        if (Automation.WorkManager.Verbose)
                        {
                            this.WriteLine(string.Format("AutomationKit360:Launch: FindConsole - Sleep: {0} ({1})", xboxManager.TranslateError(ex.ErrorCode), ex.Message));
                        }

                        System.Threading.Thread.Sleep(TimeSpan.FromSeconds(2));
                    }
                    catch (Exception ex)
                    {
                        if (Automation.WorkManager.Verbose)
                        {
                            this.WriteLine("AutomationKit360:Launch - FindConsole - Sleep: " + ex.Message);
                        }

                        System.Threading.Thread.Sleep(TimeSpan.FromSeconds(2));
                    }
                }

                // Give the application 4 seconds from the point we can get an IP address from the console after a reboot
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(4));

                ExceptionEvent.Reset();
            }
            catch (System.Runtime.InteropServices.ExternalException ex)
            {
                this.WriteLine("AutomationKit360:Launch: Error: " + xboxManager.TranslateError(ex.ErrorCode) + " (" + ex.Message + ")");
            }
            catch (Exception ex)
            {
                this.WriteLine("AutomationKit360:Launch: Error: " + ex.Message);
            }
        }

        public override void ClearPadState(uint controllerPort)
        {
            automationGamepad[controllerPort] = new XBOX_AUTOMATION_GAMEPAD();
        }

        public override void SelectButtonPress(uint controllerPort, AutomationKitButton button)
        {
            switch (button)
            {
                case AutomationKitButton.Button_X:
                    automationGamepad[controllerPort].Buttons |= XboxAutomationButtonFlags.X_Button;
                    break;
                case AutomationKitButton.Button_Y:
                    automationGamepad[controllerPort].Buttons = XboxAutomationButtonFlags.Y_Button;
                    break;
                case AutomationKitButton.Button_A:
                    automationGamepad[controllerPort].Buttons = XboxAutomationButtonFlags.A_Button;
                    break;
                case AutomationKitButton.Button_B:
                    automationGamepad[controllerPort].Buttons = XboxAutomationButtonFlags.B_Button;
                    break;
                default:
                    System.Diagnostics.Debug.Fail("Not implemented yet");
                    break;
            }
        }

        public override void SelectLeftStick(uint controllerPort, double x, double y)
        {
            automationGamepad[controllerPort].LeftThumbX = (int)(32767 * x);
            automationGamepad[controllerPort].LeftThumbY = (int)(32767 * y);
        }

        public override void SelectRightStick(uint controllerPort, double x, double y)
        {
            automationGamepad[controllerPort].RightThumbX = (int)(32767 * x);
            automationGamepad[controllerPort].RightThumbY = (int)(32767 * y);
        }

        public override bool SetPadState(uint controllerPort, bool oneFrame)
        {
            SetGamepadState(controllerPort, ref automationGamepad[controllerPort], oneFrame);

            return true;
        }

        private void SetGamepadState(uint controllerPort, ref XBOX_AUTOMATION_GAMEPAD state, bool oneFrame)
        {
            BindController(controllerPort);
            ConnectController(controllerPort);

            bool routedToSystemProcess;

            xboxConsole.XboxAutomation.GetInputProcess(controllerPort, out routedToSystemProcess);

            if (routedToSystemProcess)
            {
                // Probably some kind of on-screen blade or overlay is on screen - let's try the old "back out"
                XBOX_AUTOMATION_GAMEPAD backOutState = new XBOX_AUTOMATION_GAMEPAD();
                state.Buttons |= XboxAutomationButtonFlags.B_Button;

                xboxConsole.XboxAutomation.QueueGamepadState(controllerPort, ref backOutState, 10, 1);

                System.Threading.Thread.Sleep(50);
            }

            if (oneFrame)
            {
                xboxConsole.XboxAutomation.QueueGamepadState(controllerPort, ref state, 10, 1);
            }
            else
            {
                xboxConsole.XboxAutomation.SetGamepadState(controllerPort, ref state);
            }
        }

        private void BindController(uint controllerPort)
        {
            xboxConsole.XboxAutomation.BindController(controllerPort, 32);
        }

        private void UnbindController(uint controllerPort)
        {
            xboxConsole.XboxAutomation.UnbindController(controllerPort);
        }

        private void ConnectController(uint controllerPort)
        {
            // TODO: Check if this can clear a "Please insert controller" message
            xboxConsole.XboxAutomation.ConnectController(controllerPort);
        }
    }
}
