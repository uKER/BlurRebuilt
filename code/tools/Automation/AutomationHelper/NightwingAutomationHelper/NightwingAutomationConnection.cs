namespace NightwingAutomation
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading;

    using Automation;

    public class NightwingAutomationConnection : AutomationConnection
    {
        AutoResetEvent levelOrUiLoaded = new AutoResetEvent(false);

        public NightwingAutomationConnection(AutomationKit kit, WorkManager.BuildConfig buildConfig)
            : base(kit, buildConfig)
        {
        }

        protected override bool GetToAutomationStartScreen()
        {
            // Can we get us to the point where we can start automating

            while (true)
            {
                string currentScreen = CurrentUIScreen().ToLower();

                if (Automation.WorkManager.Verbose)
                {
                    this.WriteLine("GetToAutomationStartScreen:CurrentScreen - " + currentScreen);
                }

                if (currentScreen == @"intro\nightwingmain.lua")
                {
                    return true;
                }
                else if ((currentScreen == @"intro\activisionsting.lua") ||
                    (currentScreen == @"intro\bizarresting.lua") ||
                    (currentScreen == @"intro\mgmsting.lua") ||
                    (currentScreen == @"intro\attractmode.lua"))
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.WriteLine("GetToAutomationStartScreen... Pressing A.");
                    }

                    ClearPadState(0);
                    SelectButtonPress(0, AutomationKitButton.Button_A);       // Skip movie
                    SetPadState(0, true);
                }
                else if (currentScreen == @"frontend.lua")
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

                //else if (currentScreen == @"intro\startscreen.lua")
                //{
                //    if (Automation.WorkManager.Verbose)
                //    {
                //        this.WriteLine("GetToAutomationStartScreen... Pressing Y.");
                //    }

                //    // Press Y to go onto Debug\DebugScreen.lua
                //    ClearPadState(0);
                //    SelectButtonPress(0, AutomationKitButton.Button_Y);
                //    SetPadState(0, true);
                //}
                //else if ((currentScreen == @"intro\activisionsplash.lua") || (currentScreen == @"intro\bizarresplash.lua"))
                //{
                //    if (Automation.WorkManager.Verbose)
                //    {
                //        this.WriteLine("GetToAutomationStartScreen... Pressing A.");
                //    }

                //    ClearPadState(0);
                //    SelectButtonPress(0, AutomationKitButton.Button_A);       // Skip movie
                //    SetPadState(0, true);
                //}
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

        protected override void HandleNotifyMessage(Bizarre.Artery.MessageReader msgReader)
        {
            string message;

            msgReader.Read(out message);

            switch (message)
            {
                case "LevelOrUiLoaded":
                    levelOrUiLoaded.Set();
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

            string[] parts = message.ToLower().Split(':');

            problems.Add(new ProblemInfo(parts[0], parts[1], int.Parse(parts[2])));

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

        public void WaitForLevelOrUiToLoad()
        {
            if (!WaitForEvent(levelOrUiLoaded, TimeSpan.FromSeconds(120.0)))
            {
                throw new AutomationTimeoutException(this.Kit, "WaitForLevelOrUiToLoad", "WaitForLevelOrUiToLoad timed out");
            }
        }
    }
}
