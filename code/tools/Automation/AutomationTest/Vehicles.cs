namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    using ExcelXML;

    public class VehicleTests
    {
        public static void CreateJobs_AllCars(AmaxXtHelper xt, string exe, bool take360Shots)
        {
            // Find all routes flagged for testing, sort them by route index
            var vehicles = from v in xt.Vehicles orderby v.Name select v;

            foreach (var v in vehicles)
            {
                bool use = WorkManager.PickNMixValid ? WorkManager.PassPickNMix("AmaxVehicle", v.Id) : (v.Use == true);

                if (use)
                {
                    string jobName = v.Name + string.Format(" ({0:X8})", v.Id);

                    //// AutomationHelper.DebugOutput.Write("Found: " + jobName);

                    AmaxCity city = xt.City("UK_London_Hackney");
                    AmaxRoute route = xt.Route(city.Id, 1);

                    AmaxVehicleLoadTestJob job = new AmaxVehicleLoadTestJob(jobName, AutomationKitType.Xbox360, exe);

                    job.Take360ScreenShots = take360Shots;

                    //job.CityId = city.Id;
                    job.RouteId = route.Id;

                    job.AddCar(v.Id);

                    WorkManager.AddJob(job);
                }
            }
        }

        public static void CreateReport_Vehicles(Workbook wb)
        {
            Workbook.CellStyle stylePass = wb.FindStyle("ss:StylePass");
            Workbook.CellStyle styleFail = wb.FindStyle("ss:StyleFail");
            Workbook.CellStyle styleSkipped = wb.FindStyle("ss:StyleSkipped");
            
            Workbook.Worksheet ws = new Workbook.Worksheet("Results");

            // Header
            ExcelXML.Workbook.Worksheet.Row headerRow = new Workbook.Worksheet.Row();

            headerRow.AddCell("ID");
            headerRow.AddCell("Name");
            headerRow.AddCell("Result");
            headerRow.AddCell("Comments");

            ws.Rows.Add(headerRow);

            var vehicleJobs = from vj in WorkManager.Jobs where vj is AmaxVehicleLoadTestJob select vj;

            foreach (IJob j in vehicleJobs)
            {
                Workbook.Worksheet.Row row = ws.AddRow();

                row.AddCell(j.ID);
                row.AddCell(j.Name);

                Workbook.Worksheet.Row.Cell resultCell = row.AddCell(j.Result.ToString());

                // Select a cell style based on the job result
                switch (j.Result)
                {
                    case JobResult.Passed:
                        resultCell.Style = stylePass;
                        break;

                    case JobResult.Skipped:
                        resultCell.Style = styleSkipped;
                        break;

                    case JobResult.Failed:
                        resultCell.Style = styleFail;
                        break;
                }

                resultCell.Comment = j.JobStage;

                if (j.Comment != null)
                {
                    row.AddCell(j.Comment);
                }
                else if (j.Result == JobResult.Failed)
                {
                    row.AddCell(j.FailReason.ToString());
                }
            }

            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 50 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 280 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 60 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 100 });

            wb.Worksheets.Add(ws);
        }
    }
}