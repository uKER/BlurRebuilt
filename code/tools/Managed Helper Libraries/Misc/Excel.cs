namespace Bizarre.Misc.ExcelXML
{
    using System;
    using System.Collections.Generic;
    //using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;

    using Excel = Microsoft.Office.Interop.Excel;

    public class Workbook
    {
        public List<CellStyle> Styles { get; set; }
        public List<Worksheet> Worksheets { get; set; }

        public Workbook()
        {
        //internal Excel.Application xlApp;
        //internal Excel.Workbook xlWorkBook;
        //    this.xlApp = new Excel.ApplicationClass();
        //    this.xlWorkBook = xlApp.Workbooks.Add(Missing.Value);

        //    ((Excel.Worksheet)this.xlWorkBook.Worksheets.get_Item(2)).Delete();
        //    ((Excel.Worksheet)this.xlWorkBook.Worksheets.get_Item(2)).Delete();


            //    //add data 
            //    xlWorkSheet.Cells[4, 2] = "";
            //    xlWorkSheet.Cells[4, 3] = "Student1";
            //    xlWorkSheet.Cells[4, 4] = "Student2";
            //    xlWorkSheet.Cells[4, 5] = "Student3";

            //    xlWorkSheet.Cells[5, 2] = "Term1";
            //    xlWorkSheet.Cells[5, 3] = "80";
            //    xlWorkSheet.Cells[5, 4] = "65";
            //    xlWorkSheet.Cells[5, 5] = "45";

            //    xlWorkSheet.Cells[6, 2] = "Term2";
            //    xlWorkSheet.Cells[6, 3] = "78";
            //    xlWorkSheet.Cells[6, 4] = "72";
            //    xlWorkSheet.Cells[6, 5] = "60";

            //    xlWorkSheet.Cells[7, 2] = "Term3";
            //    xlWorkSheet.Cells[7, 3] = "82";
            //    xlWorkSheet.Cells[7, 4] = "80";
            //    xlWorkSheet.Cells[7, 5] = "65";

            //    xlWorkSheet.Cells[8, 2] = "Term4";
            //    xlWorkSheet.Cells[8, 3] = "75";
            //    xlWorkSheet.Cells[8, 4] = "82";
            //    xlWorkSheet.Cells[8, 5] = "68";

            //    xlWorkSheet.Cells[9, 2] = "Total";
            //    xlWorkSheet.Cells[9, 3] = "315";
            //    xlWorkSheet.Cells[9, 4] = "299";
            //    xlWorkSheet.Cells[9, 5] = "238";

            //    xlWorkSheet.get_Range("b2", "e3").Merge(false);

            //    Excel.Range chartRange = xlWorkSheet.get_Range("b2", "e3");
            //    chartRange.FormulaR1C1 = "MARK LIST";
            //    chartRange.HorizontalAlignment = 3;
            //    chartRange.VerticalAlignment = 3;

            //    chartRange = xlWorkSheet.get_Range("b4", "e4");
            //    chartRange.Font.Bold = true;
            //    chartRange = xlWorkSheet.get_Range("b9", "e9");
            //    chartRange.Font.Bold = true;

            //    chartRange = xlWorkSheet.get_Range("b2", "e9");
            //    chartRange.BorderAround(Excel.XlLineStyle.xlContinuous, Excel.XlBorderWeight.xlMedium, Excel.XlColorIndex.xlColorIndexAutomatic, Excel.XlColorIndex.xlColorIndexAutomatic);

            //    xlWorkBook.SaveAs("e:\\roger.xls", Excel.XlFileFormat.xlWorkbookNormal, Missing.Value, Missing.Value, Missing.Value, Missing.Value, Excel.XlSaveAsAccessMode.xlExclusive, Missing.Value, Missing.Value, Missing.Value, Missing.Value, Missing.Value);
            //    xlWorkBook.Close(true, Missing.Value, Missing.Value);
            //    xlApp.Quit();
            //}

            this.Styles = new List<CellStyle>();
            this.Worksheets = new List<Worksheet>();

            Workbook.CellStyle.BorderInfo thinBorder = new Workbook.CellStyle.BorderInfo() { LineStyle = Workbook.Style.Continuous, Weight = 1 };

            this.Styles.Add(new Workbook.CellStyle("StylePass", 0x99cc00, thinBorder));
            this.Styles.Add(new Workbook.CellStyle("StyleFail", 0xFF0000, thinBorder));
            this.Styles.Add(new Workbook.CellStyle("StyleSkipped", 0xd8bfd8, thinBorder));
        }

        public CellStyle FindStyle(string name)
        {
            foreach (var s in this.Styles)
            {
                if (s.Name == name)
                    return s;
            }

            return null;
        }

        public class CellStyle
        {
            private static uint uniqueCellIndex = 1;

            private string name;
            private uint cellIndex;
            private ePattern pattern;

            private eAlignment horizontalAlignment = eAlignment.Left;
            private eAlignment verticalAlignment = eAlignment.Bottom;

            private bool wrapText = false;

            public eAlignment HorizontalAlignment
            {
                get
                {
                    return this.horizontalAlignment;
                }

                set
                {
                    this.horizontalAlignment = value;
                }
            }

            public eAlignment VerticalAlignment
            {
                get
                {
                    return this.verticalAlignment;
                }

                set
                {
                    this.verticalAlignment = value;
                }
            }

            public bool WrapText
            {
                get
                {
                    return this.wrapText;
                }

                set
                {
                    this.wrapText = value;
                }
            }

            public BorderInfo Border
            {
                set
                {
                    this.Top = value;
                    this.Bottom = value;
                    this.Left = value;
                    this.Right = value;
                }
            }

            public BorderInfo Top
            {
                get;
                set;
            }

            public BorderInfo Bottom
            {
                get;
                set;
            }

            public BorderInfo Left
            {
                get;
                set;
            }

            public BorderInfo Right
            {
                get;
                set;
            }

            public uint Red
            {
                get;
                set;
            }

            public uint Green
            {
                get;
                set;
            }

            public uint Blue
            {
                get;
                set;
            }

            public string Name
            {
                get
                {
                    if (this.name != null)
                    {
                        return "ss:" + name;
                    }
                    else
                    {
                        return "ss:" + cellIndex;
                    }
                }

                set
                {
                    this.name = value;
                }
            }

            public ePattern Pattern
            {
                get
                {
                    return this.pattern;
                }

                set
                {
                    this.pattern = value;
                }
            }

            public CellStyle()
            {
                this.cellIndex = uniqueCellIndex++;

                this.Red = this.Green = this.Blue = 0xff;
                this.pattern = ePattern.Solid;

                this.horizontalAlignment = eAlignment.Left;
                this.verticalAlignment = eAlignment.Bottom;
            }

            public CellStyle(uint colour)
            {
                this.cellIndex = uniqueCellIndex++;

                this.SetColour(colour);
            }

            public CellStyle(uint colour, BorderInfo bi)
            {
                this.cellIndex = uniqueCellIndex++;
                this.Border = bi;

                this.SetColour(colour);
            }

            public CellStyle(string name, uint colour, BorderInfo bi)
            {
                this.name = name;
                this.cellIndex = uniqueCellIndex++;
                this.Border = bi;

                this.SetColour(colour);
            }

            public void SetColour(uint col)
            {
                this.Red = (col & 0xff0000) >> 16;
                this.Green = (col & 0x00ff00) >> 8;
                this.Blue = (col & 0x0000ff) >> 0;
            }

            public void Save(System.IO.StreamWriter sr)
            {
                sr.WriteLine("  <Style ss:ID=\"" + Name + "\">");

                if (this.Top != null || this.Bottom != null || this.Left != null || this.Right != null)
                {
                    sr.WriteLine("   <Borders>");

                    if (this.Top != null)
                    {
                        this.Top.Save("Top", sr);
                    }

                    if (this.Bottom != null)
                    {
                        this.Bottom.Save("Bottom", sr);
                    }

                    if (this.Left != null)
                    {
                        this.Left.Save("Left", sr);
                    }

                    if (this.Right != null)
                    {
                        this.Right.Save("Right", sr);
                    }

                    sr.WriteLine("   </Borders>");
                }

                if (this.horizontalAlignment != eAlignment.Left || this.verticalAlignment != eAlignment.Bottom)
                {
                    sr.Write("   <Alignment");

                    if (this.horizontalAlignment != eAlignment.Left)
                    {
                        sr.Write(" ss:Horizontal=\"" + this.horizontalAlignment + "\"");
                    }

                    if (this.verticalAlignment != eAlignment.Bottom)
                    {
                        sr.Write(" ss:Vertical=\"" + this.verticalAlignment + "\"");
                    }

                    if (this.wrapText)
                    {
                        sr.Write(" ss:WrapText=\"1\"");
                    }

                    sr.WriteLine(" />");
                }

                if (this.Red != 255 || this.Green != 255 || this.Blue != 255 || this.pattern != ePattern.Solid)
                {
                    sr.WriteLine(string.Format("   <Interior ss:Color=\"#{0:X2}{1:X2}{2:X2}\" ss:Pattern=\"{3}\" />", this.Red, this.Green, this.Blue, this.pattern.ToString()));
                }

                sr.WriteLine("  </Style>");
            }

            public class BorderInfo
            {
                public BorderInfo()
                {
                    this.LineStyle = Style.Continuous;
                    this.Weight = 1;
                }

                public Style LineStyle
                {
                    get;
                    set;
                }

                public uint Weight
                {
                    get;
                    set;
                }

                public void Save(string side, System.IO.StreamWriter sr)
                {
                    sr.WriteLine(string.Format("    <Border ss:Position=\"{0}\" ss:LineStyle=\"{1}\" ss:Weight=\"{2}\" />", side, this.LineStyle.ToString(), Weight));
                }
            }
        }

        // <Borders>
        //  <Border ss:Position="Top" ss:LineStyle="Continuous" ss:Weight="1" />
        //  <Border ss:Position="Bottom" ss:LineStyle="Continuous" ss:Weight="1" />
        //  <Border ss:Position="Left" ss:LineStyle="Continuous" ss:Weight="1" />
        //  <Border ss:Position="Right" ss:LineStyle="Continuous" ss:Weight="1" />
        // </Borders>
        // <Interior ss:Color="#FFFFFF" ss:Pattern="Solid"/>
        //</Style>

        public class Worksheet
        {
            public bool AutoFilter = false;

            public uint FreezeColumns = 0;
            public uint FreezeRows = 0;

            private Workbook workbook;

            private static uint index = 1;

            public Worksheet(Workbook workbook)
            {
                this.workbook = workbook;
                Columns = new List<Column>();
                Rows = new List<Row>();

                this.Name = "Sheet " + index++;
            }

            public Worksheet(Workbook workbook, string name)
            {
                this.workbook = workbook;
                Columns = new List<Column>();
                Rows = new List<Row>();

                this.Name = name;
                index++;
            }

            public Workbook Workbook
            {
                get
                {
                    return this.workbook;
                }
            }

            public class Column
            {
                public double Width { get; set; }

                public void Save(System.IO.StreamWriter sr)
                {
                    sr.WriteLine("   <Column ss:Width=\"" + Width.ToString() + "\" />");
                }
            }

            public Row AddRow()
            {
                Row r = new Row();

                Rows.Add(r);

                return r;
            }

            public class Row
            {
                public Int32 Index { get; set; }

                public List<Cell> Cells { get; set; }

                public Row()
                {
                    Cells = new List<Cell>();
                }

                public Cell AddCell()
                {
                    Cell c = new Cell();

                    Cells.Add(c);

                    return c;
                }

                public Cell AddCell(object o)
                {
                    Cell c = new Cell(o);

                    Cells.Add(c);

                    return c;
                }

                public Cell AddCell(CellStyle cs)
                {
                    Cell c = new Cell(cs);

                    Cells.Add(c);

                    return c;
                }

                public Cell AddCell(CellStyle cs, object o)
                {
                    Cell c = new Cell(cs, o);

                    Cells.Add(c);

                    return c;
                }

                public class Cell
                {
                    public uint Index { get; set; }
                    public uint MergeDown { get; set; }
                    public uint MergeAcross { get; set; }

                    //public static int Index;

                    // ss:Formula="=100*RC[-3]/SUM(R2C4:R30C4)

                    public Cell(CellStyle cs)
                    {
                        this.Style = cs;
                    }

                    public Cell(object o)
                    {
                        this.Value = o;
                    }

                    public Cell(CellStyle cs, object o)
                    {
                        this.Style = cs;
                        this.Value = o;
                    }

                    public Cell()
                    {
                    }

                    public CellStyle Style
                    {
                        get;
                        set;
                    }

                    public object Value
                    {
                        get;
                        set;
                    }

                    public string Formula
                    {
                        get;
                        set;
                    }

                    public string Comment
                    {
                        get;
                        set;
                    }

                    /// <summary>
                    /// Make strings safe for XML by replacing '<' and '>' with the correct XML equivalents
                    /// </summary>
                    /// <param name="value">Original string to make safe</param>
                    /// <returns>A string which is XML friendly</returns>
                    static string MakeValueSafeForXML(string value)
                    {
                        return value.Replace("<", "&lt;").Replace(">", "&gt;");
                    }

                    public void Save(System.IO.StreamWriter sr) // , uint colIndex, uint rowIndex) // , Excel.Worksheet xlWorkSheet)
                    {
                        //<Comment
                        //      ss:Author="Perkins, Roger"><ss:Data
                        //       xmlns="http://www.w3.org/TR/REC-html40"><B><Font html:Face="Tahoma"
                        //         x:CharSet="1" html:Size="9" html:Color="#000000">Perkins, Roger:</Font></B><Font
                        //        html:Face="Tahoma" x:CharSet="1" html:Size="9" html:Color="#000000">&#10;Assert - Help Me!&#10;Epic Fail</Font></ss:Data></Comment>

                        //<Cell><ss:Data ss:Type="String" xmlns="http://www.w3.org/TR/REC-html40"><Font
                        //   html:Color="#FF0000">No collision mesh</Font>, No dynamic object mesh</ss:Data></Cell>

                        //                        sr.WriteLine("    <Cell ss:StyleID=\"ss3\"><Data ss:Type=\"String\">USA_LA_Downtown</Data></Cell>");
                        string type = "";
                        string val = "";

                        //Excel.Range cellRange1 = (Excel.Range)xlWorkSheet.Cells[rowIndex, colIndex];
                        //Excel.Range wksRange = xlWorkSheet.get_Range(cellRange1, cellRange1);

                        if (Value != null)
                        {
                            //wksRange.set_Value(Missing.Value, Value);
                            //xlWorkSheet.Cells[rowIndex, colIndex] = Value;

                            //if ((Value.GetType() == typeof(Int32)) || (Value.GetType() == typeof(Double)))
                            if (Value.GetType() != typeof(string))
                            {
                                type = "Number";
                                val = Value.ToString();
                            }
                            else // if (Value.GetType() == typeof(string))
                            {
                                type = "String";
                                val = Value as string;
                            }
                        }
                        else
                        {
                            type = "String";
                            val = "";
                        }

                        string output = "    <Cell";

                        if (Index != 0)
                        {
                            output += " ss:Index=\"" + Index + "\"";
                        }

                        if (MergeAcross != 0)
                        {
                            output += " ss:MergeAcross=\"" + MergeAcross + "\"";
                        }

                        if (MergeDown != 0)
                        {
                            output += " ss:MergeDown=\"" + MergeDown + "\"";
                        }

                        if (Style != null)
                        {
                            output += " ss:StyleID=\"" + Style.Name + "\"";
                        }

                        output += string.Format("><Data ss:Type=\"{0}\">{1}</Data>", type, MakeValueSafeForXML(val));

                        if (Comment != null && Comment.Length > 0)
                        {
                            string htmlComment = Comment.Replace("\n", "&#10;");

                            output += string.Format("<Comment ss:Author=\"Automated\"><Data>{0}</Data></Comment>", MakeValueSafeForXML(htmlComment));
                        }

                        output += "</Cell>";

                        sr.WriteLine(output);
                    }
                }

                internal uint Save(System.IO.StreamWriter sr) // , uint rowIndex, Excel.Worksheet xlWorkSheet)
                {
                    string param = string.Empty;
                    uint maxCol = 0;

                    if (Index != 0)
                    {
                        param += " ss:Index=\"" + Index + "\"";
                    }

                    sr.WriteLine("   <Row" + param + ">");

                    uint colIndex = 1;

                    foreach (Cell cell in Cells)
                    {
                        if (cell.Index != 0)
                            colIndex = cell.Index;

                        if (colIndex > maxCol)
                            maxCol = colIndex;

                        cell.Save(sr); // , colIndex, rowIndex, xlWorkSheet);
                        colIndex++;
                    }

                    sr.WriteLine("   </Row>");

                    return maxCol;
                }
            }

            public string Name { get; set; }
            public List<Column> Columns { get; set; }
            public List<Row> Rows { get; set; }

            public static string ExcelColumnLetter(int intCol)
            {
                string strColumn;
                char letter1, letter2;
                int intFirstLetter = ((intCol) / 26);
                int intSecondLetter = (intCol % 26);
                intFirstLetter = intFirstLetter + 64;
                intSecondLetter = intSecondLetter + 65;

                if (intFirstLetter > 64)
                {
                    letter1 = (char)intFirstLetter;
                }
                else
                    letter1 = ' ';
                letter2 = (char)intSecondLetter;
                strColumn = string.Concat(letter1, letter2);
                return strColumn.Trim();
            }

            public void Save(System.IO.StreamWriter sr) // , Excel.Worksheet xlWorkSheet)
            {
                sr.WriteLine("  <Worksheet ss:Name=\"" + Name + "\">");
                sr.WriteLine("   <Table>");

                //for (int i = 0; i < 10; i++)
                //{
                //    try
                //    {
                //        if (i > 0)
                //        {
                //            string name = Name + "-" + i;
                //            xlWorkSheet.Name = name;
                //        }
                //        else
                //            xlWorkSheet.Name = Name;
                //        break;
                //    }
                //    catch
                //    {
                //    }
                //}

                foreach (Column col in Columns)
                {
                    col.Save(sr);
                }

                uint rowIndex = 1;
                uint maxColumn = 0;

                foreach (Row row in Rows)
                {
                    //Excel.Range xlRow = (Excel.Range)xlWorkSheet.Rows[row.Index, row.Index];

                    //Excel.Range wksRange = xlWorkSheet.get_Range("A" + row.Index, "A" + row.Index);

                    //wksRange.set_Value(Missing.Value, "MyRow " + row.Index);

                    //Marshal.ReleaseComObject(wksRange);
                    if (row.Index != 0)
                        rowIndex = (uint)row.Index;

                    uint cells = row.Save(sr); // , rowIndex); // , xlWorkSheet);
                    rowIndex++;

                    if (cells > maxColumn)
                        maxColumn = cells;
                }

                sr.WriteLine("   </Table>");

                sr.WriteLine("   <WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">");

                if (this.FreezeColumns != 0 || this.FreezeRows != 0)
                {
                    sr.WriteLine("   <Selected/>");
                    sr.WriteLine("   <FreezePanes/>");
                    sr.WriteLine("   <FrozenNoSplit/>");

                    if (this.FreezeRows != 0)
                    {
                        // Bottom row number of top pane
                        sr.WriteLine("   <SplitHorizontal>" + this.FreezeRows + "</SplitHorizontal>");
                    }
                    
                    if (this.FreezeColumns != 0)
                    {
                        // Bottom row number of top pane
                        sr.WriteLine("   <SplitVertical>" + this.FreezeColumns + "</SplitVertical>");
                    }

                    if (this.FreezeRows != 0)
                    {
                        sr.WriteLine("   <TopRowBottomPane>" + this.FreezeRows + "</TopRowBottomPane>");
                    }
                    if (this.FreezeColumns != 0)
                    {
                        sr.WriteLine("   <LeftColumnRightPane>" + this.FreezeColumns + "</LeftColumnRightPane>");
                    }
                }

                sr.WriteLine("   </WorksheetOptions>");

                if (this.AutoFilter)
                {
                    sr.WriteLine("   <AutoFilter x:Range=\"C1:C" + maxColumn + "\" xmlns=\"urn:schemas-microsoft-com:office:excel\">");
                    sr.WriteLine("   </AutoFilter>");
                }

                sr.WriteLine("  </Worksheet>");
            }
        }

        public enum Style
        {
            Continuous,
            Dash,
        }

        public enum eAlignment
        {
            Center,
            Left,
            Right,
            Top,
            Bottom,
        }

        public enum ePattern
        {
            Solid,
            Gray75,
            Gray50,
            Gray25,
            Gray125,
            Gray0625,
            ThinHorzCross,
        }

        /// <summary>
        /// Writes out an Excel spreadsheet - tries to use the binary version if possible
        /// </summary>
        /// <param name="filename">Name of the file we want to create</param>
        /// <returns>The name of the file written out</returns>
        public string Save(string filename)
        {
            string tempXML = System.IO.Path.GetTempFileName();

            string folder = System.IO.Path.GetDirectoryName(filename);

            if (!System.IO.Directory.Exists(folder))
            {
                System.IO.Directory.CreateDirectory(folder);
            }

            System.IO.StreamWriter sr = new System.IO.StreamWriter(tempXML);

            sr.WriteLine("<?xml version=\"1.0\"?>");
            sr.WriteLine("<?mso-application progid=\"Excel.Sheet\"?>");
            sr.WriteLine("<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"");
            sr.WriteLine(" xmlns:o=\"urn:schemas-microsoft-com:office:office\"");
            sr.WriteLine(" xmlns:x=\"urn:schemas-microsoft-com:office:excel\"");
            sr.WriteLine(" xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"");
            sr.WriteLine(" xmlns:html=\"http://www.w3.org/TR/REC-html40\">");

            if (Styles.Count > 0)
            {
                sr.WriteLine(" <Styles>");

                foreach (CellStyle cs in Styles)
                {
                    cs.Save(sr);
                }

                sr.WriteLine(" </Styles>");
            }

            if (Worksheets.Count > 0)
            {
                // Remove existing worksheets
                //((Excel.Worksheet)xlWorkBook.Worksheets.get_Item(2)).Delete();
                //((Excel.Worksheet)xlWorkBook.Worksheets.get_Item(2)).Delete();

                int wscount = 1;
                foreach (Worksheet ws in Worksheets)
                {
                    //if (wscount > xlWorkBook.Worksheets.Count)
                    //{
                    //    xlWorkBook.Worksheets.Add(Type.Missing, xlWorkBook.Worksheets.get_Item(wscount-1), Type.Missing, Type.Missing);
                    //}

                    //Excel.Worksheet xlWorkSheet = (Excel.Worksheet)xlWorkBook.Worksheets.get_Item(wscount);

                    //xlWorkSheet.EnableCalculation = false;

                    ws.Save(sr); // , xlWorkSheet);

                    //xlWorkSheet.EnableCalculation = true;

                    //Marshal.ReleaseComObject(xlWorkSheet);

                    wscount++;
                }
            }

            sr.WriteLine("</Workbook>");

            //xlWorkBook.SaveAs(filename + ".xlsx", Type.Missing, Type.Missing, Type.Missing, Type.Missing, Type.Missing, Microsoft.Office.Interop.Excel.XlSaveAsAccessMode.xlShared, Type.Missing, Type.Missing, Type.Missing, Type.Missing, Type.Missing);

            //xlWorkBook.Close(true, Missing.Value, Missing.Value);
            //xlApp.Quit();

            //Marshal.ReleaseComObject(xlWorkBook);
            //Marshal.ReleaseComObject(xlApp);

            //GC.Collect();

            sr.Close();

            if (System.IO.Path.GetExtension(filename).ToLower() != "xml")
            {
                // Convert XML to XLSX
                try
                {
                    Excel.Application xlApp = new Excel.ApplicationClass();

                    xlApp.Visible = false;
                    xlApp.DisplayAlerts = false;

                    Excel.Workbook xlWorkBook = xlApp.Workbooks.Open(tempXML, 0, true, 5, "", "", false, Excel.XlPlatform.xlWindows, "", false, false, 0, false, false, false);

                    string tempFilename = System.IO.Path.GetTempFileName();

                    string xlsxFilename = filename + ".xlsb";

                    xlWorkBook.SaveAs(tempFilename, Excel.XlFileFormat.xlExcel12, Type.Missing, Type.Missing, Type.Missing, Type.Missing, Microsoft.Office.Interop.Excel.XlSaveAsAccessMode.xlNoChange, Excel.XlSaveConflictResolution.xlLocalSessionChanges, false, Type.Missing, Type.Missing, Type.Missing);

                    xlWorkBook.Close(true, Missing.Value, Missing.Value);
                    xlApp.Quit();

                    if (System.IO.File.Exists(xlsxFilename))
                    {
                        System.IO.File.Delete(xlsxFilename);
                    }
                    System.IO.File.Move(tempFilename, xlsxFilename);

                    System.IO.File.Delete(tempXML);

                    return xlsxFilename;
                }
                catch
                {
                    // Problem occured in loading XML and converting to XLSB
                    filename += ".xml";
                }
            }

            if (System.IO.File.Exists(filename))
            {
                System.IO.File.Delete(filename);
            }
            System.IO.File.Move(tempXML, filename);

            return filename;
        }
    }
}
