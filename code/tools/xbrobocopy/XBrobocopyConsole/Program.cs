
using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Text;
using System.IO;

using XBrobocopy;

// TODO: Need to add /XO to stop writing over files newer in destination
// TODO: Need to add /NDL to stop listing directories (we do show path on front of file however)
// TODO: Need to add /NFL to stop listing files

namespace SyncConsole
{
    class Program
    {
        static C_XBrobocopy xbRoboCopy = new C_XBrobocopy();

        static XBrobocopy.Statistics m_Statistics = new Statistics();

        public class Options
        {
            public string XBox;
            public XBSubOptions subOptions;
            public bool Mirror;
            public bool ShowDirs;
            public bool ShowFiles;
            public bool ShowJobSummary;
            public bool Move;
            public List<string> FileSpec;
            public List<string> DirSpec;
            public bool NoProgress;
            public bool LogOnly_DoNoWork;

            public bool ExtraDebugInfo;

            public Options()
            {
                XBox = "";
                subOptions = XBSubOptions.NonRecursive;
                Mirror = false;
                ShowFiles = true;
                ShowDirs = true;
                ExtraDebugInfo = false;
                Move = false;
                NoProgress = false;
                ShowJobSummary = true;
                LogOnly_DoNoWork = false;
                FileSpec = new List<string>();
                DirSpec = new List<string>();
            }
        }
        static string m_Version = "V2.00";

        static Options      m_Options = new Options();

        static string m_PathSrc, m_PathDst;

        static System.Threading.AutoResetEvent m_SrcARE;
        static System.Threading.AutoResetEvent m_DstARE;

        static BizFileInfo m_BfiSrc;
        static BizFileInfo m_BfiDst;
        static BizSyncWorkList m_WorkList;

        //-----------------------------------------------------------
        // Functions
        //-----------------------------------------------------------


        static void UpdateFileStatus(string filename, string directory, ulong filesize, XBStatus status, BizSyncWorkList.BizWorkItem.WorkItemStatus wiStatus)
        {
            // "100%        New File              345533        car_team_colours_resource.bpr"
            if (status == XBStatus.Trying)
            {
                string opText = "    Unknown";

                switch (wiStatus)
                {
                    case BizSyncWorkList.BizWorkItem.WorkItemStatus.NewItem:
                        opText = "    New File";
                        break;
                    case BizSyncWorkList.BizWorkItem.WorkItemStatus.DestOlder:
                        opText = "    Newer";
                        break;
                    case BizSyncWorkList.BizWorkItem.WorkItemStatus.DestNewer:
                        opText = "    Older";
                        break;
                    case BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra:
                        opText = "  *EXTRA File";
                        break;
                }

                if (m_Options.ShowFiles)
                {
                    Console.Write("\t{0,-14}          {1,8}        {2}", opText, C_XBrobocopy.ConvertSizeToString(filesize), (m_Options.ShowDirs) ? filename : directory + @"\" + filename);

                    if (m_Options.NoProgress == true)
                        Console.WriteLine();
                }
            }

            //Console.Write("\r" + status + " " + m_Statistics.CurrentJob + " of " + m_Statistics.TotalJobs + " " + filename + " " + totalSize);

            // Switch on state.
            if (m_Options.NoProgress == false && m_Options.ShowFiles == true)
            {
                if (status == XBStatus.Success)
                {
                    Console.WriteLine("\r100%      "); // "                            ");
                }
                if (status == XBStatus.Failed)
                {
                    Console.WriteLine("\rFAIL      "); // "                            ");
                }
                if (status == XBStatus.Trying)
                {
                    // Display transfere data.
                    //DisplayEstimatedTime();
                }
            }
        }

        static void DisplayEstimatedTime()
        {
            // Calculate data transfere rate;
            double speed = 0.00f;

            speed = ((double)m_Statistics.BytesCopied / ((double)m_Statistics.TotalTime.ElapsedMilliseconds / 1000)) / 1000;

            // Output the speed in the correct context.
            if (speed > 1000)
                Console.Write("   Avg speed: " + String.Format("{0:0.00}", speed / 1000) + "MB/s");
            else
                Console.Write("   Avg speed: " + String.Format("{0:0.00}", speed) + "KB/s");
        }

        static void ScanSrc()
        {
            //xbRoboCopy.SetScanDelegate(OutputSrcScan);
            m_BfiSrc = xbRoboCopy.ScanSrcDir(m_PathSrc, new BizProcessStatus());
            //System.Console.Write("\n");
            m_SrcARE.Set();
        }

        static void ScanDst()
        {
            //xbRoboCopy.SetScanDelegate(OutputDstScan);
            m_BfiDst = xbRoboCopy.ScanDstDir(m_PathDst, new BizProcessStatus());
            //System.Console.Write("\n");
            m_DstARE.Set();
        }

        static void StartScan()
        {
            m_SrcARE = new System.Threading.AutoResetEvent(false);
            System.Threading.Thread myTSrc = new System.Threading.Thread(ScanSrc);
            myTSrc.IsBackground = true;
            myTSrc.Start();

            System.Threading.AutoResetEvent[] event1 = new System.Threading.AutoResetEvent[1];
            event1[0] = m_SrcARE;
            System.Threading.WaitHandle.WaitAll(event1);

            m_DstARE = new System.Threading.AutoResetEvent(false);
            System.Threading.Thread myTDst = new System.Threading.Thread(ScanDst);
            myTDst.IsBackground = true;
            myTDst.Start();

            System.Threading.AutoResetEvent[] event2 = new System.Threading.AutoResetEvent[1];
            event2[0] = m_DstARE;
            System.Threading.WaitHandle.WaitAll(event2);
        }

        static void DrawHelp()
        {
            Console.WriteLine("------------------------------------------------------------");
            Console.WriteLine("    XBrobocopy   :   Xbox file copy utility  :  " + m_Version);
            Console.WriteLine("------------------------------------------------------------\n");
            Console.WriteLine("Command line arguments: source[], destination[], options[]\n\n");
            Console.WriteLine("Options - /X360 name - Console to connect to. Will use default xbox if blank\n");
            Console.WriteLine("          /S    copies subdirectories unless they�re empty.\n");
            Console.WriteLine("          /E    copies subdirectories, even empty ones.\n");
            Console.WriteLine("          /MIR  activates mirror mode, deleting any files in the");
            Console.WriteLine("                destination that aren�t present in the source.\n");
            Console.WriteLine("          /MOVE moves the files, removing them from the source");
            Console.WriteLine("                folder. Removes the source folder as well if empty.\n");
            Console.WriteLine("          /XF filespec - excludes files indicated by filespec.");
            Console.WriteLine("              Wildcards are accepted. Example: /XF *.bak will skip");
            Console.WriteLine("              copying any file with an extension of .bak\n");
            Console.WriteLine("          /XD dirspec - excludes directories indicated by dirspec by name\\path\n");
            Console.WriteLine("          /R:n  retry �n� times before giving up in the event of error");
            Console.WriteLine("                (default is 1 million).\n");
            Console.WriteLine("          /W:n  Wait time between retries.\n");
            Console.WriteLine();
            Console.WriteLine("::");
            Console.WriteLine(":: Logging Options :");
            Console.WriteLine("::");
            Console.WriteLine("                 /L :: List only - don't copy, timestamp or delete any files.");
            Console.WriteLine();
            Console.WriteLine("               /NFL :: No File List - don't log file names.");
            Console.WriteLine("               /NDL :: No Directory List - don't log directory names.");
            Console.WriteLine();
            Console.WriteLine("                /NP :: No Progress - don't display % copied.");
            Console.WriteLine();
            Console.WriteLine("               /NJS :: No Job Summary.");
            Console.WriteLine();
        }

        static bool ReadCommandLine(string[] args)
        {
            if (args.Length < 2)
                return false;

            m_PathSrc = "";
            m_PathDst = "";
            string conCatArgs ="";

            for (int arg = 0; arg < args.Length; arg++)
            {
                conCatArgs += args[arg] + " ";

                if (conCatArgs.Contains("/h "))
                    return false;

                if (conCatArgs.ToLower().Contains("/debug "))
                {
                    conCatArgs = "";
                    m_Options.ExtraDebugInfo = true;
                }

                if (conCatArgs.ToLower().Contains("/s "))
                {
                    conCatArgs = "";
                    m_Options.subOptions = XBSubOptions.IgnoreEmptySub;
                }
                if (conCatArgs.ToLower().Contains("/e "))
                {
                    conCatArgs = "";
                    m_Options.subOptions = XBSubOptions.CopyEmptySub;
                }
                if (conCatArgs.ToLower().Contains("/mir "))
                {
                    conCatArgs = "";
                    m_Options.Mirror = true;
                    m_Options.subOptions = XBSubOptions.CopyEmptySub;
                }
                if (conCatArgs.ToLower().Contains("/move "))
                {
                    conCatArgs = "";
                    m_Options.Move = true;
                }
                if (conCatArgs.ToLower().Contains("/l"))
                {
                    conCatArgs = "";
                    m_Options.LogOnly_DoNoWork = true;
                    m_Options.NoProgress = true;
                }
                if (conCatArgs.ToLower().Contains("/np "))
                {
                    conCatArgs = "";
                    m_Options.NoProgress = true;
                }
                if (conCatArgs.ToLower().Contains("/nfl "))
                {
                    conCatArgs = "";
                    m_Options.ShowFiles = false;
                }
                if (conCatArgs.ToLower().Contains("/ndl "))
                {
                    conCatArgs = "";
                    m_Options.ShowDirs = false;
                }

                if (conCatArgs.ToLower().Contains("/njs "))
                {
                    conCatArgs = "";
                    m_Options.ShowJobSummary = false;
                }

                if (conCatArgs.ToLower().Contains("/x360 "))
                {
                    conCatArgs = "";

                    if (arg == args.Length-1)
                        return false;
                    m_Options.XBox = args[arg + 1];
                }

                if (conCatArgs.ToLower().Contains("/xf "))
                {
                    for (int i = arg + 1; i < args.Length; i++)
                    {
                        conCatArgs += args[i] + " ";
                    }

                    int start = conCatArgs.ToLower().IndexOf("/xf ");
                    string subString = conCatArgs.Substring(start);
                    string filespec = "";

                    int index = subString.ToLower().IndexOf(" /");
                    if (index == -1)
                        index = subString.Length - 1;

                    // Pull out the file names or wildcards.
                    for (int i = 4; i <= index; i++)
                    {
                        if (subString[i] == ' ')
                        {
                            m_Options.FileSpec.Add(filespec);
                            filespec = "";
                        }
                        else if (subString[i] != '*')
                            filespec += subString[i];
                    }
                    conCatArgs = "";
                }

                if (conCatArgs.ToLower().Contains("/xd "))
                {
                    for (int i = arg+1; i < args.Length; i++)
                    {
                        conCatArgs += args[i] + " ";
                    }

                    int start = conCatArgs.ToLower().IndexOf("/xd ");
                    string subString = conCatArgs.Substring(start);
                    string filespec = "";

                    int index = subString.ToLower().IndexOf(" /");
                    if (index == -1)
                    {
                        index = subString.Length - 1;
                    }
                    // Pull out the directory names.
                    for (int i = 4; i <= index; i++)
                    {
                        if (subString[i] == ' ')
                        {
                            m_Options.DirSpec.Add(filespec);
                            filespec = "";
                        }
                        else
                            filespec += subString[i];
                    }
                    conCatArgs = "";
                }

                if (conCatArgs.ToLower().Contains("/r:"))
                {
                    for (int i = arg + 1; i < args.Length; i++)
                    {
                        conCatArgs += args[i] + " ";
                    }

                    int start = conCatArgs.ToLower().IndexOf("/r:");
                    string subString = conCatArgs.Substring(start);
                    int end = subString.ToLower().IndexOf(" ") + 1;
                    string retry = subString.Substring(0, end);
                    conCatArgs = conCatArgs.Remove(start, end);
                    xbRoboCopy.SetRetrys(Convert.ToInt32(retry.Substring(retry.ToLower().IndexOf(":") + 1)));
                }
                if (conCatArgs.ToLower().Contains("/w:"))
                {
                    int start = conCatArgs.ToLower().IndexOf("/w:");
                    string subString = conCatArgs.Substring(start);
                    int end = subString.ToLower().IndexOf(" ") + 1;
                    string wait = subString.Substring(0, end);
                    conCatArgs = conCatArgs.Remove(start, end);
                    xbRoboCopy.SetWaitTime(Convert.ToInt32(wait.Substring(wait.ToLower().IndexOf(":") + 1)));
                }

                // Remaining is either source or dest
                if (conCatArgs.Length > 0 && m_PathSrc.Length <= 0)
                {
                    // We dont want the extra space
                    m_PathSrc = conCatArgs.Substring(0, conCatArgs.Length-1);
                    conCatArgs = "";

                    // Flip Slashes
                    m_PathSrc = m_PathSrc.Replace('/', '\\');

                    // Remove trailing slashes.
                    if (m_PathSrc.EndsWith("\\"))
                        m_PathSrc = m_PathSrc.Remove(m_PathSrc.LastIndexOf("\\"));

                    // Remove trailing " if summit has messed up.
                    if (m_PathSrc.EndsWith("\""))
                        m_PathSrc = m_PathSrc.Remove(m_PathSrc.LastIndexOf("\""));
                }

                if (conCatArgs.Length > 0 && m_PathDst.Length <= 0)
                {
                    m_PathDst = conCatArgs.Substring(0, conCatArgs.Length - 1);
                    conCatArgs = "";

                    m_PathDst = m_PathDst.Replace('/', '\\');

                    if (m_PathDst.EndsWith("\\"))
                        m_PathDst = m_PathDst.Remove(m_PathDst.LastIndexOf("\\"));

                    // Remove trailing " if summit has messed up.
                    if (m_PathDst.EndsWith("\""))
                        m_PathDst = m_PathDst.Remove(m_PathDst.LastIndexOf("\""));
                }
            }
            if (m_PathSrc.Length <= 0 || m_PathDst.Length <= 0 )
                return false;
            return true;
        }


        //-----------------------------------------------------------
        // Main
        //-----------------------------------------------------------
        static int Main(string[] args)
        {
            if (!ReadCommandLine(args))
            {
                DrawHelp();
                return 1;
            }

#if(FILE_COPY_XBCP)
            m_Version += "<xbcp>";
#endif

            System.Console.Write("\n\n");
            System.Console.WriteLine("--------------------------------------------------------");
            System.Console.WriteLine("    XBrobocopy   :   Xbox file copy utility  :  " + m_Version);
            System.Console.WriteLine("--------------------------------------------------------\n\n");


            xbRoboCopy.SetExePath(Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName));
            // Connect to the xbox.
            if (!xbRoboCopy.ConnectToXbox(m_Options.XBox))
            {
                return 1;
            }

            string test = Environment.GetEnvironmentVariable("XEDK");

            // Check that source exists.
            if (!xbRoboCopy.SrcExists(m_PathSrc))
            {
                Console.WriteLine("\n*ERROR* Source does not exist: " + m_PathSrc);
                return 2;
            }
            // Check that destination exists.
            if (!xbRoboCopy.DstExists(m_PathDst))
            {
                // Try and Create the chain of directories.
                if (!xbRoboCopy.CreateDirectory(m_PathDst))
                {
                    Console.WriteLine("\n*ERROR* Cann't create destination: " + m_PathDst);
                    return 2;
                }
            }

            // Get the worklist.
            StartScan();
            m_WorkList = new BizSyncWorkList(Path.GetFileName(m_PathSrc));

            if (m_BfiSrc == null)
            {
                Console.WriteLine("\n*ERROR* Source does not exist!");
                return 2;
            }
            if (m_BfiDst == null)
            {
                Console.WriteLine("\n*ERROR* Destination cannot be created!");
                return 2;
            }

            // Need to fix the paths if they are a root.
            if (m_PathSrc.EndsWith("\\") == true)
                m_PathSrc = m_PathSrc.Replace("\\", "");

            //Console.WriteLine("\nBuilding work list!");
           
            // Now we need to work out what the difference is between the two sets
            SyncType operationToPerform = (!m_Options.Mirror) ? SyncType.CopyIfNewer : SyncType.Sync;

            xbRoboCopy.SetLogOnly(m_Options.LogOnly_DoNoWork);
            xbRoboCopy.SetExtraDebug(m_Options.ExtraDebugInfo);

            xbRoboCopy.CompareFolders(operationToPerform, m_WorkList, m_BfiSrc, m_BfiDst, m_Options.subOptions, m_PathSrc + "\\", "", m_Options.FileSpec, m_Options.DirSpec, ref m_Statistics);
            xbRoboCopy.SetCopyDelegate(UpdateFileStatus);

			if (m_WorkList.FileCount > 0)
            {
                m_Statistics.TotalJobs = (int)m_WorkList.FileCount;

                // Need to fix the paths if they are a root.
                if (m_PathDst.EndsWith("\\") == true)
                    m_PathDst = m_PathDst.Replace("\\", "");
                else
                {
                    // Need to create an intial directory.
                    xbRoboCopy.CreateFolder(m_PathDst);
                }

                int groupIndex = 1;

                Console.WriteLine("\n------------------------------------------------------------------------------\n");

                //if (!m_Options.Move)
                //    Console.WriteLine("\nCopying files...");
                //else
                //    Console.WriteLine("\nMoving files...");
                m_Statistics.TotalTime.Start();

                // Process each job.
                foreach (BizSyncWorkList.Group group in m_WorkList.groups)
                {
                    foreach (BizSyncWorkList.BizWorkItem item in group.items)
                    {
                        m_Statistics.CurrentJob++;
                        string filenameSrc = m_PathSrc + "\\" + item.name;
                        string filenameDst = m_PathDst + "\\" + item.name;

                        // Check item is not on the ignore list.
                        if (!item.ignore)
                        {
                            if (!item.delete)
                            {
                                // If its a file.
                                if (!item.folder)
                                {
                                    bool success;

                                    if (m_Options.Move)
                                    {
                                        if (m_Options.ExtraDebugInfo)
                                            Console.WriteLine("MoveFile:\n\t'{0}' ->\n\t'{1}'\n\t({2})", filenameSrc, filenameDst, item.m_Status);

                                        success = xbRoboCopy.MoveFile(filenameSrc, filenameDst, item.m_Status);
                                    }
                                    else
                                    {
                                        if (m_Options.ExtraDebugInfo)
                                            Console.WriteLine("CopyFile:\n\t'{0}' ->\n\t'{1}'\n\t({2})", filenameSrc, filenameDst, item.m_Status);

                                        success = xbRoboCopy.CopyFile(filenameSrc, filenameDst, item.m_Status);
                                    }

                                    if (success)
                                    {
                                        m_Statistics.FilesCopied++;
                                        m_Statistics.BytesCopied += item.size;
                                    }
                                    else
                                    {
                                        m_Statistics.FilesFailed++;
                                        m_Statistics.BytesFailed += item.size;
                                    }

                                    //Console.WriteLine(" ");
                                }
                                else // Its a folder.
                                {
                                    string srcFullPath = m_PathSrc + "\\" + item.name;
                                    string dstFullPath = m_PathDst + "\\" + item.name;

                                    if (!item.showFolder)
                                    {
                                        if (m_Options.ShowDirs)
                                            Console.WriteLine("\t  New Dir               " + srcFullPath);

                                        if (xbRoboCopy.CreateFolder(dstFullPath))
                                        {
                                            m_Statistics.DirsCopied++;
                                        }
                                        else
                                            m_Statistics.DirsFailed++;
                                    }
                                    else
                                    {
                                        if (m_Options.ShowDirs)
                                            Console.WriteLine("\t\t\t\t" + srcFullPath);
                                    }
                                }
                            }
                            else // Delete the item.
                            {
                                if (!item.folder)
                                {
                                    xbRoboCopy.DeleteFile(filenameDst);
                                    
                                    m_Statistics.FilesExtras++;
                                    m_Statistics.BytesExtras += item.size;
                                }
                                else
                                {
                                    if (!item.fakeDelete)
                                    {
                                        xbRoboCopy.DeleteFolder(filenameDst);
                                        m_Statistics.DirsExtras++;
                                    }
                                    else
                                        Console.WriteLine("\t*EXTRA Dir              " + filenameDst);
                                }
                            }
                        }
                        else // Ignore the item.
                        {
                            // Items are ignored.
                            if (!item.folder)
                            {
                                m_Statistics.FilesSkipped++;
                                Console.Write("Ignoring: " + filenameSrc);
                            }
                            else
                            {
                                m_Statistics.DirsSkipped++;
                                Console.Write("Ignoring: " + filenameSrc);
                            }
                            Console.WriteLine(" ");
                        }
                    }
                    groupIndex++;
                }

                // Clean up the source folders if /MOVE. Needs to be done here since deleting them during
                // the processing on the worklist would delete the files in the folders.
                if (m_Options.Move == true)
                {
                    DirectoryInfo folder = new DirectoryInfo(m_PathSrc);
                    folder.Delete(true);
                }
            }
            else
            {
                //System.Console.WriteLine("\nThere are no files to copy! Data size: " + m_WorkList.size/1000 + "kb");
            }
            //System.Console.WriteLine("\n\nTransfer Finished!");

            if (m_Options.ShowJobSummary)
                OutputResults(m_Statistics);

            return 0;
        }

        static int[] maxColLength = new int[5];
        static string[,] columnValues = new string[5, 5];

        static void SetColumnValue(int col, int row, string value)
        {
            columnValues[col, row] = value;
            
            if (value.Length > maxColLength[col])
                maxColLength[col] = value.Length;
        }

        static void DisplayResultLine(string prefix, int row)
        {
            string txt = "    ";

            if (prefix != "")
                txt += String.Format("{0,5} :", prefix);
            else
                txt += "       ";

            for (int iCol = 0; iCol < 5; iCol++)
            {
                txt += "   " + String.Format("{0," + maxColLength[iCol] + "}",  columnValues[iCol,row]);
            }

            Console.WriteLine(txt); // "     Dirs : {0," + maxColLength[0] + "}   ", columnValues[0, 0]);
        }

        static string ConvertTimeToString(TimeSpan ts)
        {
            return String.Format("{0:00}:{1:00}:{2:00}", ts.Hours, ts.Minutes, ts.Seconds);
        }

        static void OutputResults(XBrobocopy.Statistics m_Statistics)
        {
            // Output the results in a table.
            Console.WriteLine("------------------------------------------------------------------------------\n");

            SetColumnValue(0, 0, "Total");
            SetColumnValue(1, 0, "Copied");
            SetColumnValue(2, 0, "Failed");
            SetColumnValue(3, 0, "Skipped");
            SetColumnValue(4, 0, "Extras");

            SetColumnValue(0, 1, Convert.ToString(m_Statistics.TotalDirs));
            SetColumnValue(0, 2, Convert.ToString(m_Statistics.TotalFiles));
            SetColumnValue(0, 3, C_XBrobocopy.ConvertSizeToString(m_Statistics.TotalBytes));
            //SetColumnValue(0, 3, Convert.ToString(m_Statistics.TotalTimes));

            //dirSuc = Convert.ToString(m_Statistics.TotalFiles);
            //Console.Write("    Files : " + dirSuc + padding.Substring(0, 12 - dirSuc.Length));
            //dirSuc = Convert.ToString(m_Statistics.FilesSuccessfull);
            //Console.Write(dirSuc + padding.Substring(0, 14 - dirSuc.Length));
            //dirSuc = Convert.ToString(m_Statistics.FilesFailed);
            //Console.Write(dirSuc + padding.Substring(0, 10 - dirSuc.Length));
            //dirSuc = Convert.ToString(m_Statistics.FilesIgnored);
            //Console.Write(dirSuc + padding.Substring(0, 10 - dirSuc.Length));
            //dirSuc = Convert.ToString(m_Statistics.FilesDeleted);
            //Console.WriteLine(dirSuc);

            SetColumnValue(1, 1, m_Statistics.DirsCopied.ToString());
            SetColumnValue(2, 1, m_Statistics.DirsFailed.ToString());
            SetColumnValue(3, 1, m_Statistics.DirsSkipped.ToString());
            SetColumnValue(4, 1, m_Statistics.DirsExtras.ToString());

            SetColumnValue(1, 2, m_Statistics.FilesCopied.ToString());
            SetColumnValue(2, 2, m_Statistics.FilesFailed.ToString());
            SetColumnValue(3, 2, m_Statistics.FilesSkipped.ToString());
            SetColumnValue(4, 2, m_Statistics.FilesExtras.ToString());

            SetColumnValue(1, 3, C_XBrobocopy.ConvertSizeToString(m_Statistics.BytesCopied));
            SetColumnValue(2, 3, C_XBrobocopy.ConvertSizeToString(m_Statistics.BytesFailed));
            SetColumnValue(3, 3, C_XBrobocopy.ConvertSizeToString(m_Statistics.BytesSkipped));
            SetColumnValue(4, 3, C_XBrobocopy.ConvertSizeToString(m_Statistics.BytesExtras));

            //SetColumnValue(0, 3, String.Format("{0:0.0}", ConvertSizeToString(((double)m_Statistics.DataSent / GIGABYTE)) + "GB");
            //Console.Write("    Bytes : ");
            //string totalSize = String.Format("{0:0.0}", ((double)m_Statistics.DataSent / GIGABYTE)) + "GB";
            //if (m_Statistics.DataSent < GIGABYTE)
            //    totalSize = String.Format("{0:0.0}", ((double)m_Statistics.DataSent / MEGABYTE)) + "MB";
            //if (m_Statistics.DataSent < MEGABYTE)
            //    totalSize = String.Format("{0:0.0}", ((double)m_Statistics.DataSent / KILOBYTE)) + "KB";
            //if (m_Statistics.DataSent < KILOBYTE)
            //    totalSize = String.Format("{0:0.0}", ((double)m_Statistics.DataSent)) + "B";
            //Console.WriteLine(totalSize + "     ");

            //SetColumnValue(0, 4, String.Format("{0:0.00}", ((float)m_Statistics.TotalTime.ElapsedMilliseconds / 60000)) + " minutes");
            SetColumnValue(0, 4, ConvertTimeToString(m_Statistics.TotalTime.Elapsed));
            
            //Console.Write("    Times : ");
            //string totalTime = String.Format("{0:0.00}", ((float)m_Statistics.TotalTime.ElapsedMilliseconds / 60000)) + " minutes";
            //if ((m_Statistics.TotalTime.ElapsedMilliseconds / 1000) < 60)
            //    totalTime = String.Format("{0:0.0}", ((float)m_Statistics.TotalTime.ElapsedMilliseconds % 60000) / 1000) + " seconds";
            //Console.WriteLine(totalTime);
            //Console.WriteLine("\n");

            DisplayResultLine("", 0);
            DisplayResultLine("Dirs", 1);
            DisplayResultLine("Files", 2);
            DisplayResultLine("Bytes", 3);
            DisplayResultLine("Times", 4);

            //Console.WriteLine("            {0," + maxColLength[0] + "}   ", "Total", "Copied", "Failed", "Ignored", "Extras");
            //Console.WriteLine("     Dirs : {0," + maxColLength[0] + "}   ", columnValues[0,0]);
            //Console.WriteLine("    Files : {0," + maxColLength[0] + "}   ", columnValues[0,1]);
            //Console.WriteLine("    Bytes : {0," + maxColLength[0] + "}   ", columnValues[0,2]);
            //Console.WriteLine("    Times : {0," + maxColLength[0] + "}   ", columnValues[0,3]);
        }
    }
}
