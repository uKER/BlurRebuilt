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
			WorkManager.Initialise("Racer2010 SmokeTest", args);
            
			// TODO: Need to name the type of project

			// Load in Sorcery data so we can use it to generate tests
			AmaxXtHelper xt = new AmaxAutomationHelper.AmaxXtHelper();
			if (xt.IsLoaded == false)
			{
				if (!xt.Open(@"Z:\Racer2010\sorcery3\main"))
				{
					Console.Write("Could not connect to Sorcery database. If you have Sorcery open, close it and try again./n");
				}

			}

			if (xt.IsLoaded)
			{

				// Temporary hack to only run a small number of jobs
				//WorkManager.SetJobRange(50, 50);

				bool basicStabilityOnly = true;

				Arguments argParser = new Arguments(args);

				if (argParser["stability"] == "true")
				{
					basicStabilityOnly = false;
				}

				WorkManager.TestSuites.Add(new StabilityTests.MemLeakSuite(xt, basicStabilityOnly, false, false));

				if (argParser["dash"] == "true")
				{
					WorkManager.TestSuites.Add(new RouteTests.DashTestSuite(xt));
				}

				if (argParser["load"] == "true")
				{
					//RaceTests.CreateJobs_CircuitLoad(xt, amaxExeName);
				}

				if (argParser["vehicles"] == "true")
				{
					WorkManager.TestSuites.Add(new VehicleTests.VehicleSuite(xt, true));
				}

				if (argParser["events"] == "true")
				{
					WorkManager.TestSuites.Add(new EventTests.EventTestSuite(xt));
				}

				WorkManager.Process();
			}

            Console.Write("TEST FINISHED. PRESS ANY KEY TO CONTINUE...");
            Console.ReadKey();
        }
    }
}
