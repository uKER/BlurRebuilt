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

            public override void AddAdditionalColumns(TestSuite.AdditionalColumns additionalColumns)
            {
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Size(k)", Width = 100 });
            }

            public override void AddAdditionalPostColumns(TestSuite.AdditionalColumns additionalColumns)
            {
                additionalColumns.GroupTitle = "Problems";

                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Default Car", Width = 50, Comment = "Missing mesh file" });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Collision Mesh", Width = 50 });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Shadow Caster", Width = 50 });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Dynamic ObjMesh", Width = 50 });

                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Collider Mesh", Width = 50, Comment = "No Car Collider Collision Mesh" });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Invalid PartCode", Width = 50, Comment = "Invalid PartSubCode(s)" });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Exhaust Emitter", Width = 50, Comment = "No Exhaust Emitter In Sorcery" });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Damage Particles", Width = 50, Comment = "No Damage Particle Effects In Sorcery" });
            }

            public override void AdditionalColumnValue(IJob job, TestSuite.AdditionalColumn column, Workbook.Worksheet worksheet, Workbook.Worksheet.Row.Cell cell)
            {
                AmaxVehicleLoadTestJob vehicleJob = job as AmaxVehicleLoadTestJob;

                switch (column.Name)
                {
                    case "Size(k)":
                        if (vehicleJob.SizeInMemory > 0)
                        {
                            cell.Value = (int)(vehicleJob.SizeInMemory / 1024);
                        }
                        break;
                }
            }

            public override void AdditionalPostColumnValue(IJob job, TestSuite.AdditionalColumn column, Workbook.Worksheet worksheet, Workbook.Worksheet.Row.Cell cell)
            {
                AmaxVehicleLoadTestJob vehicleJob = job as AmaxVehicleLoadTestJob;

                if (job.Result != JobResult.Skipped)
                {
                    bool problemFound = false;
                    string comment = null;

                    switch (column.Name)
                    {
                        case "Default Car":
                            problemFound = vehicleJob.vehicleProblems.defaultCar;
                            break;

                        case "Collision Mesh":
                            problemFound = vehicleJob.vehicleProblems.missingCollisionMesh;
                            break;

                        case "Shadow Caster":
                            problemFound = vehicleJob.vehicleProblems.missingShadowCaster;
                            break;

                        case "Dynamic ObjMesh":
                            problemFound = vehicleJob.vehicleProblems.missingDynamicObjectMesh;
                            break;

                        case "Collider Mesh":
                            problemFound = vehicleJob.vehicleProblems.missingCarColliderMesh;
                            break;

                        case "Invalid PartCode":
                            problemFound = vehicleJob.vehicleProblems.invalidPartCodes;
                            comment = vehicleJob.vehicleProblems.partCodes;
                            break;
                        
                        case "Exhaust Emitter":
                            problemFound = vehicleJob.vehicleProblems.noExhaustEmitterInSorcery;
                            break;

                        case "Damage Particles":
                            problemFound = vehicleJob.vehicleProblems.noDamageParticleEffectsInSorcery;
                            break;
                    }

                    if (problemFound)
                    {
                        cell.Value = problemFound ? "FAIL" : "OK";
                        cell.Style = problemFound ? worksheet.Workbook.FindStyle("ss:StyleFail") : worksheet.Workbook.FindStyle("ss:StylePass");
                        cell.Comment = comment;
                    }
                }
                else
                {
                    //cell.Style = worksheet.Workbook.FindStyle("ss:StyleSkipped");
                }
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

                        AmaxCity city = xt.City("VehicleTestScene"); // UK_London_Hackney;
                        AmaxRoute route = xt.Route(city.Id, 0); // 1

                        foreach (var kitRequest in WorkManager.KitServer.RequestEachAvailable(1))
                        {
                            AmaxVehicleLoadTestJob job = new AmaxVehicleLoadTestJob(jobName, kitRequest, WorkManager.BuildConfigs[0]);

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
}