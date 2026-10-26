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

                AmaxCity city = xt.City("Kanzen_LA_River");
                AmaxRoute route = xt.Route(city.Id, 1);

                AmaxVehicle vehicle = xt.Vehicle("Shelby_MustangGT500_Stock");

				KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1,  };

                AmaxMemoryLeakJob job = new AmaxMemoryLeakJob("Memory Leak - " + numCars + " car(s)", TimeSpan.FromSeconds(45), 6, kitRequirements, WorkManager.BuildConfigs[0])
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

            private void CreateJobs_BasicChecks(AmaxXtHelper xt)
            {
                AmaxCity city = xt.City("Kanzen_LA_River");
                AmaxRoute route = xt.Route(city.Id, 1);

                AmaxVehicle vehicle = xt.Vehicle("Shelby_MustangGT500_Stock");
                AmaxVehicle vehicle2 = xt.Vehicle("Audi_TT_Stock");
                AmaxVehicle vehicle3 = xt.Vehicle("Ford_F150_Smooth");
                AmaxVehicle vehicle4 = xt.Vehicle("Mazda_3MPS_Rat");
                AmaxVehicle vehicle5 = xt.Vehicle("Corvette_C3_Rat");

				KitServer.KitRequest kitRequirements360 = new KitServer.KitRequest() { XBOX_360 = 1 };
				KitServer.KitRequest kitRequirementsPS3 = new KitServer.KitRequest() { PS3 = 1 };


				AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob("Level Load - 1 car", "Stability", new TestFinish(TimeSpan.FromSeconds(15)), kitRequirements360, WorkManager.BuildConfigs[0]);
                job.RouteId = route.Id;
                job.Tag = "BASIC-0";
                job.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                Add(job);

				AmaxCircuitLoadTestJob jobPS3 = new AmaxCircuitLoadTestJob("Level Load - 1 car", "Stability", new TestFinish(TimeSpan.FromSeconds(15)), kitRequirementsPS3, WorkManager.BuildConfigs[1]);
				jobPS3.RouteId = route.Id;
				jobPS3.Tag = "BASIC-0";
				jobPS3.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				//Add(jobPS3);

				AmaxCircuitLoadTestJob job2 = new AmaxCircuitLoadTestJob("Level Load - 20 cars", "Stability", new TestFinish(TimeSpan.FromSeconds(15)), kitRequirements360, WorkManager.BuildConfigs[0]);
                job2.RouteId = route.Id;
                job2.Tag = "BASIC-1";
                job2.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                job2.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
                //Add(job2);

				AmaxCircuitLoadTestJob job2PS3 = new AmaxCircuitLoadTestJob("Level Load - 20 cars", "Stability", new TestFinish(TimeSpan.FromSeconds(15)), kitRequirementsPS3, WorkManager.BuildConfigs[1]);
				job2PS3.RouteId = route.Id;
				job2PS3.Tag = "BASIC-1";
				job2PS3.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle2.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle3.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle4.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				job2PS3.AddCar(vehicle5.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);
				//Add(job2PS3);
            }

            private void CreateJobs_MemLeakCheck(AmaxXtHelper xt)
            {
                AmaxCity city = xt.City("Kanzen_LA_River");
                AmaxRoute route = xt.Route(city.Id, 1);

                AmaxVehicle vehicle = xt.Vehicle("Shelby_MustangGT500_Stock");

				KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1,  };

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

                    AmaxMemoryLeakJob job = new AmaxMemoryLeakJob("Memory Leak - " + numCars + " car(s)", TimeSpan.FromSeconds(20), 6, kitRequirements, WorkManager.BuildConfigs[0])
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