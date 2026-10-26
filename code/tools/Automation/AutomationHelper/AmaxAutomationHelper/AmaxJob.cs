namespace AmaxAutomation
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    using Automation;
    using AmaxAutomationHelper;

    public class TestFinish
    {
        public enum eFinishCriteria
        {
            kLaps,
            kTimeInSecs,
            kUnknown
        };

        private eFinishCriteria criteria = eFinishCriteria.kUnknown;
        private object criteriaValue = 0;
        private TimeSpan timeout = TimeSpan.FromSeconds(120);

        public TestFinish(TimeSpan ts)
        {
            criteria = eFinishCriteria.kTimeInSecs;
            criteriaValue = ts;
        }

        public TestFinish(eFinishCriteria criteria, object criteriaValue, TimeSpan timeout)
        {
            this.criteria = criteria;
            this.criteriaValue = criteriaValue;
            this.timeout = timeout;
        }

        public eFinishCriteria Criteria
        {
            get
            {
                return this.criteria;
            }
        }

        public object CriteriaValue
        {
            get
            {
                return this.criteriaValue;
            }
        }

        public TimeSpan Timeout
        {
            get
            {
                System.Diagnostics.Debug.Assert(this.criteria == eFinishCriteria.kLaps);
                return this.timeout;
            }
        }

        public override string ToString()
        {
            switch (this.criteria)
            {
                case eFinishCriteria.kTimeInSecs:
                    return "kTimeInSecs: " + ((TimeSpan)this.criteriaValue).TotalSeconds;
                
                case eFinishCriteria.kLaps:
                    return "kLaps: " + (int)this.criteriaValue;
                
                default:
                    System.Diagnostics.Debug.Assert(false);
                    return this.criteria.ToString();

            }
        }

        public void Parse(string param)
        {
            // Do we start with a number - then we assume this is time in seconds
            if (param != "true")
            {
                if (char.IsDigit(param, 0))
                {
                    criteria = eFinishCriteria.kTimeInSecs;
                    criteriaValue = int.Parse(param);
                }
                else
                {
                    char firstChar = char.ToLower(param[0]);

                    if (firstChar == 'l')
                    {
                        // We want to wait for X laps before we stop the test - 0 is complete race
                        criteria = eFinishCriteria.kLaps;
                        criteriaValue = int.Parse(param.Substring(1));
                    }
                    else if (firstChar == 's')
                    {
                        // Time is in seconds
                        criteria = eFinishCriteria.kTimeInSecs;
                        criteriaValue = TimeSpan.FromSeconds(int.Parse(param.Substring(1)));
                    }
                }
            }
        }
    }

    public abstract class AmaxJob : BaseJob
    {
        protected TestFinish testFinishCriteria;

        public VehicleProblems vehicleProblems = new VehicleProblems();

        public uint EventId
        {
            get;
            set;
        }

        public uint RouteId
        {
            get;
            set;
        }

        public uint Laps
        {
            get;
            set;
        }

        public bool ShowHUD
        {
            get;
            set;
        }

        public void SetRandomCars(uint numberOfCars, AmaxXtHelper xt)
        {
            System.Diagnostics.Debug.Assert(vehicles.Count == 0, "Not allowed to mix and match RandomCars with adding vehicles");

            Random randy = new Random();

            List<uint> pickedCars = new List<uint>();

            for (int i = 0; i < numberOfCars; i++)
            {
                while (true)
                {
                    // Randomly pick a car
                    int rc = randy.Next() % xt.Vehicles.Count;

                    AmaxVehicle v = xt.Vehicles[rc];

                    bool canUse = WorkManager.PickNMixValid ? WorkManager.PassPickNMix("AmaxVehicle", v.Id) : (v.Use == true);

                    if (canUse)
                    {
                        this.vehicles.Add(new AmaxJobVehicle() { Id = v.Id, ControlType = AmaxJob.AmaxJobVehicle.eControlType.Ai });
                        break;
                    }
                }
            }
        }

        public List<AmaxJobVehicle> Vehicles
        {
            get
            {
                return this.vehicles;
            }
        }

        private List<AmaxJobVehicle> vehicles = new List<AmaxJobVehicle>();

        public AmaxJob(String jobName, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(jobName, kitRequirements, buildConfig)
        {
            this.EventId = uint.MaxValue;
            this.RouteId = uint.MaxValue;
            this.ShowHUD = true;
            
            this.Laps = 3;
        }

        public void AddCar(uint id)
        {
            this.vehicles.Add(new AmaxJobVehicle() { Id = id });
        }

        public void AddCar(uint id, AmaxJobVehicle.eControlType controlType)
        {
            this.vehicles.Add(new AmaxJobVehicle() { Id = id, ControlType = controlType });
        }

        public override bool Execute(List<AutomationKit> kits)
        {
            List<AmaxAutomationConnection> connections = new List<AmaxAutomationConnection>();

            bool problemFound = false;
            bool connectionIssue = false;

            // We need to get the kits into the right condition to do the work
            foreach (AutomationKit kit in kits)
            {
                try
                {
                    // Try and connect to kit - launch exe if needed
                    AmaxAutomationConnection aac = new AmaxAutomationConnection(kit, this.BuildConfig);

                    connections.Add(aac);

                    if (!aac.AutoConnectForAutomation(this.RebootKitsBeforeRunning))
                    {
                        connectionIssue = true;
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

            if (!connectionIssue) // Check if all kits are connected ok
            {
                System.Diagnostics.Debug.Assert(connections.Count > 0);

                try
                {
                    DoWork(connections);
                    PostWork(connections);
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
                    string message = string.Format("Assert: '{0}'", JobStage);

                    if (ex.Message != null && ex.Message.Length > 0)
                    {
                        message += " - " + ex.Message;
                    }
                    ex.Kit.WriteLine(message);

                    ex.Kit.SaveCrashDump(this.ID + " - assert", false);

                    this.Result = JobResult.Failed;
                    this.FailReason = JobFailReason.Asserted;

                    this.FailInfo = ex;

                    problemFound = true;
                }
                catch (AutomationTimeoutException ex)
                {
                    ex.Kit.WriteLine(string.Format("Timeout: '{0}' - Cmd({1}) Msg({2})", JobStage, ex.Command, ex.Message));

                    this.Result = JobResult.Failed;
                    this.FailReason = JobFailReason.TimedOut;

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
            }
            else
            {
                // Failed to connect
                this.Result = JobResult.ConnectionIssue;
                problemFound = true;
            }

            foreach (AmaxAutomationConnection conn in connections)
            {
                conn.Dispose();
            }

            return problemFound == false;
        }

        protected void SetupGameStateAndWaitForRaceToLoad(AmaxAutomationConnection aac)
        {
            JobStage = "Sending state to game";

            aac.SendCommand("Reset");

            aac.SendDebugVariable("UI.Render", this.ShowHUD ? "1" : "0");
            aac.SendDebugVariable("GameSys.DebugHUD.Subscriber.Render", "0");   // Disable on-screen debug output
            aac.SendDebugVariable("Graphics.FX.ShaderType", "Speed");
            aac.SendDebugVariable("GameSys.MiniMonitor.Enable", "0");

            if (this.EventId != uint.MaxValue)
            {
                System.Diagnostics.Debug.Assert(this.vehicles.Count == 1, "Events should have 1 car selected");

                aac.SendCommand("EventId", this.EventId, this.vehicles[0].Id);
            }
            else
            {
                System.Diagnostics.Debug.Assert(this.RouteId != uint.MaxValue);
                aac.SendCommand("RouteId", this.RouteId);

                aac.SendCommand("Laps", this.Laps);

                // Vehicle setup
                aac.SendCommand("NumVehicles", this.vehicles.Count);

                foreach (var v in vehicles)
                {
                    // VehicleId - ControlType - StartPos
                    aac.SendCommand("Vehicle", this.vehicles.IndexOf(v), v.Id, (int)v.ControlType, this.vehicles.IndexOf(v));
                }
            }

            JobStage = "Trying to load the circuit";

            // Load the race with our settings
            aac.SendCommand("StartEvent");

            JobStage = "Waiting for the circuit to load";

            // Wait for the race to finish loading
            aac.WaitForRaceToLoad();
        }

        protected abstract void DoWork(List<AmaxAutomationConnection> connections);

        /// <summary>
        /// Function that is called after the work is done in which we can check how everything went
        /// </summary>
        /// <param name="connections">A list of all the connections involved in the test</param>
        protected virtual void PostWork(List<AmaxAutomationConnection> connections)
        {
        }

        public class AmaxJobVehicle
        {
            public AmaxJobVehicle()
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

        /// <summary>
        /// Check if the game flagged any issues with vehicles we loaded
        /// </summary>
        /// <param name="aac">A connection to the kit we're testing on</param>
        protected void CheckForVehicleProblems(AmaxAutomationConnection aac)
        {
            // Check for any problems with the vehicle we loaded and report back
            if (aac.IsProblem("vehicle", "nofile", 0) > 0)
            {
                vehicleProblems.defaultCar = true;
            }
            else
            {
                // If it's not the default model, report any other issues we can find
                if (aac.IsProblem("vehicle", "nocolmesh", 0) > 0)
                {
                    vehicleProblems.missingCollisionMesh = true;
                    this.Result = JobResult.Failed;
                    this.JobStage = "No collision mesh - falls through world";
                }

                if (aac.IsProblem("vehicle", "noshadcast", 0) > 0)
                {
                    vehicleProblems.missingShadowCaster = true;
                }

                if (aac.IsProblem("vehicle", "nodobjmesh", 0) > 0)
                {
                    vehicleProblems.missingDynamicObjectMesh = true;
                }

                if (aac.IsProblem("vehicle", "nocarmesh", 0) > 0)
                {
                    vehicleProblems.missingCarColliderMesh = true;
                }

                if (aac.IsProblem("vehicle", "invpartsc", 0) > 0)
                {
                    List<AmaxAutomationConnection.ProblemInfo> problems = aac.FindProblems("vehicle", "invpartsc", 0);

                    List<string> parts = new List<string>();
                    string partString = string.Empty;

                    foreach (var p in problems)
                    {
                        if (!parts.Contains(p.Value(0)))
                        {
                            parts.Add(p.Value(0));

                            if (partString.Length > 0)
                            {
                                partString += "\n";
                            }

                            partString += p.Value(0);
                        }
                    }

                    vehicleProblems.invalidPartCodes = true;
                    if (partString.Length > 0)
                    {
                        vehicleProblems.partCodes = partString;
                    }
                }

                if (aac.IsProblem("vehicle", "noexemit", 0) > 0)
                {
                    vehicleProblems.noExhaustEmitterInSorcery = true;
                }

                if (aac.IsProblem("vehicle", "nopareff", 0) > 0)
                {
                    vehicleProblems.noDamageParticleEffectsInSorcery = true;
                }
            }
        }

        protected void CheckForCircuitProblems(AmaxAutomationConnection aac)
        {
            // Check for any problems with the vehicle we loaded and report back
            int missingStartPos = aac.IsProblem("level", "nostartpos");
            if (missingStartPos > 0)
            {
                AddToComment(string.Format("Missing {0} Start Positions", missingStartPos));
            }

            if (aac.IsProblem("level", "nosplits", 0) > 0)
                AddToComment("Start/Fin Split Missing");
            if (aac.IsProblem("level", "nosplits", 1) > 0)
                AddToComment("Split 1 Missing");
            if (aac.IsProblem("level", "nosplits", 2) > 0)
                AddToComment("Split 2 Missing");

            if (aac.IsProblem("level", "aibroken") > 0)
            {
                AddToComment("AI broken");
            }

            // Check for missing multi-player actor lists
            if (aac.IsProblem("level", "nompactorlists") > 0)
                AddToComment("Route missing multi-player actor lists");

            // ActorStudio placement errors

            int actorsAtOrigin = aac.IsProblem("actorstudio", "actoratorigin");
            if (actorsAtOrigin > 0)
                AddToComment(string.Format("{0} Actors placed at origin", actorsAtOrigin));

            int actorsAtSamePos = aac.IsProblem("actorstudio", "actorsatsameposition");
            if (actorsAtSamePos > 0)
                AddToComment(string.Format("{0} Actors placed at same position", actorsAtSamePos));

            if (aac.IsProblem("actorstudio", "placementerrorsfound") > 0)
                AddToComment("ActorStudio placement errors have been detected");

            // Load fails with Physical Object Actors

            int physObjProbs = aac.IsProblem("physicalobject", "nulldefinition");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects with NULL definitions", physObjProbs));

            physObjProbs = aac.IsProblem("physicalobject", "nullrootobject");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects with NULL root objects", physObjProbs));

            physObjProbs = aac.IsProblem("physicalobject", "nullmodelhandle");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects with NULL model handles", physObjProbs));

            physObjProbs = aac.IsProblem("physicalobject", "audiocomponentfail");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects failed to create audio components", physObjProbs));

            physObjProbs = aac.IsProblem("physicalobject", "damageobjectinitfail");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects failed to initialise damage objects", physObjProbs));

            physObjProbs = aac.IsProblem("physicalobject", "modelresourceinitfail");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects failed to initialise model resources", physObjProbs));

            physObjProbs = aac.IsProblem("physicalobject", "rendercomponentcreatefail");
            if (physObjProbs > 0)
                AddToComment(string.Format("{0} PhysicalObjects failed to create render components", physObjProbs));
        }

        public class VehicleProblems
        {
            public bool defaultCar = false;
            public bool missingCollisionMesh = false;
            public bool missingShadowCaster = false;
            public bool missingDynamicObjectMesh = false;
            public bool missingCarColliderMesh = false;
            public bool invalidPartCodes = false;
            public bool noExhaustEmitterInSorcery = false;
            public bool noDamageParticleEffectsInSorcery = false;

            public string partCodes = null;
        }
    }

    public class AmaxCircuitLoadTestJob : AmaxJob
    {
        private uint levelUniqueObjects;
        private uint levelDynamicObjects;
        private uint levelPhysicalObjects;

        private string screenshotFolder;

        public AmaxCircuitLoadTestJob(string name, string screenshotFolder, TestFinish testFinishCriteria, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, kitRequirements, buildConfig)
        {
            this.screenshotFolder = screenshotFolder;
            this.testFinishCriteria = testFinishCriteria;
        }

        public uint LevelUniqueObjects
        {
            get
            {
                return this.levelUniqueObjects;
            }
        }

        public uint LevelDynamicObjects
        {
            get
            {
                return this.levelDynamicObjects;
            }
        }

        public uint LevelPhysicalObjects
        {
            get
            {
                return this.levelPhysicalObjects;
            }
        }

        protected override void DoWork(List<AmaxAutomationConnection> connections)
        {
            System.Diagnostics.Debug.Assert(connections.Count > 0);

            AmaxAutomationConnection aac = connections[0];

            aac.DisableScreenSaver();

            aac.ClearProblems();

            JobStage = "Trying to setup the circuit load";

            aac.SendCommand("Reset");

            // Send over all the stuff we need, city, cars, etc
            SetupGameStateAndWaitForRaceToLoad(aac);

            {
                CommandResult cr = aac.SendCommand("LevelObjectInfo");

                string[] info = cr.SingleLine.Split('!');

                levelUniqueObjects = uint.Parse(info[0]);
                levelDynamicObjects = uint.Parse(info[1]);
                levelPhysicalObjects = uint.Parse(info[2]);
            }


            JobStage = "Wait in race for a period of time";

            aac.WaitForTime(TimeSpan.FromSeconds(3));
            aac.TakeScreenshot(string.Format(this.screenshotFolder + @"\{0} - {1} - 1.jpg", this.ID, this.Name));

            if (aac.IsProblem("level", "aibroken") == 0)
            {
                aac.WaitForTime(this.testFinishCriteria);

                if (this.testFinishCriteria.Criteria == TestFinish.eFinishCriteria.kLaps)
                    aac.WaitForTime(TimeSpan.FromSeconds(1));

                aac.TakeScreenshot(string.Format(this.screenshotFolder + @"\{0} - {1} - 2.jpg", this.ID, this.Name));
            }

            JobStage = "Trying to return to the UI";

            // End the race and return to the UI
            aac.SendCommand("EndEvent");

            aac.WaitForUiToLoad();

            JobStage = "";

            this.Result = JobResult.Passed;

            CheckForCircuitProblems(aac);
        }
    }

    public class AmaxVehicleLoadTestJob : AmaxJob
    {
        uint vehicleMemorySize = 0;

        public AmaxVehicleLoadTestJob(string name, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, kitRequirements, buildConfig)
        {
            Take360ScreenShots = false;
        }

        public bool Take360ScreenShots
        {
            get;
            set;
        }

        public uint SizeInMemory
        {
            get
            {
                return vehicleMemorySize;
            }
        }

        protected override void DoWork(List<AmaxAutomationConnection> connections)
        {
            AmaxAutomationConnection aac = connections[0];

            aac.DisableScreenSaver();

            aac.ClearProblems();

            JobStage = "Trying to setup the circuit load";

            aac.SendCommand("Reset");

            // Re-enable the chase camera just for taking rotation shots
            aac.SendDebugVariable("Camera2.VehicleFeed.DisableChaseCam2", "0");

            // Send over all the stuff we need, city, cars, etc
            SetupGameStateAndWaitForRaceToLoad(aac);

            JobStage = "Wait in race for a period of time";

            aac.WaitForTime(TimeSpan.FromSeconds(4));

            this.Problems = aac.Problems;

            if (aac.IsProblem("vehicle", "nofile", 0) == 0)
            {
                if (!Take360ScreenShots)
                {
                    aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1}.jpg", this.ID, this.Name));
                }
                else
                {
                    // Take pics of the non-chase camera's
                    //aac.SendCommand("CarCam", (int)3);
                    //System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));
                    //aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1} - 0-Normal - {2}.jpg", this.ID, this.Name, "Dash"));

                    //aac.SendCommand("CarCam", (int)4);
                    //System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));
                    //aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1} - 0-Normal - {2}.jpg", this.ID, this.Name, "Bonnet"));

                    aac.SendCommand("CarCam", (int)5);
                    System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));
                    aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1} - 0-Normal - {2}.jpg", this.ID, this.Name, "Bumper"));

                    // Now do a 360 around the car + Top
                    for (int iRun = 0; iRun < 2; iRun++)
                    {
                        string prefix = "1-Normal - ";

                        // Turn on damage
                        if (iRun == 1)
                        {
                            aac.SendDebugVariable("Vehicle.Damage.Helpers.FakeArtScratchLevel", "1");
                            aac.SendDebugVariable("Vehicle.Damage.Helpers.FakeArtDamage", "1");
                            System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));

                            prefix = "2-Damage - ";
                        }

                        aac.SendCommand("CarCam", (int)2);
                        System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));

                        aac.SendCommand("TopCam");
                        System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));
                        aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1} - {2}.jpg", this.ID, this.Name, prefix + "Top"));

                        aac.SendCommand("CarCam", (int)2);
                        System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));

                        for (int angle = 0, pic = 0; angle < 360; angle += 45, pic++)
                        {
                            double angleInRadians = angle * Math.PI / 180;

                            // Move stick to correct position
                            double x = -Math.Sin(angleInRadians);
                            double y = Math.Cos(angleInRadians);

                            aac.ClearPadState(0);
                            aac.SelectRightStick(0, x, y);
                            aac.SetPadState(0, false);

                            System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));

                            aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1} - {2}Chase {3}.jpg", this.ID, this.Name, prefix, pic));
                        }
                        
                        aac.ClearPadState(0);
                        aac.SetPadState(0, false);
                    }

                    // Reset pad back to defaults
                    aac.ClearPadState(0);
                    aac.SetPadState(0, false);
                }
            }

            // Check vehicle memory usage
            {
                CommandResult cr = aac.SendCommand("MemUsage", "actors.vehicle.render");

                string[] info = cr.SingleLine.Split(':');

                vehicleMemorySize = uint.Parse(info[2]);
            }

            JobStage = "Trying to return to the UI";

            // End the race and return to the UI
            aac.SendCommand("EndEvent");

            aac.WaitForUiToLoad();

            JobStage = "";

            this.Result = JobResult.Passed;

            CheckForVehicleProblems(aac);
        }
    }

    public class AmaxRaceTestJob : AmaxJob
    {
        private TimeSpan raceDuration;
        private uint tryGoingInAndOut;

        public AmaxRaceTestJob(string name, TimeSpan raceDuration, uint tryGoingInAndOut, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, kitRequirements, buildConfig)
        {
            this.raceDuration = raceDuration;
            this.tryGoingInAndOut = tryGoingInAndOut;
        }

        protected override void DoWork(List<AmaxAutomationConnection> connections)
        {
            AmaxAutomationConnection aac = connections[0];

            for (int i = 1; i <= tryGoingInAndOut; i++)
            {
                //aac.DisableScreenSaver();

                aac.ClearProblems();

                JobStage = "Trying to setup the circuit load";

                aac.SendCommand("Reset");

                // Send over all the stuff we need, city, cars, etc
                SetupGameStateAndWaitForRaceToLoad(aac);

                JobStage = "Wait in race for a period of time";

                aac.WaitForTime(this.raceDuration);

                this.Problems = aac.Problems;

                // aac.TakeScreenshot(string.Format(@"Vehicles\{0} - {1}.jpg", this.ID, this.Name));

                JobStage = "Trying to return to the UI";

                // End the race and return to the UI
                aac.SendCommand("EndEvent");

                aac.WaitForUiToLoad();

                JobStage = "";
            }

            this.Result = JobResult.Passed;
        }
    }

    public class AmaxEventTestJob : AmaxJob
    {
        public AmaxEventTestJob(string name, TestFinish testFinishCriteria, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, kitRequirements, buildConfig)
        {
            this.testFinishCriteria = testFinishCriteria;
        }

        protected override void DoWork(List<AmaxAutomationConnection> connections)
        {
            AmaxAutomationConnection aac = connections[0];

            aac.DisableScreenSaver();

            aac.ClearProblems();

            // Send over all the stuff we need, city, cars, etc
            SetupGameStateAndWaitForRaceToLoad(aac);

            aac.WaitForTime(TimeSpan.FromSeconds(2));

            aac.TakeScreenshot(string.Format(@"Events\{0} - {1} - 1.jpg", this.ID, this.Name));

            JobStage = "Wait in race for a period of time";

            if (aac.IsProblem("level", "aibroken") == 0)
            {
                aac.WaitForRaceToStart();

                DateTime startTime = DateTime.Now;

                bool didRaceFinish = aac.WaitForTime(this.testFinishCriteria);

                DateTime endTime = DateTime.Now;

                // Check for finishing race very quickly... bad sign
                if (didRaceFinish)
                {
                    TimeSpan timeDiff = endTime - startTime;

                    if (timeDiff < TimeSpan.FromSeconds(20))
                    {
                        aac.SendCommand("EndEvent");
                        aac.WaitForUiToLoad();

                        JobStage = "";
                        this.Result = JobResult.Failed;
                        this.FailReason = JobFailReason.Logic;
                        this.Comment = "Race ended within 20 secs of starting";
                        return;
                    }
                }

                if (this.testFinishCriteria.Criteria == TestFinish.eFinishCriteria.kLaps)
                    aac.WaitForTime(TimeSpan.FromSeconds(1));

                aac.TakeScreenshot(string.Format(@"Events\{0} - {1} - 2.jpg", this.ID, this.Name));
            }

            this.Problems = aac.Problems;

            JobStage = "Trying to return to the UI";

            // End the race and return to the UI
            aac.SendCommand("EndEvent");

            aac.WaitForUiToLoad();

            JobStage = "";

            this.Result = JobResult.Passed;
        }
    }

    public class AmaxPerfJob : AmaxJob
    {
        public AmaxPerfJob(string name, TestFinish testFinishCriteria, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, kitRequirements, buildConfig)
        {
            this.testFinishCriteria = testFinishCriteria;
        }

        protected override void DoWork(List<AmaxAutomationConnection> connections)
        {
            AmaxAutomationConnection aac = connections[0];

            aac.DisableScreenSaver();

            aac.ClearProblems();

            // Turn off all logging to prevent spikes
            aac.SendCommand("LogOnlyFatal");

            aac.SendDebugVariable("Graphics.FX.ShaderType", "Speed");
            aac.SendDebugVariable("GameSys.MiniMonitor.Enable", "0");
            aac.SendDebugVariable("Game.MaxPhysicsUpdates", "2");
            aac.SendCommand("PerfStart", "AmaxPerf", WorkManager.MasterVersion);

            // Send over all the stuff we need, city, cars, etc
            SetupGameStateAndWaitForRaceToLoad(aac);

            aac.WaitForTime(TimeSpan.FromSeconds(2));

            JobStage = "Wait in race for a period of time";

            if (aac.IsProblem("level", "aibroken") == 0)
            {
                aac.WaitForTime(this.testFinishCriteria);

                if (this.testFinishCriteria.Criteria == TestFinish.eFinishCriteria.kLaps)
                    aac.WaitForTime(TimeSpan.FromSeconds(1));
            }

            aac.SendCommand("PerfStop");

            this.Problems = aac.Problems;

            JobStage = "Trying to return to the UI";

            // End the race and return to the UI
            aac.SendCommand("EndEvent");

            aac.WaitForUiToLoad();

            JobStage = "";

            this.Result = JobResult.Passed;
        }
    }

    public class AmaxMemoryLeakJob : AmaxRaceTestJob
    {
        public AmaxMemoryLeakJob(string name, TimeSpan raceDuration, uint tryGoingInAndOut, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
            : base(name, raceDuration, tryGoingInAndOut, kitRequirements, buildConfig)
        {
            this.RebootKitsBeforeRunning = true;
        }

        protected override void PostWork(List<AmaxAutomationConnection> connections)
        {
            AmaxAutomationConnection aac = connections[0];

            CommandResult cr = aac.SendCommand("CheckMemLeak");

            string[] memInfo = cr.SingleLine.Split(':');

            bool frontendNoAllocs = (memInfo[1] == "1");
            uint frontendAllocationCount = uint.Parse(memInfo[2]);
            uint frontendLeakSystem = uint.Parse(memInfo[3]);
            uint frontendLeakPhysical = uint.Parse(memInfo[4]);

            bool gameNoAllocs = (memInfo[5] == "1");
            uint gameAllocationCount = uint.Parse(memInfo[6]);
            uint gameLeakSystem = uint.Parse(memInfo[7]);
            uint gameLeakPhysical = uint.Parse(memInfo[8]);

            uint physicalDifference = uint.Parse(memInfo[9]);
            uint totalDifference = uint.Parse(memInfo[10]);

            // Check results
            if (physicalDifference > 0)
            {
                this.AddToComment(string.Format("SOME PHYSICAL MEMORY NOT TRACKED ({0})", physicalDifference));
                this.Result = JobResult.Failed;
            }

            if (frontendLeakPhysical > 0 && gameLeakPhysical > 0)
            {
                this.AddToComment("PHYSICAL LEAK(S) DETECTED IN BOTH FRONTEND AND IN-GAME!");
            }
            else if (frontendLeakPhysical > 0)
            {
                this.AddToComment("PHYSICAL LEAK(S) IN FRONTEND DETECTED!");
            }
            else if (gameLeakPhysical > 0)
            {
                this.AddToComment("PHYSICAL LEAK(S) IN-GAME DETECTED!");
            }

            if (!frontendNoAllocs && frontendAllocationCount > 0)
            {
                string message = string.Format("Average of Allocs({0}) System({1}) Physical({2})", frontendAllocationCount, frontendLeakSystem, frontendLeakPhysical);

                this.AddToComment("Frontend: " + message);

                this.Result = JobResult.Failed;
            }

            if (!gameNoAllocs && gameAllocationCount > 0)
            {
                string message = string.Format("Average of Allocs({0}) System({1}) Physical({2})", gameAllocationCount, gameLeakSystem, gameLeakPhysical);
                
                this.AddToComment("InGame: " + message);
                
                this.Result = JobResult.Failed;
                this.FailReason = JobFailReason.MemoryLeak;
                this.ResultsData = new MemoryLeakInfo() { System = frontendLeakSystem + gameLeakSystem, Physical = frontendLeakPhysical + gameLeakPhysical };
            }
        }
    }
}
