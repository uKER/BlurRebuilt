namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    public class RouteTests
    {
        /// <summary>
        /// Test suite to load routes and drive 20 cars for 30secs (then take a picture)
        /// </summary>
        public class DashTestSuite : TestSuite
        {
            public DashTestSuite(AmaxXtHelper context)
                : base(context, "Dash")
            {
            }

            public override void CreateJobs(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                AmaxVehicle shelby = xt.Vehicle("Shelby_MustangGT500_Stock");

                var dashRoutes = from r in xt.Routes orderby r.City.Name, r.Index select r;

                foreach (var r in dashRoutes)
                {
                    // Check if this route is allowed for testing
                    if (WorkManager.PassPickNMix("Route", r.Id))
                    {
                        string jobName = r.Name + string.Format(" ({0:X8})", r.Id);

                        //// WorkManager.MasterLog.WriteLine("Found: " + jobName);

						KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1,  };

                        AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob(jobName, "Dash", new TestFinish(TimeSpan.FromSeconds(30)), kitRequirements, WorkManager.BuildConfigs[0]);

                        job.RouteId = r.Id;
                        //job.SetRandomCars(20, xt);
                        for (int iCar = 0; iCar < 20; iCar++)
                        {
                            job.Vehicles.Add(new AmaxJob.AmaxJobVehicle() { Id = shelby.Id, ControlType = AmaxJob.AmaxJobVehicle.eControlType.Ai });
                        }

                        job.AddDependency("BASIC-1");

                        Add(job);
                    }
                }
            }
        }

        //public class RouteTestSuite : TestSuite
        //{
        //    public RouteTestSuite(AmaxXtHelper context)
        //        : base(context, "Routes")
        //    {
        //    }

        //    public override void CreateJobs(object context)
        //    {
        //        AmaxXtHelper xt = context as AmaxXtHelper;

        //        //WorkManager.Reports += new WorkManager.CreateReport(VehicleTests.CreateReport_Vehicles);

        //        // Find all routes flagged for testing, sort them by route index
        //        //var routes = from r in xt.Routes where (r.AutomatedTesting == true) orderby r.City, r.Index select r;
        //        var routes = from r in xt.Routes orderby r.City, r.Index select r;

        //        foreach (var route in routes)
        //        {
        //            if (WorkManager.PassPickNMix("Route", route.Id))
        //            {
        //                AmaxCity city = xt.City(route.City);

        //                string jobName = city.Name + " " + route.Index;

        //                WorkManager.MasterLog.WriteLine("Found: " + jobName);

        //                KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

        //                AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob(jobName, kitRequirements, WorkManager.BuildConfigs[0]);

        //                //job.CityId = city.Id;
        //                job.RouteId = route.Id;

        //                Add(job);
        //            }
        //        }
        //    }
        //}
    }
}
