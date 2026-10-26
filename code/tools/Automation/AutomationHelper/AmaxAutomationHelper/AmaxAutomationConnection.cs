namespace AmaxAutomation
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading;

    using Automation;

    public class AmaxAutomationConnection : AutomationConnection
    {
        AutoResetEvent raceLoaded = new AutoResetEvent(false);
        AutoResetEvent raceStarted = new AutoResetEvent(false);
        AutoResetEvent raceFinished = new AutoResetEvent(false);
        AutoResetEvent uiLoaded = new AutoResetEvent(false);
        
        AutoResetEvent lapEvent = new AutoResetEvent(false);
        int lapFinished = 0;
        
        AutoResetEvent splitEvent = new AutoResetEvent(false);

        public AmaxAutomationConnection(AutomationKit kit, WorkManager.BuildConfig buildConfig)
            : base(kit, buildConfig)
        {
        }

        protected override bool GetToAutomationStartScreen()
        {
            // Can we get us to the point where we can start automating

           // Enable Amax debug stuff so we can still get to the Debug screen when locked out in special builds
            SendCommand("ExecuteLuaCommand", "UIGlobals.DevMode = true");
            SendCommand("ExecuteLuaCommand", "GUI.allow_debug_buttons = true");

            DateTime startTime = DateTime.Now;

            while (DateTime.Now - startTime < TimeSpan.FromSeconds(20))
            {
                string currentScreen = CurrentUIScreen().ToLower();

                if (Automation.WorkManager.Verbose)
                {
                    this.WriteLine("GetToAutomationStartScreen:CurrentScreen - " + currentScreen);
                }

                if (currentScreen == @"debug\debugscreen.lua")
                {
                    return true;
                }
                else if (currentScreen == @"intro\startscreen.lua")
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.WriteLine("GetToAutomationStartScreen... Pressing Y.");
                    }

                    // Press Y to go onto Debug\DebugScreen.lua
                    ClearPadState(0);
                    SelectButtonPress(0, AutomationKitButton.Button_Y);
                    SetPadState(0, true);
                }
                else if ((currentScreen == @"intro\activisionsplash.lua") || (currentScreen == @"intro\bizarresplash.lua"))
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.WriteLine("GetToAutomationStartScreen... Pressing A.");
                    }

                    ClearPadState(0);
                    SelectButtonPress(0, AutomationKitButton.Button_A);       // Skip movie
                    SetPadState(0, true);
                }
                else
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.WriteLine("GetToAutomationStartScreen: Don't know how to handle this screen");
                    }

                    return false;
                }

                Thread.Sleep(TimeSpan.FromSeconds(1));
            }

            if (Automation.WorkManager.Verbose)
            {
                this.WriteLine("GetToAutomationStartScreen: Timed out trying to handle screens");
            }

            return false;
        }

        //override bool HandleSpecialScreen(string screenName)
        //{
        //    if ((screenName.ToLower() == "screens\\amax\\intro\\moviesting.lua") ||
        //        (screenName.ToLower() == "screens\\amax\\intro\\bizsting.lua") ||
        //        (screenName.ToLower() == "screens\amax\intro\profileload.lua") ||
        //        (screenName.ToLower() == "story\amaxstory.lua"))
        //    {
        //        // Need to hit the A key

        //        return true;
        //    }
        //    else
        //        return false;
        //}


        public bool WaitForTime(TestFinish tf)
        {
            switch (tf.Criteria)
            {
                case TestFinish.eFinishCriteria.kTimeInSecs:
                    {
                        WaitHandle[] wha = { Kit.AssertEvent, Kit.ExceptionEvent };
                        WaitForTimeOrEvent((TimeSpan)tf.CriteriaValue, wha);
                    }
                    break;

                case TestFinish.eFinishCriteria.kLaps:
                    int lapCount = (int)tf.CriteriaValue;

                    if (this.IsProblem("level", "aibroken") == 0)
                    {
                        while (true)
                        {
                            WaitHandle[] wha = { this.Kit.AssertEvent, this.Kit.ExceptionEvent, lapEvent, splitEvent, raceFinished };
                            int eventId = WaitForTimeOrEvent(tf.Timeout, wha);

                            // Check for the lap event firing - and then that we've hit the lap we're looking for
                            if (eventId == WaitHandle.WaitTimeout)
                            {
                                throw new AutomationTimeoutException(this.Kit, "WaitingFor", tf.ToString());
                            }
                            else if (eventId == 2)
                            {
                                if (this.lapFinished == lapCount)
                                    break;
                            }
                            else if (eventId == 4) // Race finished
                            {
                                return true;
                            }
                        }
                    }
                    else
                    {
                        // AI is broken - can't wait for laps
                        this.WriteLine("Wanted to wait for " + lapCount + " laps, but AI was broken");
                    }
                    break;

                default:
                    System.Diagnostics.Debug.Assert(false, "Unknown type");
                    break;
            }

            return false;
        }

        protected override void HandleNotifyMessage(Bizarre.Artery.MessageReader msgReader)
        {
            String message;

            msgReader.Read(out message);

            string[] messages = message.Split(':');

            switch (messages[0])
            {
                case "RaceLoaded":
                    raceLoaded.Set();
                    break;

                case "RaceStarted":
                    raceStarted.Set();
                    break;

                case "RaceFinished":
                    raceFinished.Set();
                    break;

                case "UiLoaded":
                    uiLoaded.Set();
                    break;

                case "LapInc":
                    {
                        int vehicleIndex = int.Parse(messages[1]);
                        int lapFinished = int.Parse(messages[2]);
                        this.lapFinished = lapFinished;
                        lapEvent.Set();
                    }
                    break;

                case "SplitCrossed":
                    {
                        int vehicleIndex = int.Parse(messages[1]);
                        int lap = int.Parse(messages[2]);
                        int split = int.Parse(messages[3]);
                        splitEvent.Set();
                    }
                    break;

                default:
                    this.WriteLine(string.Format("HandleNotifyMessage: Don't know how to handle '{0}'", message));
                    break;
            }
        }

        protected override void HandleProblemMessage(Bizarre.Artery.MessageReader msgReader)
        {
            string message;

            msgReader.Read(out message);

            string[] parts = message.Split(':');

            string[] parts2 = parts[1].Split(new string[] { "{?}" }, StringSplitOptions.None);
            parts2[0] = parts2[0].ToLower();

            problems.Add(new ProblemInfo(parts[0].ToLower(), parts2, int.Parse(parts[2])));

            //switch (parts[0])
            //{
            //    case "level":
            //        break;

            //    case "vehicle":
            //        switch (parts[1])
            //        {
            //            case "nofile":
            //                uint index = uint.Parse(parts[2]);
            //                break;
                        
            //            default:
            //                AutomationHelper.DebugOutput.Write(string.Format("HandleProblemMessage: Vehicle: Don't know how to handle '{0}'", message));
            //                break;
            //        }
            //        break;
                
            //    case "wheel":
            //        switch (parts[1])
            //        {
            //            case "nofile":
            //                break;

            //            default:
            //                AutomationHelper.DebugOutput.Write(string.Format("HandleProblemMessage: Wheel: Don't know how to handle '{0}'", message));
            //                break;
            //        }
            //        break;

            //    default:
            //        AutomationHelper.DebugOutput.Write(string.Format("HandleProblemMessage: Don't know how to handle '{0}'", message));
            //        break;
            //}
        }

        public void WaitForRaceToLoad()
        {
            if (!WaitForEvent(raceLoaded, TimeSpan.FromSeconds(120.0)))
            {
                throw new AutomationTimeoutException(this.Kit, "WaitForRaceToLoad", "WaitForRaceToLoad timed out");
            }
        }

        public void WaitForRaceToStart()
        {
            if (!WaitForEvent(raceStarted, TimeSpan.FromSeconds(120.0)))
            {
                throw new AutomationTimeoutException(this.Kit, "WaitForRaceToStart", "WaitForRaceToStart timed out");
            }
        }

        public void WaitForRaceToFinish()
        {
            if (!WaitForEvent(raceFinished, TimeSpan.FromSeconds(240.0)))
            {
                throw new AutomationTimeoutException(this.Kit, "WaitForRaceToFinish", "WaitForRaceToFinish timed out");
            }
        }

        public void WaitForUiToLoad()
        {
            if (!WaitForEvent(uiLoaded, TimeSpan.FromSeconds(60.0)))
            {
                throw new AutomationTimeoutException(this.Kit, "WaitForUiToLoad", "WaitForUiToLoad timed out");
            }

            // Ensure we're on the debug screen again
            for (int i = 0; i < 50; i++)
            {
                string screen = CurrentUIScreen();

                if (screen == @"debug\debugscreen.lua")
                {
                    System.Threading.Thread.Sleep(100);
                    return;
                }

                System.Threading.Thread.Sleep(100);
            }

            throw new AutomationTimeoutException(this.Kit, "WaitForUiToLoad", "WaitForUiToLoad timed out - DebugScreen");
        }
    }
}
