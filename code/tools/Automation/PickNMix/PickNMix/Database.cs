namespace PickNMix
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Xml;

    using XtInterop;

    class Database
    {
        private string sorceryDatabase;
        private string gameDefinitionFile;

        private XtDb db;

        private Dictionary<uint, Record> recordDictionary;

        private ObservableCollection<Category> data;

        public Database()
        {
        }

        public string SorceryDatabase
        {
            get
            {
                return this.sorceryDatabase;
            }

            set
            {
                if (this.sorceryDatabase != value)
                {
                    this.sorceryDatabase = value;

                    LoadSorceryData();
                }
            }
        }
        
        public string GameDefinitionFile
        {
            get
            {
                return this.gameDefinitionFile;
            }

            set
            {
                if (this.gameDefinitionFile != value)
                {
                    this.gameDefinitionFile = value;

                    LoadGameDef();
                    LoadSorceryData();
                }
            }
        }

        public ObservableCollection<Category> Data
        {
            get
            {
                return this.data;
            }

            set
            {
                this.data = value;
            }
        }

        public Dictionary<string, CategoryHighlight> AllowedCategories
        {
            get;
            set;
        }

        private void LoadGameDef()
        {
            AllowedCategories = new Dictionary<string, CategoryHighlight>();

            if (System.IO.File.Exists(this.gameDefinitionFile))
            {
                XmlDocument doc = new XmlDocument();

                doc.Load(this.gameDefinitionFile);

                XmlNodeList recs = doc.GetElementsByTagName("Type");

                foreach (XmlNode n in recs)
                {
                    CategoryHighlight ch = new CategoryHighlight();

                    string type = n.Attributes["type"].Value;

                    XmlAttribute field = n.Attributes["field"];
                    if (field != null)
                    {
                        ch.Field = field.Value;
                        ch.Value = n.Attributes["value"].Value;
                    }

                    AllowedCategories.Add(type, ch);
                }
            }

            //AllowedCategories.Add("AmaxVehicle", new CategoryHighlight() { Field = "use", Value = "1" });
            //AllowedCategories.Add("Route", new CategoryHighlight());
        }

        private bool LoadSorceryData()
        {
            if (db != null)
            {
                db.Dispose();
            }

            db = new XtDb();

            string dir = System.IO.Path.GetDirectoryName(SorceryDatabase);

            bool ok = db.Open(dir);
            if (ok)
            {
                recordDictionary = new Dictionary<uint, Record>();

                var list = from records in db.Records
                           orderby records.Type
                           where (AllowedCategories.Keys.Count == 0 || AllowedCategories.Keys.Contains(records.Type))
                           group records by records.Type into g
                           select new
                           {
                               Name = g.Key,
                               Records = from r in g
                                         orderby r.Name
                                         select
                                             new
                                             {
                                                 Name = r.Name,
                                                 Use = false,
                                                 Data = r
                                             }
                           };

                data = new ObservableCollection<Database.Category>();

                foreach (var c in list)
                {
                    Database.Category nc = new Database.Category() { Name = c.Name, Records = new ObservableCollection<Database.Record>() };

                    foreach (var r in c.Records)
                    {
                        bool highlight = false;

                        if (AllowedCategories.Keys.Contains(c.Name))
                        {
                            Database.CategoryHighlight ch = AllowedCategories[c.Name];

                            XtBlock xtb = r.Data.Root.Data;

                            if (xtb.Keys.Contains(ch.Field))
                            {
                                XtField xtf = r.Data.Root.Data[ch.Field] as XtField;

                                if (ch.Value == xtf.Value)
                                {
                                    highlight = true;
                                }
                            }
                        }

                        Record newRec = new Record() { Name = r.Name, Highlight = highlight, Data = r.Data, Use = false };

                        recordDictionary[r.Data.Id] = newRec;

                        nc.Records.Add(newRec);
                    }

                    data.Add(nc);
                }

                //foreach (var l in database.Data)
                //{
                //    var l2 = from r in l.Records select r;

                //    System.Diagnostics.Debug.WriteLine("R: " + l.Name.ToString() + " " + l2.Count());
                //}

                return true;
            }
            
            return false;
        }

        public void Load(string filename)
        {
            XmlDocument doc = new XmlDocument();

            doc.Load(filename);

            var sorceryElement = doc.GetElementsByTagName("Sorcery");
            var gamedefElement = doc.GetElementsByTagName("GameDefinition");

            gameDefinitionFile = gamedefElement[0].InnerText;
            LoadGameDef();

            sorceryDatabase = sorceryElement[0].InnerText;
            LoadSorceryData();

            // Tag records in use
            XmlNodeList recs = doc.GetElementsByTagName("Record");

            foreach (XmlNode r in recs)
            {
                uint id = uint.Parse(r.Attributes["id"].Value);

                if (recordDictionary.Keys.Contains(id))
                {
                    recordDictionary[id].Use = true;
                }
            }
        }

        public void Save(string filename)
        {
            XmlTextWriter textWriter = new XmlTextWriter(filename, null);

            textWriter.WriteStartDocument();

            textWriter.WriteStartElement("root");

            textWriter.WriteElementString("Sorcery", sorceryDatabase);
            textWriter.WriteElementString("GameDefinition", gameDefinitionFile);

            foreach (var t in this.Data)
            {
                foreach (var r in t.Records)
                {
                    if (r.Use)
                    {
                        textWriter.WriteStartElement("Record");
                        textWriter.WriteAttributeString("type", t.Name);
                        textWriter.WriteAttributeString("id", r.Data.Id.ToString());
                        textWriter.WriteEndElement();
                    }
                }
            }
            
            textWriter.WriteEndElement();

            textWriter.WriteEndDocument();
            textWriter.Close();
        }

        public class Category
        {
            private string name;

            private ObservableCollection<Record> records;

            public string Name
            {
                get
                {
                    return this.name;
                }

                set
                {
                    this.name = value;
                }
            }

            public ObservableCollection<Record> Records
            {
                get
                {
                    return this.records;
                }

                set
                {
                    this.records = value;
                }
            }
        }

        public class Record : INotifyPropertyChanged
        {
            private string name;
            private bool use;
            private bool highlight;

            #region INotifyPropertyChanged Members

            public event PropertyChangedEventHandler PropertyChanged;

            #endregion

            private void NotifyPropertyChanged(String info)
            {
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs(info));
                }
            }

            public string Name
            {
                get
                {
                    return this.name;
                }

                set
                {
                    this.name = value;
                    this.NotifyPropertyChanged("Name");
                }
            }

            public bool Use
            {
                get
                {
                    return this.use;
                }

                set
                {
                    this.use = value;
                    this.NotifyPropertyChanged("Use");
                }
            }

            public bool Highlight
            {
                get
                {
                    return this.highlight;
                }

                set
                {
                    this.highlight = value;
                }
            }

            public XtRecord Data;
        }

        public class CategoryHighlight
        {
            public string Field = string.Empty;
            public string Value = string.Empty;
        }
    }
}
