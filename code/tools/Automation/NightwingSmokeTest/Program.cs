namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using NightwingAutomation;
    using NightwingAutomationHelper;
    using Automation;

    using Bizarre.Misc;

    public class Program
    {
        private static void Main(string[] args)
        {
            // Initialise the automation system and parse command line arguements
            if (!WorkManager.Initialise("Nightwing SmokeTest", args))
                return;
            
            // TODO: Need to name the type of project

            // Load in Sorcery data so we can use it to generate tests
            NightwingXtHelper xt = new NightwingAutomationHelper.NightwingXtHelper();
            if (xt.IsLoaded == false)
            {
                xt.Open(@"Z:\Nightwing\sorcery3\main");
            }

            // Temporary hack to only run a small number of jobs
            //WorkManager.SetJobRange(1, 4);

            //bool basicStabilityOnly = true;

            Arguments argParser = new Arguments(args);

            //if (argParser["stability"] == "true")
            //{
            //    basicStabilityOnly = false;
            //}

            //WorkManager.TestSuites.Add(new StabilityTests.MemLeakSuite(xt, basicStabilityOnly, false, false));

            //if (argParser["dash"] == "true")
            //{
            //    WorkManager.TestSuites.Add(new RaceTests.RaceTestSuite(xt));
            //}

            //if (argParser["load"] == "true")
            //{
            //    //RaceTests.CreateJobs_CircuitLoad(xt, amaxExeName);
            //}

            //if (argParser["vehicles"] == "true")
            //{
            //    WorkManager.TestSuites.Add(new VehicleTests.VehicleSuite(xt, true));
            //}

            WorkManager.TestSuites.Add(new LevelTests.LevelSuite(xt, true));

            WorkManager.Process();
        }
    }
}
