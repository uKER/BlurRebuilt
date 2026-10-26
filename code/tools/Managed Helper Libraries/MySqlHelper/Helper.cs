namespace Bizarre.MySqlHelper
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;

    using MySql.Data;
    using MySql.Data.MySqlClient;

    public enum eDlType
    {
        kDlBool,
        kDlS8,
        kDlU8,
        kDlS32,
        kDlU32,
        kDlF32,
        kDlS64,
        kDlString,

        kTimestamp,
        kAutoTimestamp,         // Automatically stamp the time the record is created
        kKeyReference           // Special value not sent from game to create auto-incrementing key fields
    }

    public class QuotedValueBuilder
    {
        protected string value = string.Empty;
        protected string encloseChar;

        public QuotedValueBuilder(string encloseChar)
        {
            this.encloseChar = encloseChar;
        }

        public string Value
        {
            get
            {
                return this.value;
            }
        }

        public void Add(string v)
        {
            if (this.value.Length > 0)
            {
                this.value += ",";
            }

            this.value += this.encloseChar + v + this.encloseChar;
        }
        
        public void AddWithSpecialPrepend(string v, string prepend)
        {
            if (this.value.Length > 0)
            {
                this.value += ",";
            }

            this.value += prepend + this.encloseChar + v + this.encloseChar;
        }
    }

    public class SqlValuesBuilder
    {
        private QuotedValueBuilder columns = new QuotedValueBuilder("`");
        private QuotedValueBuilder values = new QuotedValueBuilder("'");

        public string Columns
        {
            get
            {
                return this.columns.Value;
            }
        }

        public string Values
        {
            get
            {
                return this.values.Value;
            }
        }

        public void AddColumnValuePair(eDlType type, string column, string value)
        {
            this.columns.Add(column);

            if (type != eDlType.kDlBool)
            {
                value = value.Replace("'", "''");
                value = value.Replace("’", "’’");
            }

            if (type != eDlType.kDlBool)
            {
                this.values.Add(value);
            }
            else
            {
                this.values.AddWithSpecialPrepend(value, "b");
            }
        }
    }

    public class SqlTableDefinition
    {
        public string Name
        {
            get;
            set;
        }

        /// <summary>
        /// Is the table storing data per session, or per sub-session
        /// </summary>
        public bool LinkedToSubSession
        {
            get;
            set;
        }

        public SqlTableDefinition()
        {
            this.LinkedToSubSession = true;
        }

        public List<Column> Columns = new List<Column>();
        public List<Index> Indexes = new List<Index>();
        public List<ForeignKey> ForeignKeys = new List<ForeignKey>();

        public class Column
        {
            public string Name;
            public eDlType Type;
            public bool CanBeNull = true;
        }

        public class Index
        {
            public string Name;
            public List<string> Columns = new List<string>();
        }

        public class ForeignKey
        {
            public string Column;

            public string ForeignTable;
            public string ForeignColumn;
        }

        public static string ConvertToSQLType(eDlType type)
        {
            switch (type)
            {
                case eDlType.kDlBool:
                    return "BIT";

                case eDlType.kDlS8:
                    return "TINYINT";

                case eDlType.kDlU8:
                    return "TINYINT UNSIGNED";

                case eDlType.kDlS32:
                    return "INT";

                case eDlType.kDlU32:
                    return "INT UNSIGNED";

                case eDlType.kDlF32:
                    return "FLOAT";

                case eDlType.kDlString:
                    return "TEXT";

                case eDlType.kTimestamp:
                case eDlType.kAutoTimestamp:
                    return "TIMESTAMP";

                case eDlType.kKeyReference:
                    return "INT UNSIGNED NOT NULL";
            }

            return "TEXT";
        }
    }

    public class BizSqlConnection
    {
        static bool Verbose = false;

        MySqlConnection dbConnection = null;

        string connectionString;

        public MySqlConnection SqlConnection
        {
            get
            {
                return dbConnection;
            }
        }

        public static string GetError(int errorCode)
        {
            switch (errorCode)
            {
                case 1042:
                    return "Database not possibly running at expected location.";

                case 1044:
                    return "Access denied.";

                case 1045:
                    return "Problem logging in with username/password.";

                case 1062:
                    return "Duplicate entry for key 'PRIMARY'";
            }

            return "Unknown: " + errorCode;
        }

        public bool Connect(string connectionString)
        {
            this.connectionString = connectionString;

            this.dbConnection = new MySqlConnection(connectionString);

            try
            {
                dbConnection.Open();
                return true;
            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {
                string err = BizSqlConnection.GetError(ex.Number);

                System.Console.WriteLine("Connect ERROR: " + err);

                System.Console.WriteLine("Exception: " + ex.Message);
            }

            return false;
        }

        public void Disconnect()
        {
            if (dbConnection != null)
            {
                dbConnection.Close();
                dbConnection = null;
            }
        }

        public bool CreateDatabase(string name)
        {
            string query = "CREATE DATABASE IF NOT EXISTS `" + name + "`";

            MySqlCommand cmd = new MySqlCommand(query, dbConnection);

            cmd.ExecuteNonQuery();

            return true;
        }

        public bool UseDatabase(string name)
        {
            MySqlCommand cmd = new MySqlCommand("USE `" + name + "`", dbConnection);

            cmd.ExecuteNonQuery();

            return true;
        }

        private bool CreateColumns(SqlTableDefinition td)
        {
            // Check columns exist, create any that don't - check types of those that do
            string queryColumns = "SHOW COLUMNS FROM `" + td.Name + "`";

            MySqlCommand myCommand = new MySqlCommand(queryColumns, dbConnection);

            MySqlDataReader myReader = myCommand.ExecuteReader();

            List<string> existingColumns = new List<string>();

            try
            {
                while (myReader.Read())
                {
                    string columnName = myReader.GetString(0);
                    string columnType = myReader.GetString(1);

                    if (Verbose)
                        System.Diagnostics.Debug.WriteLine(columnName + ": " + columnType);

                    existingColumns.Add(columnName.ToLower());
                }
            }
            finally
            {
                myReader.Close();
            }

            // Add any columns which don't exist
            foreach (var c in td.Columns)
            {
                if (!existingColumns.Contains(c.Name.ToLower()))
                {
                    string columnCreate = "ALTER TABLE `" + td.Name + "` ADD COLUMN `" + c.Name + "` " + SqlTableDefinition.ConvertToSQLType(c.Type);

                    switch (c.Type)
                    {
                        case eDlType.kKeyReference:
                            break;

                        case eDlType.kTimestamp:
                            columnCreate += " DEFAULT NULL NULL";
                            break;

                        case eDlType.kAutoTimestamp:
                            columnCreate += " DEFAULT CURRENT_TIMESTAMP";
                            break;

                        default:
                            if (c.CanBeNull)
                                columnCreate += " DEFAULT NULL";
                            else
                                columnCreate += " NOT NULL";
                            break;
                    }

                    MySqlCommand myAlterCommand = new MySqlCommand(columnCreate, dbConnection);

                    myAlterCommand.ExecuteNonQuery();
                }
            }

            return true;
        }

        private bool CreateIndexes(SqlTableDefinition td)
        {
            // Check columns exist, create any that don't - check types of those that do
            string queryColumns = "SHOW INDEXES FROM `" + td.Name + "`";

            MySqlCommand myCommand = new MySqlCommand(queryColumns, dbConnection);

            MySqlDataReader myReader = myCommand.ExecuteReader();

            List<string> existingIndexes = new List<string>();

            try
            {
                while (myReader.Read())
                {
                    // 0-Table, 1-Unique, 2-KeyName, 3-SeqInIndex, 4-ColumnName, Collation, Cardinality, SubPart, Packed, NULL
                    string keyName = myReader.GetString(2);

                    if (Verbose)
                        System.Diagnostics.Debug.WriteLine(keyName);

                    existingIndexes.Add(keyName.ToLower());
                }
            }
            finally
            {
                myReader.Close();
            }

            // Add any indexes which don't exist
            foreach (var i in td.Indexes)
            {
                if (!existingIndexes.Contains(i.Name.ToLower()))
                {
                    QuotedValueBuilder indexes = new QuotedValueBuilder("`");

                    foreach (var cn in i.Columns)
                    {
                        indexes.Add(cn);
                    }

                    string indexCreate = "ALTER TABLE `" + td.Name + "` ADD INDEX `" + i.Name + "` (" + indexes.Value + ")";

                    try
                    {
                        MySqlCommand myAlterCommand = new MySqlCommand(indexCreate, dbConnection);

                        myAlterCommand.ExecuteNonQuery();
                    }
                    catch (MySqlException ex)
                    {
                        string message = "Exception: CreateIndexes: " + ex.Message;
                        System.Diagnostics.Debug.WriteLine(message);
                        System.Console.WriteLine(message);

                        string cmd = "Exception: CreateIndexes: " + indexCreate;
                        System.Diagnostics.Debug.WriteLine(cmd);
                        System.Console.WriteLine(cmd);
                    }
                }
            }

            return true;
        }

        private bool CreateForeignKeys(SqlTableDefinition td)
        {
            // Check columns exist, create any that don't - check types of those that do
            string queryTable = "SHOW CREATE TABLE `" + td.Name + "`";

            MySqlCommand myCommand = new MySqlCommand(queryTable, dbConnection);

            MySqlDataReader myReader = myCommand.ExecuteReader();

            List<SqlTableDefinition.ForeignKey> existingFK = new List<SqlTableDefinition.ForeignKey>();

            try
            {
                while (myReader.Read())
                {
                    // 0-Table, 1-Unique, 2-KeyName, 3-SeqInIndex, 4-ColumnName, Collation, Cardinality, SubPart, Packed, NULL
                    string tableInfo = myReader.GetString(1);

                    string sk = ".*CONSTRAINT.*`([^`]+)`.+FOREIGN KEY \\(`([^`]+)`\\) REFERENCES `([^`]+)` \\(`([^`\\)]+)`\\)";

                    //                    Regex r = new Regex(".*CONSTRAINT.*`.*`.*FOREIGN KEY (`.*`) REFERENCES `.*` (`.*`)");
                    Regex r = new Regex(sk);
                    //Match mym = Regex.Match(tableInfo, ".*CONSTRAINT.*`.*`.*FOREIGN KEY.*(.*).*REFERENCES");

                    MatchCollection matchCollection = r.Matches(tableInfo);

                    if (matchCollection.Count > 0)
                    {
                        // Groups 0(all) - 1(FK name) - 2(FK column) - 3(FK table) - 4(FK table column(s))
                        for (int i = 0; i < matchCollection.Count; i++)
                        {
                            GroupCollection groupCollection = matchCollection[i].Groups;

                            string fkName = groupCollection[1].Value;
                            string fkColumn = groupCollection[2].Value.ToLower();
                            string fkTable = groupCollection[3].Value.ToLower();
                            string fkTableColumn = groupCollection[4].Value.ToLower();

                            existingFK.Add(new SqlTableDefinition.ForeignKey() { Column = fkColumn, ForeignTable = fkTable, ForeignColumn = fkTableColumn });
                        }
                    }
                }
            }
            finally
            {
                myReader.Close();
            }

            // Add any foreign keys which don't exist
            foreach (var fk in td.ForeignKeys)
            {
                bool found = false;

                foreach (var efk in existingFK)
                {
                    if ((efk.Column == fk.Column.ToLower()) &&
                        (efk.ForeignTable == fk.ForeignTable.ToLower()) &&
                        (efk.ForeignColumn == fk.ForeignColumn.ToLower()))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    // Create FK
                    string fkCreate = "ALTER TABLE `" + td.Name + "` ADD CONSTRAINT FOREIGN KEY (`" + fk.Column + "`) REFERENCES `" + fk.ForeignTable + "` (`" + fk.ForeignColumn + "`) ON DELETE CASCADE";

                    try
                    {
                        MySqlCommand myAlterCommand = new MySqlCommand(fkCreate, dbConnection);

                        myAlterCommand.ExecuteNonQuery();
                    }
                    catch (MySqlException ex)
                    {
                        string message = "Exception: CreateForeignKeys: " + ex.Message;
                        System.Diagnostics.Debug.WriteLine(message);
                        System.Console.WriteLine(message);

                        string cmd = "Exception: CreateForeignKeys: " + fkCreate;
                        System.Diagnostics.Debug.WriteLine(cmd);
                        System.Console.WriteLine(cmd);
                    }
                }
            }

            return true;
        }

        public bool CreateTable(SqlTableDefinition td)
        {
            string createQuery = "CREATE TABLE IF NOT EXISTS `" + td.Name + "` ( _id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY )";

            MySqlCommand cmd = new MySqlCommand(createQuery, dbConnection);

            cmd.ExecuteNonQuery();

            CreateColumns(td);

            if (td.Indexes.Count > 0)
            {
                CreateIndexes(td);
            }

            if (td.ForeignKeys.Count > 0)
            {
                CreateForeignKeys(td);
            }

            return true;
        }

        public bool UpdateDataInTable(string table, SqlValuesBuilder ivb)
        {
            // Check columns exist, create any that don't - check types of those that do
            string insertData = "REPLACE INTO `" + table + "` (" + ivb.Columns + ") VALUES (" + ivb.Values + ")";

            // Put into Database
            try
            {
                MySqlCommand myCommand = new MySqlCommand(insertData, dbConnection);

                myCommand.ExecuteNonQuery();
                
                return true;
            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {
                System.Console.WriteLine("InsertDataIntoTable ERROR: " + ex.Message);
                System.Console.WriteLine("Exception: " + ex.Message);
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("Exception: Insert: " + ex.Message);
                System.Console.WriteLine("Command: " + insertData);
            }
            
            return false;
        }

        public long InsertDataIntoTable(string table, SqlValuesBuilder ivb)
        {
            // Check columns exist, create any that don't - check types of those that do
            string insertData = "INSERT INTO `" + table + "` (" + ivb.Columns + ") VALUES (" + ivb.Values + ")";

            // Put into Database
            try
            {
                MySqlCommand myCommand = new MySqlCommand(insertData, dbConnection);

                myCommand.ExecuteNonQuery();

                return myCommand.LastInsertedId;
            }
            catch (MySql.Data.MySqlClient.MySqlException ex)
            {
                System.Console.WriteLine("InsertDataIntoTable: Exception: " + ex.Message);
                System.Console.WriteLine("InsertDataIntoTable: Command: " + insertData);
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("InsertDataIntoTable: Exception2: Insert: " + ex.Message);
                System.Console.WriteLine("InsertDataIntoTable: Exception2: Command: " + insertData);
            }
            
            return -1;
        }
    }
}
