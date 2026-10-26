namespace AutomationHelper
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using XtInterop;

    public class XtHelper
    {
        private XtDb db = null;

        public XtDb Db
        {
            get
            {
                return this.db;
            }
        }

        public bool OpenFrom(XtDb db)
        {
            this.db = db;

            return true;
        }

        public bool Open(string path)
        {
            XtDb d = new XtDb();

            bool ok = d.Open(path);
            if (ok)
            {
                this.db = d;
            }
            else
            {
                System.Console.WriteLine("XtDb.Open failed '" + d.GetErrorString() + "'");
                System.Diagnostics.Debug.WriteLine("XtDb.Open failed '" + d.GetErrorString() + "'");
            }

            return ok;
        }

        public List<XtRecord> FindRecordsByType(string type)
        {
            List<XtRecord> alist = new List<XtRecord>();

            if (this.db == null)
                return alist;

            var list = (from records in this.db.Records where (records.Type.CompareTo(type) == 0) select records);

            foreach (var l in list)
            {
                alist.Add(l);
            }

            return alist;
        }

        public List<XtRecord> FindRecordsByTypeAndPack(string type, string pack)
        {
            List<XtRecord> alist = new List<XtRecord>();

            if (this.db == null)
                return alist;

            var list = (from records in this.db.Records where (records.Type.CompareTo(type) == 0 && records.PackName == pack) select records);

            foreach (var l in list)
            {
                alist.Add(l);
            }

            return alist;
        }
    }
}
