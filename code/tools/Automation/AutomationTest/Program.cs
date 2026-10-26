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
    using ExcelXML;

    public class Program
    {
        private static string amaxExeName = "amax.xex";

        private static void CreateJobs_Test(AmaxXtHelper xt)
        {
            AmaxCity city = xt.City("USA_LA_River");
            AmaxRoute route = xt.Route(city.Id, 1);

            string jobName = city.Name + " " + route.Index;

            WorkManager.MasterLog.WriteLine("Found: " + jobName);

            AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob(jobName, AutomationKitType.Xbox360, amaxExeName);

            //job.CityId = city.Id;
            job.RouteId = route.Id;

            WorkManager.AddJob(job);
        }

        private static void CreateReport()
        {
            Workbook wb = new Workbook();

            VehicleTests.CreateReport_Vehicles(wb);
            WorkManager.CreateReportSheet_JobKits(wb);

            wb.Save(System.IO.Path.Combine(WorkManager.ReportFolder, "results.xml"));
        }

        private static void Main(string[] args)
        {
            // Initialise the automation system and parse command line arguements
            WorkManager.Initialise("Amax SmokeTest", args);
            
            // TODO: Need to name the type of project

            // Load in Sorcery data so we can use it to generate tests
            AmaxXtHelper xt = new AmaxAutomationHelper.AmaxXtHelper();
            if (xt.IsLoaded == false)
            {
                xt.Open(@"Z:\Amax\sorcery3\main");
            }

            // TODO: Need to set path and exe in a block for all 3 platforms - then set per job

            // Set build path on devkit
            AutomationConnection.SetBuildPath(@"e:\Amax\");

            // Name of executable to run
            amaxExeName = "amax.xex";

            // Temporary hack to only run a small number of jobs
            //WorkManager.SetJobRange(12, 16);

            Arguments argParser = new Arguments(args);

            if (argParser["routes"] == "true")
            {
                RouteTests.CreateJobs_AllFlaggedRoutes(xt, amaxExeName);
            }

            if (argParser["vehicles"] == "true")
            {
                VehicleTests.CreateJobs_AllCars(xt, amaxExeName, true);
            }

            //RaceTests.CreateJobs(xt, amaxExeName);
            
            WorkManager.Process(0);

            WorkManager.DumpReport();

            CreateReport();
        }
    }
}
