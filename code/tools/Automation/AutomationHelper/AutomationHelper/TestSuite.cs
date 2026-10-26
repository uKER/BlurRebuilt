namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Automation;
    using Bizarre.Misc.ExcelXML;

    // TODO: Need to keep track of which jobs come from which suite


    /// <summary>
    /// A class to make writing tests easier and formulate reports
    /// </summary>
    public abstract class TestSuite
    {
        static uint globalTestSuiteId = 1;

        static object lockObj = new object();

        private string name;
        private object context;

        private uint testSuiteId;

        public TestSuite(object context, string name)
        {
            this.name = name;
            this.context = context;

            lock (lockObj)
            {
                this.testSuiteId = globalTestSuiteId++;
            }
        }

        public object Context
        {
            get
            {
                return this.context;
            }
        }

        public string Name
        {
            get
            {
                return this.name;
            }
        }

        public uint Id
        {
            get
            {
                return this.testSuiteId;
            }
        }

        public abstract void CreateJobs(object context);

        public void Add(IJob job)
        {
            job.TestSuite = testSuiteId;

            WorkManager.AddJob(job);
        }

        public virtual void AddAdditionalColumns(AdditionalColumns additionalColumns)
        {
        }

        public virtual void AdditionalColumnValue(IJob job, TestSuite.AdditionalColumn columnName, Workbook.Worksheet workSheet, Workbook.Worksheet.Row.Cell cell)
        {
        }

        public virtual void AddAdditionalPostColumns(AdditionalColumns additionalColumns)
        {
        }

        public virtual void AdditionalPostColumnValue(IJob job, TestSuite.AdditionalColumn column, Workbook.Worksheet workSheet, Workbook.Worksheet.Row.Cell cell)
        {
        }

        /// <summary>
        /// Add a standard header row to a worksheet. If you want something different you need to override it
        /// </summary>
        /// <param name="row">A row object to which we add the cells for the header</param>
        public virtual void AddHeader(Workbook.Worksheet worksheet, TestSuite.AdditionalColumns additionalColumns, TestSuite.AdditionalColumns additionalPostColumns)
        {
            Workbook.CellStyle styleCentre = new Workbook.CellStyle();
            styleCentre.HorizontalAlignment = Workbook.eAlignment.Center;
            worksheet.Workbook.Styles.Add(styleCentre);

            Workbook.Worksheet.Row rowT = worksheet.AddRow();

            // ID and Name
            rowT.AddCell("");
            rowT.AddCell("");

            // Add a blank column for each additional column that was added
            if (additionalColumns.columns.Count > 0)
            {
                Workbook.Worksheet.Row.Cell cac = rowT.AddCell(additionalColumns.GroupTitle);
                cac.MergeAcross = (uint)additionalColumns.columns.Count-1;
                cac.Style = styleCentre;
            }

            Workbook.Worksheet.Row.Cell lc = rowT.AddCell("Log Output");
            lc.MergeAcross = 2;
            lc.Style = styleCentre;

            rowT.AddCell(""); // Result

            if (additionalPostColumns.columns.Count > 0)
            {
                Workbook.Worksheet.Row.Cell capc = rowT.AddCell(additionalPostColumns.GroupTitle);
                capc.MergeAcross = (uint)additionalPostColumns.columns.Count-1;
                capc.Style = styleCentre;
            }

            // Next Row
            Workbook.Worksheet.Row row = worksheet.AddRow();

            row.AddCell("ID");
            row.AddCell("Name");

            Workbook.CellStyle centreStyle = new Workbook.CellStyle() { HorizontalAlignment = Workbook.eAlignment.Center, WrapText = true };
            worksheet.Workbook.Styles.Add(centreStyle);

            // Any additional columns
            foreach (var ac in additionalColumns.columns)
            {
                Workbook.Worksheet.Row.Cell cell = row.AddCell(ac.Name);

                if (ac.Comment.Length > 0)
                {
                    cell.Comment = ac.Comment;
                }

                cell.Style = centreStyle;
            }

            row.AddCell("Warn");
            row.AddCell("Error");
            row.AddCell("Fatal");

            row.AddCell("Result");

            worksheet.FreezeRows = 2;

            // Any additional columns after comments
            foreach (var ac in additionalPostColumns.columns)
            {
                Workbook.Worksheet.Row.Cell cell = row.AddCell(ac.Name);

                if (ac.Comment.Length > 0)
                {
                    cell.Comment = ac.Comment;
                }

                cell.Style = centreStyle;
            }

            row.AddCell("Comments");
        }

        /// <summary>
        /// A function that gets called for each job we performed. By default it adds a
        /// row to the worksheet and adds the standard data
        /// </summary>
        /// <param name="worksheet">The current worksheet for this suite</param>
        public virtual void AddDataRow(IJob job, Workbook.Worksheet worksheet, TestSuite.AdditionalColumns additionalColumns, TestSuite.AdditionalColumns additionalPostColumns)
        {
            Workbook.CellStyle stylePass = worksheet.Workbook.FindStyle("ss:StylePass");
            Workbook.CellStyle styleFail = worksheet.Workbook.FindStyle("ss:StyleFail");
            Workbook.CellStyle styleSkipped = worksheet.Workbook.FindStyle("ss:StyleSkipped");

            Workbook.Worksheet.Row row = worksheet.AddRow();

            row.AddCell(job.ID);

            string jobName = job.Name;

			if (job.KitRequirements.PS3 > 0)
			{
                jobName += string.Format(" - {0} x PS3", job.KitRequirements.PS3);
			}
			if (job.KitRequirements.XBOX_360 > 0)
			{
                jobName += string.Format(" - {0} x 360", job.KitRequirements.XBOX_360);
			}
			if (job.KitRequirements.WIN32 > 0)
			{
                jobName += string.Format(" - {0} x Win32", job.KitRequirements.WIN32);
			}
			if (job.KitRequirements.WIN64 > 0)
			{
                jobName += string.Format(" - {0} x Win64", job.KitRequirements.WIN64);
			}
            row.AddCell(jobName);

            // Any additional columns
            foreach (var ac in additionalColumns.columns)
            {
                Workbook.Worksheet.Row.Cell cell = row.AddCell();

                AdditionalColumnValue(job, ac, worksheet, cell);
            }

            row.AddCell(job.LogWarnings);
            row.AddCell(job.LogErrors);
            row.AddCell(job.LogFatalErrors);

            Workbook.Worksheet.Row.Cell resultCell = row.AddCell(job.Result.ToString());

            // Select a cell style based on the job result
            switch (job.Result)
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

            // Any additional post-columns
            foreach (var ac in additionalPostColumns.columns)
            {
                Workbook.Worksheet.Row.Cell cell = row.AddCell();

                AdditionalPostColumnValue(job, ac, worksheet, cell);
            }

            resultCell.Comment = job.JobStage;

            if (job.Comment != null)
            {
                row.AddCell(job.Comment);
            }
            else if (job.Result == JobResult.Failed)
            {
                row.AddCell(job.FailReason.ToString());
            }
        }

        public virtual void SetColumnWidths(Workbook.Worksheet worksheet, TestSuite.AdditionalColumns additionalColumnWidths, TestSuite.AdditionalColumns additionalPostColumnWidths)
        {
            // ID + Name
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 50 });
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 280 });

            // Any additional columns
            foreach (var acw in additionalColumnWidths.columns)
            {
                worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = acw.Width });
            }

            // Log output
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 30 });
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 30 });
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 30 });

            // Result + Comments
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 60 });

            // Any additional post-columns
            foreach (var acw in additionalPostColumnWidths.columns)
            {
                worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = acw.Width });
            }

            // Comments
            worksheet.Columns.Add(new Workbook.Worksheet.Column() { Width = 100 });
        }

        public class AdditionalColumn
        {
            public string Name = string.Empty;
            public string Comment = string.Empty;
            public uint Width = 50;
        }

        public class AdditionalColumns
        {
            public string GroupTitle = string.Empty;

            public List<AdditionalColumn> columns = new List<AdditionalColumn>();
        }
    }
}
