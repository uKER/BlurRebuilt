namespace NightwingAutomationHelper
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Automation;
    using AutomationHelper;

    using XtInterop;

    public class NightwingXtRecord
    {
        public NightwingXtRecord(XtRecord rec, string nameField)
        {
            this.Name = ((XtField)rec.Root.Data[nameField]).Value;
            this.Id = rec.Id;

            //if (rec.Root.Data.Keys.Contains("AutomatedTesting"))
            //{
            //    this.AutomatedTesting = ((XtField)rec.Root.Data["AutomatedTesting"]).Value == "1";
            //}
            //else
            //{
            //    this.AutomatedTesting = false;
            //}

            this.Record = rec;
        }

        public string Name { get; set; }

        public uint Id { get; set; }

        //public bool AutomatedTesting { get; set; }
        
        public XtRecord Record { get; set; }
    }

    public class NightwingLevel : NightwingXtRecord
    {
        public NightwingLevel(XtRecord rec)
            : base(rec, "LevelName")
        {
        }
    }

    public class NightwingXtHelper : XtHelper
    {
        private List<NightwingLevel> singlePlayerLevels = new List<NightwingLevel>();

        private List<NightwingLevel> levels = new List<NightwingLevel>();

        public List<NightwingLevel> Levels
        {
            get
            {
                return this.levels;
            }
        }

        public List<NightwingLevel> SinglePlayerLevels
        {
            get
            {
                return this.singlePlayerLevels;
            }
        }

        public bool IsLoaded
        {
            get
            {
                return this.Db != null;
            }
        }

        public NightwingXtHelper()
        {
            if (WorkManager.XtHelper != null)
            {
                base.OpenFrom(WorkManager.XtHelper.Db);

                InitData();
            }
        }

        protected bool InitData()
        {
            List<XtRecord> levelList = FindRecordsByType("LevelDef");
            List<XtRecord> themeList = FindRecordsByType("ThemeDef");

            foreach (var l in levelList)
            {
                this.levels.Add(new NightwingLevel(l));
            }

            foreach (var t in themeList)
            {
                XtField name = t.Root.Data["ThemeName"] as XtField;
                XtArray levels = t.Root.Data["LevelDefs"] as XtArray;

                switch (name.Value)
                {
                    case "Singleplayer":
                        {
                            for (int i=0; i < levels.Count; i++)
                            {
                                XtField xtf = levels[i] as XtField;

                                uint levelId = Convert.ToUInt32(xtf.Value, 16);

                                this.singlePlayerLevels.Add(this.Level(levelId));
                            }

                            break;
                        }
                }
            }

            return true;
        }

        public bool Init(string path)
        {
            bool ok = Open(path);

            if (ok)
            {
                InitData();
            }

            return ok;
        }

        public uint FindIdFromNameAndType(string name, string type)
        {
            var list = from records in Db.Records where (records.Type.CompareTo(type) == 0) select records;

            foreach (var record in list)
            {
                string recordName = ((XtField)record.Root.Data["Name"]).Value;

                if (recordName == name)
                {
                    return record.Id;
                }
            }

            return UInt32.MaxValue;
        }

        public NightwingLevel Level(uint levelId)
        {
            foreach (var l in this.levels)
            {
                if (l.Id == levelId)
                {
                    return l;
                }
            }

            return null;
        }
    }
}
