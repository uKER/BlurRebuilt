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

    // TODO: How many times in/out

    public class StabilityTests
    {
        public class MemLeakSuite : TestSuite
        {
            bool justRunMemLeakTest = false;
            bool quickRun = false;
            bool basicTestOnly = true;

            public MemLeakSuite(object context)
                : base(context, "Stability")
            {
            }

            public MemLeakSuite(AmaxXtHelper context, bool basicTestOnly, bool quickRun, bool justRunMemLeakTest)
                : base(context, "Stability")
            {
                this.quickRun = quickRun;
                this.justRunMemLeakTest = justRunMemLeakTest;
                this.basicTestOnly = basicTestOnly;
            }

            public override void CreateJobs(object context)
            {
                CreateJobs_BasicChecks(context as AmaxXtHelper);

                if (!basicTestOnly)
                {
                    if (!this.justRunMemLeakTest)
                    {
                        CreateJobs_MemLeakCheck(context as AmaxXtHelper);
                    }
                    else
                    {
                        CreateJob_MemLeakRun(context as AmaxXtHelper);
                    }
                }
            }

            private void CreateJob_MemLeakRun(AmaxXtHelper xt)
            {
                uint numCars = 8;

                AmaxCity city = xt.City("USA_LA_River"); // UK_London_Hackney
                AmaxRoute route = xt.Route(city.Id, 3); // 1

                AmaxVehicle vehicle = xt.Vehicle("Shelby_MustangGT500_Stock");

                foreach (var kitRequest in WorkManager.KitServer.RequestEachAvailable(1))
                {
                    AmaxMemoryLeakJob job = new AmaxMemoryLeakJob("Memory Leak - " + numCars + " car(s)", TimeSpan.FromSeconds(45), 6, kitRequest, WorkManager.BuildConfigs[0])
                    {
                        RouteId = route.Id,
                    };

                    job.RebootKitsBeforeRunning = true;

                    for (int c = 0; c < numCars; c++)
                    {
                        job.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                    }

                    Add(job);
                }
            }

            private void CreateJobs_BasicChecks(AmaxXtHelper xt)
            {
                AmaxCity city = xt.City("USA_LA_River"); // UK_London_Hackney
                if (city != null)
                {
                    AmaxRoute route = xt.Route(city.Id, 3); // 1

                    if (route != null)
                    {
                        AmaxVehicle vehicle = xt.Vehicle("Shelby_MustangGT500_Stock");

                        foreach (var kitRequest in WorkManager.KitServer.RequestEachAvailable(1))
                        {
                            TestFinish tfc = new TestFinish(TimeSpan.FromSeconds(15));

                            AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob("Level Load - 0 cars", "Stability", tfc, kitRequest, WorkManager.BuildConfigs[0]);
                            job.RouteId = route.Id;
                            job.Tag = "BASIC-0";
                            Add(job);

                            AmaxCircuitLoadTestJob job2 = new AmaxCircuitLoadTestJob("Level Load - 1 car", "Stability", tfc, kitRequest, WorkManager.BuildConfigs[0]);
                            job2.RouteId = route.Id;
                            job2.Tag = "BASIC-1";
                            job2.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                            Add(job2);
                        }
                    }
                }
            }

            private void CreateJobs_MemLeakCheck(AmaxXtHelper xt)
            {
                AmaxCity city = xt.City("USA_LA_River"); // UK_London_Hackney
                AmaxRoute route = xt.Route(city.Id, 3); // 1

                AmaxVehicle vehicle = xt.Vehicle("Shelby_MustangGT500_Stock");

                foreach (var kitRequest in WorkManager.KitServer.RequestEachAvailable(1))
                {
                    int passes = this.quickRun ? 0 : 2;

                    for (int i = 0; i <= passes; i++)
                    {
                        uint numCars;

                        switch (i)
                        {
                            case 1:
                                numCars = 1;
                                break;

                            case 2:
                                numCars = 8;
                                break;

                            default:
                                numCars = 0;
                                break;
                        }

                        TestFinish tfc = new TestFinish(TimeSpan.FromSeconds(20));

                        AmaxMemoryLeakJob job = new AmaxMemoryLeakJob("Memory Leak - " + numCars + " car(s)", TimeSpan.FromSeconds(20), 6, kitRequest, WorkManager.BuildConfigs[0])
                        {
                            RouteId = route.Id,
                        };

                        for (int c = 0; c < numCars; c++)
                        {
                            job.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                        }

                        if (numCars == 0)
                        {
                            job.AddDependency("BASIC-0");
                        }
                        else
                        {
                            job.AddDependency("BASIC-1");
                        }

                        Add(job);
                    }
                }
            }
        }
    }
}