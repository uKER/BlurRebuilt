namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    using Bizarre.Misc.ExcelXML;

    public class VehicleTests
    {
        public class VehicleSuite : TestSuite
        {
            private bool take360Shots;

            public VehicleSuite(AmaxXtHelper context, bool take360Shots)
                : base(context, "Vehicles")
            {
                this.take360Shots = take360Shots;
            }

            public override void CreateJobs(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                // Find all routes flagged for testing, sort them by route index
                var vehicles = from v in xt.Vehicles orderby v.Name select v;

                foreach (var v in vehicles)
                {
                    bool use = WorkManager.PickNMixValid ? WorkManager.PassPickNMix("AmaxVehicle", v.Id) : (v.Use == true);

                    if (use)
                    {
                        string jobName = v.Name + string.Format(" ({0:X8})", v.Id);

                        //// AutomationHelper.DebugOutput.Write("Found: " + jobName);

                        AmaxCity city = xt.City("Kanzen_LA_River"); // UK_London_Hackney;
                        AmaxRoute route = xt.Route(city.Id, 1); // 1

                        KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1,  };

                        AmaxVehicleLoadTestJob job = new AmaxVehicleLoadTestJob(jobName, kitRequirements, WorkManager.BuildConfigs[0]);

                        job.ShowHUD = false;

                        job.Take360ScreenShots = take360Shots;

                        job.RouteId = route.Id;

                        job.AddCar(v.Id);

                        job.AddDependency("BASIC-1");

                        Add(job);
                    }
                }
            }
        }
    }
}