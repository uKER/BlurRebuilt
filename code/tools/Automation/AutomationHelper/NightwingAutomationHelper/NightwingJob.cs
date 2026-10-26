namespace NightwingAutomation
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    using Automation;
    using NightwingAutomationHelper;

    public abstract class NightwingJob : BaseJob
    {
        public uint LevelId
        {
            get;
            set;
        }

        //public bool ShowHUD
        //{
        //    get;
        //    set;
        //}

        //public void SetRandomCars(uint numberOfCars, NightwingXtHelper xt)
        //{
        //    System.Diagnostics.Debug.Assert(vehicles.Count == 0, "Not allowed to mix and match RandomCars with adding vehicles");

        //    Random randy = new Random();

        //    List<uint> pickedCars = new List<uint>();

        //    for (int i = 0; i < numberOfCars; i++)
        //    {
        //        while (true)
        //        {
        //            // Randomly pick a car
        //            int rc = randy.Next() % xt.Vehicles.Count;

        //            NightwingVehicle v = xt.Vehicles[rc];

        //            bool canUse = WorkManager.PickNMixValid ? WorkManager.PassPickNMix("AmaxVehicle", v.Id) : (v.Use == true);

        //            if (canUse)
        //            {
        //                this.vehicles.Add(new NightwingJobVehicle() { Id = v.Id, ControlType = NightwingJob.NightwingJobVehicle.eControlType.Ai });
        //                break;
        //            }
        //        }
        //    }
        //}

        //public List<NightwingJobVehicle> Vehicles
        //{
        //    get
        //    {
        //        return this.vehicles;
        //    }
        //}

        //private List<NightwingJobVehicle> vehicles = new List<NightwingJobVehicle>();

        public NightwingJob(String jobName, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(jobName, kitRequirements, buildConfig)
        {
            this.LevelId = uint.MaxValue;
            //this.ShowHUD = true;
        }

        //public void AddCar(uint id)
        //{
        //    vehicles.Add(new NightwingJobVehicle() { Id = id } );
        //}

        //public void AddCar(uint id, NightwingJobVehicle.eControlType controlType)
        //{
        //    vehicles.Add(new NightwingJobVehicle() { Id = id, ControlType = controlType });
        //}

        public override bool Execute(List<AutomationKit> kits)
        {
            List<NightwingAutomationConnection> connections = new List<NightwingAutomationConnection>();

            bool problemFound = false;

            // We need to get the kits into the right condition to do the work
            foreach (AutomationKit kit in kits)
            {
                try
                {
                    // Try and connect to kit - launch exe if needed
                    NightwingAutomationConnection aac = new NightwingAutomationConnection(kit, this.BuildConfig);

                    connections.Add(aac);

                    bool connected = aac.AutoConnectForAutomation(this.RebootKitsBeforeRunning);

                    if (!connected)
                    {
                        problemFound = true;
                        break;
                    }

                    this.buildVersion = aac.BuildVersion;
                    this.buildDate = aac.BuildDate;
                    this.buildTime = aac.BuildTime;

                    //// Check what screen it's on and try and get to one we can handle
                }
                catch (Exception ex)
                {
                    kit.WriteLine("Exception: Execute: " + ex.Message);
                }
            }

            if (!problemFound) // Check if all kits are connected ok
            {
                System.Diagnostics.Debug.Assert(connections.Count > 0);

                try
                {
                    DoWork(connections);
                }
                catch (AutomationCrashException ex)
                {
                    ex.Kit.SaveCrashDump(this.ID + " - crash", true);

                    ex.Kit.WriteLine("Crash: '" + JobStage + "' - " + ex.Message);

                    this.Result = JobResult.Failed;
                    this.FailReason = JobFailReason.Crashed;

                    problemFound = true;
                }
                catch (AutomationAssertException ex)
                {
                    ex.Kit.SaveCrashDump(this.ID + " - assert", false);

                    ex.Kit.WriteLine("Assert: '" + JobStage + ((ex.Message != null && ex.Message.Length > 0) ? "' - " + ex.Message : string.Empty));

                    this.Result = JobResult.Failed;
                    this.FailReason = JobFailReason.Asserted;

                    this.FailInfo = ex;

                    problemFound = true;
                }
                catch (Exception ex)
                {
                    WorkManager.MasterLog.WriteLine("Exception: DoWork: " + ex.Message);
                    foreach (var c in connections)
                    {
                        c.WriteLine("Unknown kit: Exception: DoWork: " + ex.Message);
                    }

                    this.Result = JobResult.Failed;

                    problemFound = true;
                }

                // Reset all kits involved in the job to ensure they are safe to use next time
                if (problemFound)
                {
                    foreach (var con in connections)
                    {
                        // TODO: Reboot kit(s)
                    }
                }

                foreach (NightwingAutomationConnection conn in connections)
                {
                    conn.Dispose();
                }
            }

            return problemFound == false;
        }

        protected void SetupGameStateAndWaitForLevelToLoad(NightwingAutomationConnection aac)
        {
            JobStage = "Sending state to game";

            //aac.SendDebugVariable("UI.Render", this.ShowHUD ? "1" : "0");

            System.Diagnostics.Debug.Assert(LevelId != uint.MaxValue);
            aac.SendCommand("LevelId", LevelId);

            // Vehicle setup
            //aac.SendCommand("NumVehicles", vehicles.Count);

            //foreach (var v in vehicles)
            //{
            //    aac.SendCommand("Vehicle", vehicles.IndexOf(v), v.Id, (int)v.ControlType, vehicles.IndexOf(v));
            //}

            JobStage = "Trying to load the level";

            // Load the race with our settings
            aac.SendCommand("StartEvent");

            JobStage = "Waiting for the level to load";

            // Wait for the race to finish loading
            aac.WaitForLevelOrUiToLoad();
        }

        protected abstract void DoWork(List<NightwingAutomationConnection> connections);

        public class NightwingJobVehicle
        {
            public NightwingJobVehicle()
            {
                ControlType = eControlType.Player;
            }

            public enum eControlType
            {
                Player,
                Ai,
                Network,
                Replay,
                Showroom,
            }

            public uint Id
            {
                get;
                set;
            }

            public eControlType ControlType
            {
                get;
                set;
            }
        }
    }

    public class NightwingLevelMemorySize
    {
        public string name;
        public uint numBytes;

        public NightwingLevelMemorySize()
        {
            name = "<unknown>";
            numBytes = 0;
        }

        public NightwingLevelMemorySize(string memName, uint memSize)
        {
            name = memName;
            numBytes = memSize;
        }
    }

    public class MatchMemorySizeName
    {
        private string name;

        public MatchMemorySizeName(string name)
        {
            this.name = name;
        }

        public bool Compare(NightwingLevelMemorySize memorySize)
        {
            return this.name == memorySize.name;
        }
    }

    public class NightwingLevelLoadTestJob : NightwingJob
    {
        protected List<NightwingLevelMemorySize> memorySizes;
        public static HashSet<string> s_MemoryNames = new HashSet<string>();

        public NightwingLevelLoadTestJob(string name, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, kitRequirements, buildConfig)
        {
            memorySizes = new List<NightwingLevelMemorySize>();
        }

        public uint GetMemorySize(string memoryName)
        {
            MatchMemorySizeName predicate = new MatchMemorySizeName(memoryName);
            NightwingLevelMemorySize memorySize = this.memorySizes.Find(predicate.Compare);
            return (memorySize == null) ? 0 : memorySize.numBytes;
        }

        protected void ObtainMemorySummary(NightwingAutomationConnection aac)
        {
            CommandResult result = aac.SendCommand("RequestMemorySummary");

            if (!result.Failed)
            {
                string dataString = result.SingleLine;

                int pos = 0;

                while (pos < dataString.Length)
                {
                    int commaPos = dataString.IndexOf(',', pos);

                    if ((commaPos > pos) && (commaPos < dataString.Length))
                    {
                        string memName = dataString.Substring(pos, commaPos - pos);
                        pos = commaPos + 1;

                        commaPos = dataString.IndexOf(',', pos);

                        if ((commaPos > pos) && (commaPos < dataString.Length))
                        {
                            uint memSize = uint.Parse(dataString.Substring(pos, commaPos - pos));
                            pos = commaPos + 1;

                            memorySizes.Add(new NightwingLevelMemorySize(memName, memSize));
                            s_MemoryNames.Add(memName);
                        }
                        else
                        {
                            pos = dataString.Length;
                        }
                    }
                    else
                    {
                        pos = dataString.Length;
                    }
                }
            }
        }

        protected override void DoWork(List<NightwingAutomationConnection> connections)
        {
            System.Diagnostics.Debug.Assert(connections.Count > 0);

            NightwingAutomationConnection aac = connections[0];

            aac.DisableScreenSaver();

            aac.ClearProblems();

            JobStage = "Trying to setup the circuit load";

            //aac.SendCommand("Reset");

            // Send over all the stuff we need, city, cars, etc
            SetupGameStateAndWaitForLevelToLoad(aac);

            JobStage = "Wait for level to finish streaming";

            //aac.WaitForTime(TimeSpan.FromSeconds(6));
            bool keepLooping = true;

            DateTime start = DateTime.Now;
            
            while (keepLooping)
            {
                string screen = aac.CurrentUIScreen(TimeSpan.FromSeconds(10)).ToLower();

                switch (screen)
                {
                    case "ingame\\hud.lua":
                        keepLooping = false;
                        break;

                    case "ingame\\playlevelmovie.lua":
                        // Try and skip this screen
                        aac.ClearPadState(0);
                        aac.SelectButtonPress(0, AutomationKitButton.Button_A);
                        aac.SetPadState(0, true);
                        break;

                        // Do nothing cases
                    case "frontend.lua":
                    default:
                        break;
                }

                if (DateTime.Now - start > TimeSpan.FromSeconds(60))
                {
                    // Problem has occured - took too long
                    break;
                }

                System.Threading.Thread.Sleep(50);

                // Check to see if we've gone past a time threshold
            }

            System.Threading.Thread.Sleep(TimeSpan.FromSeconds(20));

            ObtainMemorySummary(aac);
            aac.TakeScreenshot(string.Format(@"Levels\{0} - {1}.jpg", this.ID, this.Name));

            JobStage = "Trying to return to the UI";

            // End the race and return to the UI
            aac.SendCommand("EndEvent");

            aac.WaitForLevelOrUiToLoad();

            System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));

            JobStage = "";

            this.Result = JobResult.Passed;
        }
    }

    //public class NightwingVehicleLoadTestJob : NightwingJob
    //{
    //    public NightwingVehicleLoadTestJob(string name, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
    //        : base(name, kitRequirements, buildConfig)
    //    {
    //        Take360ScreenShots = false;
    //    }

    //    public bool Take360ScreenShots
    //    {
    //        get;
    //        set;
    //    }

    //    protected override void DoWork(List<NightwingAutomationConnection> connections)
    //    {
    //        NightwingAutomationConnection aac = connections[0];

    //        aac.DisableScreenSaver();

    //        aac.ClearProblems();

    //        JobStage = "Trying to setup the circuit load";

    //        aac.SendCommand("Reset");

    //        // Send over all the stuff we need, city, cars, etc
    //        SetupGameStateAndWaitForRaceToLoad(aac);

    //        JobStage = "Wait in race for a period of time";

    //        aac.WaitForTime(TimeSpan.FromSeconds(4));

    //        this.Problems = aac.Problems;

    //        if (!aac.IsProblem("vehicle", "nofile", 0))
    //        {
    //            if (!Take360ScreenShots)
    //            {
    //                aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1}.jpg", this.ID, this.Name));
    //            }
    //            else
    //            {
    //                for (int angle = 0, pic = 0; angle < 360; angle += 45, pic++)
    //                {
    //                    double angleInRadians = angle * Math.PI / 180;

    //                    // Move stick to correct position
    //                    double x = -Math.Sin(angleInRadians);
    //                    double y = Math.Cos(angleInRadians);

    //                    aac.ClearPadState(0);
    //                    aac.SelectRightStick(0, x, y);
    //                    aac.SetPadState(0, false);

    //                    System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));

    //                    aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1} - {2}.jpg", this.ID, this.Name, pic));
    //                }

    //                // Reset pad back to defaults
    //                aac.ClearPadState(0);
    //                aac.SetPadState(0, false);
    //            }
    //        }

    //        JobStage = "Trying to return to the UI";

    //        // End the race and return to the UI
    //        aac.SendCommand("EndEvent");

    //        aac.WaitForUiToLoad();

    //        JobStage = "";

    //        this.Result = JobResult.Passed;

    //        CheckForProblems(aac);
    //    }

    //    /// <summary>
    //    /// Check that the vehicle we loaded was corrent in every way
    //    /// </summary>
    //    /// <param name="aac">A connection to the kit we're testing on</param>
    //    private void CheckForProblems(NightwingAutomationConnection aac)
    //    {
    //        // Check for any problems with the vehicle we loaded and report back
    //        if (aac.IsProblem("vehicle", "nofile", 0))
    //        {
    //            AddToComment("Default car (Missing mesh file)");
    //        }
    //        else
    //        {
    //            // If it's not the default model, report any other issues we can find
    //            if (aac.IsProblem("vehicle", "nocolmesh", 0))
    //            {
    //                AddToComment("No collision mesh");
    //                this.Result = JobResult.Failed;
    //                this.JobStage = "No collision mesh - falls through world";
    //            }

    //            if (aac.IsProblem("vehicle", "noshadcast", 0))
    //            {
    //                AddToComment("No shadow caster");
    //            }

    //            if (aac.IsProblem("vehicle", "nodobjmesh", 0))
    //            {
    //                AddToComment("No dynamic object mesh");
    //            }
                
    //            if (aac.IsProblem("vehicle", "invpartsc", 0))
    //            {
    //                AddToComment("Invalid PartSubCode(s)");
    //            }
    //        }
    //    }
    //}

    //public class NightwingRaceTestJob : NightwingJob
    //{
    //    private TimeSpan raceDuration;
    //    private uint tryGoingInAndOut;

    //    public NightwingRaceTestJob(string name, TimeSpan raceDuration, uint tryGoingInAndOut, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
    //        : base(name, kitRequirements, buildConfig)
    //    {
    //        this.raceDuration = raceDuration;
    //        this.tryGoingInAndOut = tryGoingInAndOut;
    //    }

    //    protected override void DoWork(List<NightwingAutomationConnection> connections)
    //    {
    //        NightwingAutomationConnection aac = connections[0];

    //        for (int i = 1; i <= tryGoingInAndOut; i++)
    //        {
    //            aac.ClearProblems();

    //            JobStage = "Trying to setup the circuit load";

    //            aac.SendCommand("Reset");

    //            // Send over all the stuff we need, city, cars, etc
    //            SetupGameStateAndWaitForRaceToLoad(aac);

    //            JobStage = "Wait in race for a period of time";

    //            aac.WaitForTime(this.raceDuration);

    //            this.Problems = aac.Problems;

    //            // aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1}.jpg", this.ID, this.Name));

    //            JobStage = "Trying to return to the UI";

    //            // End the race and return to the UI
    //            aac.SendCommand("EndEvent");

    //            aac.WaitForUiToLoad();

    //            JobStage = "";
    //        }

    //        this.Result = JobResult.Passed;
    //    }
    //}

    //public class NightwingMemoryLeakJob : NightwingRaceTestJob
    //{
    //    public NightwingMemoryLeakJob(string name, TimeSpan raceDuration, uint tryGoingInAndOut, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
    //        : base(name, raceDuration, tryGoingInAndOut, kitRequirements, buildConfig)
    //    {
    //        this.RebootKitsBeforeRunning = true;
    //    }

    //    protected override void DoWork(List<NightwingAutomationConnection> connections)
    //    {
    //        base.DoWork(connections);

    //        NightwingAutomationConnection aac = connections[0];

    //        CommandResult cr = aac.SendCommand("CheckMemLeak");

    //        string[] memInfo = cr.SingleLine.Split(':');

    //        bool frontendNoAllocs = (memInfo[1] == "1");
    //        uint frontendAllocationCount = uint.Parse(memInfo[2]);
    //        uint frontendLeakSystem = uint.Parse(memInfo[3]);
    //        uint frontendLeakPhysical = uint.Parse(memInfo[4]);

    //        bool gameNoAllocs = (memInfo[5] == "1");
    //        uint gameAllocationCount = uint.Parse(memInfo[6]);
    //        uint gameLeakSystem = uint.Parse(memInfo[7]);
    //        uint gameLeakPhysical = uint.Parse(memInfo[8]);

    //        uint physicalDifference = uint.Parse(memInfo[9]);
    //        uint totalDifference = uint.Parse(memInfo[10]);

    //        // Check results
    //        if (physicalDifference > 0)
    //        {
    //            this.AddToComment(string.Format("SOME PHYSICAL MEMORY NOT TRACKED ({0})", physicalDifference));
    //            this.Result = JobResult.Failed;
    //        }

    //        if (frontendLeakPhysical > 0 && gameLeakPhysical > 0)
    //        {
    //            this.AddToComment("PHYSICAL LEAK(S) DETECTED IN BOTH FRONTEND AND IN-GAME!");
    //        }
    //        else if (frontendLeakPhysical > 0)
    //        {
    //            this.AddToComment("PHYSICAL LEAK(S) IN FRONTEND DETECTED!");
    //        }
    //        else if (gameLeakPhysical > 0)
    //        {
    //            this.AddToComment("PHYSICAL LEAK(S) IN-GAME DETECTED!");
    //        }

    //        if (!frontendNoAllocs && frontendAllocationCount > 0)
    //        {
    //            string message = string.Format("Average of Allocs({0}) System({1}) Physical({2})", frontendAllocationCount, frontendLeakSystem, frontendLeakPhysical);

    //            this.AddToComment("Frontend: " + message);

    //            this.Result = JobResult.Failed;
    //        }

    //        if (!gameNoAllocs && gameAllocationCount > 0)
    //        {
    //            string message = string.Format("Average of Allocs({0}) System({1}) Physical({2})", gameAllocationCount, gameLeakSystem, gameLeakPhysical);
                
    //            this.AddToComment("InGame: " + message);
                
    //            this.Result = JobResult.Failed;
    //        }

    //        //LeakInfo li = new LeakInfo()
    //        //{
    //        //    FrontendNoAllocs = (memInfo[1] == "1"),
    //        //    FrontendAllocationCount = uint.Parse(memInfo[2]),
    //        //    FrontendLeakSystem = uint.Parse(memInfo[3]),
    //        //    FrontendLeakPhysical = uint.Parse(memInfo[4]),

    //        //    GameNoAllocs = (memInfo[5] == "1"),
    //        //    GameAllocationCount = uint.Parse(memInfo[2]),
    //        //    GameLeakSystem = uint.Parse(memInfo[3]),
    //        //    GameLeakPhysical = uint.Parse(memInfo[4])
    //        //};

    //        //this.ResultsData = li;
    //    }

    //    //public class LeakInfo
    //    //{
    //    //    public bool FrontendNoAllocs { get; set; }
    //    //    public uint FrontendLeakSystem { get; set; }
    //    //    public uint FrontendLeakPhysical { get; set; }
    //    //    public uint FrontendAllocationCount { get; set; }

    //    //    public bool GameNoAllocs { get; set; }
    //    //    public uint GameLeakSystem { get; set; }
    //    //    public uint GameLeakPhysical { get; set; }
    //    //    public uint GameAllocationCount { get; set; }
    //    //}
    //}
}