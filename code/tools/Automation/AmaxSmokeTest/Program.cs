namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    using Bizarre.Misc;

    public class Program
    {
        private static void Main(string[] args)
        {
            // Initialise the automation system and parse command line arguements
            if (!WorkManager.Initialise("Amax SmokeTest", args))
                return;
            
            // TODO: Need to name the type of project

            // Load in Sorcery data so we can use it to generate tests
            AmaxXtHelper xt = new AmaxAutomationHelper.AmaxXtHelper();
            if (xt.IsLoaded == false)
            {
                xt.Open(@"Z:\Amax\sorcery3\main");
                
                if (xt.IsLoaded == false)
                {
                    WorkManager.MasterLog.WriteLine("ERROR: Problem opening Sorcery database");
                    return;
                }
            }

            // Temporary hack to only run a small number of jobs
            //WorkManager.SetJobRange(50, 50);

            bool basicStabilityOnly = true;

            Arguments argParser = new Arguments(args);

            if (argParser["stability"] != null)
            {
                basicStabilityOnly = false;
            }

            WorkManager.TestSuites.Add(new StabilityTests.MemLeakSuite(xt, basicStabilityOnly, false, false));

            if (argParser["dash"] != null)
            {
                WorkManager.TestSuites.Add(new RouteTests.DashTestSuite(xt, argParser["dash"]));
            }

            if (argParser["load"] != null)
            {
                //RaceTests.CreateJobs_CircuitLoad(xt, amaxExeName);
            }

            if (argParser["vehicles"] != null)
            {
                WorkManager.TestSuites.Add(new VehicleTests.VehicleSuite(xt, true));
            }

            if (argParser["events"] != null)
            {
                WorkManager.TestSuites.Add(new EventTests.EventTestSuite(xt, argParser["events"]));
            }

            if (argParser["perf"] != null)
            {
                WorkManager.TestSuites.Add(new PerfTests.PerfSuite(xt, argParser["perf"]));
            }

            WorkManager.Process();
        }
    }
}
