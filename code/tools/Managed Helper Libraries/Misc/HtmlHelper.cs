namespace Bizarre.Misc
{
    using System;
    //using System.Collections.Generic;
    //using System.Linq;
    using System.Text;

    public class HtmlHelper
    {
        private string body = string.Empty;

        private bool inTable = false;
        private bool inTableRow = false;

        private string textAlign = "left";

        public string Body
        {
            get
            {
                System.Diagnostics.Debug.Assert(this.inTable == false, "Check we're not in the middle of a table");
                return this.body;
            }
        }

        public string TextAlignment
        {
            get
            {
                return this.textAlign;
            }

            set
            {
                this.textAlign = value;
            }
        }

        public static string ColourText(string text, string colour)
        {
            return "<font color=\"" + colour + "\">" + text + "</font>";
        }

        public static string BoldText(string text)
        {
            return "<b>" + text + "</b>";
        }

        public void BeginTable(int border)
        {
            this.body += "<table Border=\"" + border + "\">";
            this.inTable = true;
        }

        public void BeginTable()
        {
            this.BeginTable(1);
        }

        public void EndTable()
        {
            System.Diagnostics.Debug.Assert(this.inTable == true, "Check we've started a table");

            this.body += "</table>";
            this.inTable = false;
        }

        public void BeginTableRow()
        {
            System.Diagnostics.Debug.Assert(this.inTable == true, "Check we've started a table");
            System.Diagnostics.Debug.Assert(this.inTableRow == false, "Check we've not started a table row");

            this.body += "<tr>";
            this.inTableRow = true;
        }
        
        public void EndTableRow()
        {
            System.Diagnostics.Debug.Assert(this.inTableRow == true, "Check we started a table row");
            
            this.body += "</tr>\n";
            this.inTableRow = false;
            this.textAlign = "left";
        }

        public void AddTableHeader(string header, int columnSpan)
        {
            System.Diagnostics.Debug.Assert(this.inTableRow == true, "Check we started a table row");

            if (columnSpan > 1)
            {
                this.body += "<th colspan=" + columnSpan + ">" + header + "</th>";
            }
            else
            {
                this.body += "<th>" + header + "</th>";
            }
        }

        public void AddTableHeader(string header)
        {
            AddTableHeader(header, 1);
        }

        public void AddTableData(string data)
        {
            System.Diagnostics.Debug.Assert(this.inTableRow == true, "Check we started a table row");

            this.body += "<td align=\"" + this.textAlign + "\">" + data + "</td>";
        }
    }
}
