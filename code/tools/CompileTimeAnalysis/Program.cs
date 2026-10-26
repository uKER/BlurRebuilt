using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using ExcelXML;

// TODO: count lines of code in file and display

// Z:\tools\common_bin\jam.exe -a -c -d0 -p -sSERVER_BUILD=1 -sTARGET=XBOX_360 -sBUILD=Release

// E:\devAmax\code\mercurygame\game\360r.txt E:\devAmax\code\mercurygame\game\ps3r.txt E:\devAmax\code\mercurygame\game\pcr.txt

// E:\crap\nw_360_r.txt

namespace CompileTimeAnalysis
{
    public class Library
    {
        public Library()
        {
            Duration360 = 0;
            DurationPS3 = 0;
            DurationPC = 0;
            FileCount = 0;
        }

        public string Name { get; set; }
        public string Path { get; set; }

        public Int32 FileCount { get; set; }
        public Int32 Duration360 { get; set; }
        public Int32 DurationPS3 { get; set; }
        public Int32 DurationPC { get; set; }
    }

    public class FileInfo
    {
        public enum Platforms
        {
            Platform_360,
            Platform_PS3,
            Platform_PC,
            Unknown
        };

        public static Platforms platform = Platforms.Unknown;

        public FileInfo()
        {
            Filename = "";
            Duration360 = Int32.MinValue;
            DurationPS3 = Int32.MinValue;
            DurationPC = Int32.MinValue;
        }

        public string Filename { get; set; }
        public Library Library { get; set; }
        public bool SPU { get; set; }

        public Int32 PlatformDuration(Platforms p)
        {
            switch (p)
            {
                case Platforms.Platform_360:
                    return Duration360;
                case Platforms.Platform_PS3:
                    return DurationPS3;
                case Platforms.Platform_PC:
                    return DurationPC;
            }

            return 0;
        }

        public Int32 Duration
        {
            get
            {
                System.Diagnostics.Debug.Assert(platform != Platforms.Unknown);
                
                switch (platform)
                {
                    case Platforms.Platform_360:
                        return Duration360;
                    case Platforms.Platform_PS3:
                        return DurationPS3;
                    case Platforms.Platform_PC:
                        return DurationPC;
                }

                return 0;
            }

            set
            {
                System.Diagnostics.Debug.Assert(platform != Platforms.Unknown);

                switch (platform)
                {
                    case Platforms.Platform_360:
                        Duration360 = value;
                        break;
                    case Platforms.Platform_PS3:
                        DurationPS3 = value;
                        break;
                    case Platforms.Platform_PC:
                        DurationPC = value;
                        break;
                }
            }
        }
        
        protected Int32 Duration360 { get; set; }
        protected Int32 DurationPS3 { get; set; }
        protected Int32 DurationPC { get; set; }
    }

    class Program
    {
        static List<Library> libs = new List<Library>();
        static List<FileInfo> files = new List<FileInfo>();

        static bool ParseFile(string file, FileInfo.Platforms platform)
        {
            FileInfo.platform = platform;

            FileInfo curFile = null;
            string myPath = "";
            bool spu = false;

            System.IO.StreamReader tr = new System.IO.StreamReader(file);

            while (!tr.EndOfStream)
            {
                string line = tr.ReadLine();

                if (line.StartsWith("lib: "))
                {
                    string[] t = line.Substring(4).Split(':');

                    string name = t[0].Trim();
                    string path = t[2].Trim();

                    if (libs.Find(l => l.Path.ToLower() == path.ToLower()) == null)
                    {
                        libs.Add(new Library() { Name = name, Path = path });
                    }
                }
                else if (line.StartsWith("path: "))
                {
                    myPath = line.Substring(6).Trim();
                    spu = false;
                }
                else if (line.StartsWith("Duration: "))
                {
                    if (myPath.Length > 0)
                    {
                        if (myPath.ToLower().EndsWith("stdafx.h"))
                        {
                            myPath = myPath.Substring(0, myPath.Length - 1) + "cpp";
                        }

                        Library fileLibrary = null;

                        // Try and match libary
                        foreach (Library l in libs)
                        {
                            if (myPath.ToLower().StartsWith(l.Path.ToLower()) || (fileLibrary == null && l.Path == "."))
                            {
                                if ((fileLibrary == null) || (l.Path.Length > fileLibrary.Path.Length))
                                    fileLibrary = l;
                            }
                        }

                        string pathWithoutLib = (fileLibrary != null) ? myPath.Substring(fileLibrary.Path.Length) : myPath;

                        if (pathWithoutLib.ToLower().StartsWith(@"\obj\"))
                        {
                            pathWithoutLib  = pathWithoutLib.Substring(pathWithoutLib.IndexOf(@"\.") + 1);
                            pathWithoutLib = pathWithoutLib.Substring(pathWithoutLib.IndexOf(".cpp") + 5);
                        }

                        curFile = files.Find(f => f.Filename.ToLower() == pathWithoutLib.ToLower() && f.SPU == spu);
                        if (curFile == null)
                        {
                            curFile = new FileInfo() { Filename = pathWithoutLib, Library = fileLibrary, SPU = spu };

                            files.Add(curFile);
                        }
                     
                        string duration = line.Substring(10).Trim();

                        if (curFile.Duration == Int32.MinValue)
                        {
                            curFile.Duration = Int32.Parse(duration);
                        }

                        myPath = "";
                    }
                }
                else
                {
                    if (line.ToLower().Contains("(spu)"))
                        spu = true;
               }
            }


            FileInfo.platform = FileInfo.Platforms.Unknown;

            return true;
        }

        static object FormatDuration(Int32 d)
        {
            if (d != Int32.MinValue)
                return Double.Parse(String.Format("{0:0.0}", d / 1000.0));
            else
                return "";
        }

        static void DumpStats(string filename, FileInfo.Platforms platform)
        {
            bool ps3Valid = false;
            bool pcValid = false;
            bool xbox360Valid = false;

            FileInfo.platform = platform;

            // Write out columns

            Workbook wb = new Workbook();

            #region File times
            {
                // Sort all our records by longest first
                var r = from f in files orderby f.Duration descending select f;

                Workbook.Worksheet ws = new Workbook.Worksheet("Files");
                wb.Worksheets.Add(ws);

                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 100 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 400 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 42 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 72 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 72 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 72 });

                // Total them up as we go
                Int32 total360 = 0;
                Int32 totalPS3 = 0;
                Int32 totalPC = 0;

                foreach (var f in r)
                {
                    if (f.PlatformDuration(FileInfo.Platforms.Platform_360) != Int32.MinValue)
                    {
                        var t = f.PlatformDuration(FileInfo.Platforms.Platform_360);

                        total360 += t;
                        f.Library.Duration360 += t;
                        xbox360Valid = true;
                    }

                    if (f.PlatformDuration(FileInfo.Platforms.Platform_PS3) != Int32.MinValue)
                    {
                        var t = f.PlatformDuration(FileInfo.Platforms.Platform_PS3);
                        totalPS3 += t;
                        f.Library.DurationPS3 += t;
                        ps3Valid = true;
                    }

                    if (f.PlatformDuration(FileInfo.Platforms.Platform_PC) != Int32.MinValue)
                    {
                        var t = f.PlatformDuration(FileInfo.Platforms.Platform_PC);
                        totalPC += t;
                        f.Library.DurationPC += t;
                        pcValid = true;
                    }

                    f.Library.FileCount++;
                }

                Workbook.Worksheet.Row headerRow = new Workbook.Worksheet.Row();
                ws.Rows.Add(headerRow);
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "Library" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "File" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "SPU" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "360 (" + FormatDuration(total360).ToString() + ")" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "PS3 (" + FormatDuration(totalPS3).ToString() + ")" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "PC (" + FormatDuration(totalPC).ToString() + ")" });

                foreach (var f in r)
                {
                    Workbook.Worksheet.Row row = new Workbook.Worksheet.Row();
                    ws.Rows.Add(row);

                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = f.Library.Name });

                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = f.Filename });
                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = f.SPU ? "SPU" : "" });

                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = FormatDuration(f.PlatformDuration(FileInfo.Platforms.Platform_360)) });
                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = FormatDuration(f.PlatformDuration(FileInfo.Platforms.Platform_PS3)) });
                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = FormatDuration(f.PlatformDuration(FileInfo.Platforms.Platform_PC)) });
                }
            }
            #endregion

            #region Libary times
            {
                // Sort all our records by longest first
                System.Linq.IOrderedEnumerable<CompileTimeAnalysis.Library> r;
                
                if (ps3Valid)
                    r = from l in libs orderby l.DurationPS3 descending select l;
                else if (xbox360Valid)
                    r = from l in libs orderby l.Duration360 descending select l;
                else
                    r = from l in libs orderby l.DurationPC descending select l;

                Workbook.Worksheet ws = new Workbook.Worksheet("Libs");
                wb.Worksheets.Add(ws);

                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 100 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 57 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 60 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 60 });
                ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 60 });

                Workbook.Worksheet.Row headerRow = new Workbook.Worksheet.Row();
                ws.Rows.Add(headerRow);
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "Library" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "FileCount" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "360" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "PS3" });
                headerRow.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = "PC" });

                foreach (var l in r)
                {
                    Workbook.Worksheet.Row row = new Workbook.Worksheet.Row();
                    ws.Rows.Add(row);

                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = l.Name });
                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = l.FileCount });

                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = FormatDuration(l.Duration360) });
                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = FormatDuration(l.DurationPS3) });
                    row.Cells.Add(new Workbook.Worksheet.Row.Cell() { Value = FormatDuration(l.DurationPC) });
                }
            }
            #endregion

            wb.Save(filename);

            FileInfo.platform = FileInfo.Platforms.Unknown;
        }

        static void Main(string[] args)
        {
            bool ps3Valid = false;
            bool xboxValid = true;
            bool pcValid = false;

            ParseFile(args[0], FileInfo.Platforms.Platform_360);
            if ((args.Length > 1) && System.IO.File.Exists(args[1]))
            {
                ParseFile(args[1], FileInfo.Platforms.Platform_PS3);
                ps3Valid = true;
            }
            if ((args.Length > 2) && System.IO.File.Exists(args[2]))
            {
                pcValid = true;
                ParseFile(args[2], FileInfo.Platforms.Platform_PC);
            }

            // Let's write out an excel spreadsheet now
            if (ps3Valid)
                DumpStats("resultsNew.xml", FileInfo.Platforms.Platform_PS3);
            else
                DumpStats("resultsNew.xml", FileInfo.Platforms.Platform_360);

            System.Console.WriteLine("Done...");
        }
    }
}
