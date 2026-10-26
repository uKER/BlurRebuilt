
using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Collections;
using System.Text;
using System.IO;
using System.Web;

using XDevkit;
using XDevkitWorkaround;

namespace XBrobocopy
{
    // Signiture for the delegate.
    public delegate void DelegateCopyUpdate(string filename, string directory, ulong filesize, XBStatus status, BizSyncWorkList.BizWorkItem.WorkItemStatus wiStatus);

    public enum SyncType
    {
        CopyIfNewer,        // Only copy files if newer in src or don't exist in dest
        CopyAlways,         // Always copy files, even if newer on dest
        Sync,               // Bring the two copies in sync and remove unwanted files from the destination
    }

    public enum XBStatus
    {
        Trying = 0,
        Success,
        Failed,
    }

    public enum XBSubOptions
    {
        NonRecursive = 0,
        CopyEmptySub,
        IgnoreEmptySub
    }

    public enum XBDMErrorInternal : uint
    {
        // Com exceptions from dealing with the XBDM dll.
        NOERR = (UInt32)0x02da0000,
        UNDEFINED = (UInt32)0x82DA0000,
        MAXCONNECT = (UInt32)0x82DA0001,
        CANNOTCONNECT = (UInt32)0x82DA0100,
        CONNECTIONLOST = (UInt32)0x82DA0101,
    }

    public class Statistics
    {
        private ulong bytesCopied = 0;
        private ulong bytesFailed = 0;
        private ulong bytesSkipped = 0;
        private ulong bytesExtras = 0;

        private int filesCopied = 0;
        private int filesFailed = 0;
        private int filesSkipped = 0;
        private int filesExtras = 0;

        private int dirsCopied = 0;
        private int dirsFailed = 0;
        private int dirsSkipped = 0;
        private int dirsExtras = 0;

        private ulong totalBytes = 0;
        private int totalDirs = 0;
        private int totalFiles = 0;
        // private int totalTime = 0;

        public ulong TotalBytes
        {
            get
            {
                return totalBytes;
            }
            set
            {
                totalBytes = value;
            }
        }
        public int TotalFiles
        {
            get
            {
                return totalFiles;
            }
            set
            {
                totalFiles = value;
            }
        }
        public int TotalDirs
        {
            get
            {
                return totalDirs;
            }
            set
            {
                totalDirs = value;
            }
        }

        public ulong BytesCopied
        {
            get
            {
                return bytesCopied;
            }
            set
            {
                TotalBytes = TotalBytes - bytesCopied + value;
                bytesCopied = value;
            }
        }
        public ulong BytesFailed
        {
            get
            {
                return bytesFailed;
            }
            set
            {
                TotalBytes = TotalBytes - bytesFailed + value;
                bytesFailed = value;
            }
        }
        public ulong BytesSkipped
        {
            get
            {
                return bytesSkipped;
            }
            set
            {
                TotalBytes = TotalBytes - bytesSkipped + value;
                bytesSkipped = value;
            }
        }
        public ulong BytesExtras
        {
            get
            {
                return bytesExtras;
            }
            set
            {
                TotalBytes = TotalBytes - bytesExtras + value;
                bytesExtras = value;
            }
        }

        public int FilesCopied
        {
            get
            {
                return filesCopied;
            }
            set
            {
                TotalFiles = TotalFiles + value - filesCopied;
                filesCopied = value;
            }
        }
        public int FilesFailed
        {
            get
            {
                return filesFailed;
            }
            set
            {
                TotalFiles = TotalFiles + value - filesFailed;
                filesFailed = value;
            }
        }
        public int FilesSkipped
        {
            get
            {
                return filesSkipped;
            }
            set
            {
                TotalFiles = TotalFiles + value - filesSkipped;
                filesSkipped = value;
            }
        }
        public int FilesExtras
        {
            get
            {
                return filesExtras;
            }
            set
            {
                filesExtras = value;
            }
        }

        public int DirsCopied
        {
            get
            {
                return dirsCopied;
            }
            set
            {
                TotalDirs = TotalDirs + value - dirsCopied;
                dirsCopied = value;
            }
        }
        public int DirsFailed
        {
            get
            {
                return dirsFailed;
            }
            set
            {
                TotalDirs = TotalDirs + value - dirsFailed;
                dirsFailed = value;
            }
        }
        public int DirsSkipped
        {
            get
            {
                return dirsSkipped;
            }
            set
            {
                TotalDirs = TotalDirs + value - dirsSkipped;
                dirsSkipped = value;
            }
        }
        public int DirsExtras
        {
            get
            {
                return dirsExtras;
            }
            set
            {
                dirsExtras = value;
            }
        }

        public Stopwatch TotalTime = new Stopwatch();
        public int TotalJobs;
        public int CurrentJob;

        public Statistics()
        {
            TotalBytes = 0;

            FilesFailed = 0;
            FilesCopied = 0;
            FilesSkipped = 0;
            FilesExtras = 0;
            
            DirsFailed = 0;
            DirsCopied = 0;
            DirsSkipped = 0;
            DirsExtras = 0;

            TotalJobs = 0;
            
            CurrentJob = 0;

            //TotalTime = new Stopwatch();
        }
    };

    public class C_XBrobocopy
    {
        private static bool verboseDebug = false;

        // Prviate data.
        private XboxManagerClass    m_XboxManager = new XboxManagerClass();
        private XboxConsole         m_XboxConsole;
        
        private DelegateCopyUpdate  m_CopyDelegate;
        private DelegateScanUpdate  m_ScanDelegate;
        
        // Some settings.
        private int     m_ConRetrys = 10;
        private int     m_NumRetrys = 1000000; // 1 million retrys.
        private long    m_WaitTime = 30000;    // Wait time 30 seconds.
        private string  m_XBoxName = "";
        private string  m_ExePath = "";

        private bool m_logOnly = false;
        private bool m_extraDebugOutput = false;

        static ulong GIGABYTE = 1073741824;
        static ulong MEGABYTE = 1048576;
        static ulong KILOBYTE = 1024;

        public static string ConvertSizeToString(ulong size)
        {
            if (size >= GIGABYTE)
            {
                return String.Format("{0:0.00} g", ((double)size / GIGABYTE));
            }
            else if (size >= MEGABYTE)
            {
                return String.Format("{0:0.00} m", ((double)size / MEGABYTE));
            }
            else
            {
                return size.ToString();
            }
        }

        public static void OutputSrcScan(ulong size)
        {
            string totalSize = String.Format("{0:0.0}", ((double)size / GIGABYTE)) + "GB";
            if (size < GIGABYTE)
                totalSize = String.Format("{0:0.0}", ((double)size / MEGABYTE)) + "MB";
            if (size < MEGABYTE)
                totalSize = String.Format("{0:0.0}", ((double)size / KILOBYTE)) + "KB";
            if (size < KILOBYTE)
                totalSize = String.Format("{0:0.0}", ((double)size)) + "B";

            System.Console.Write("\rSize: " + totalSize + "   ");
        }

        public static void OutputDstScan(ulong size)
        {
            string totalSize = String.Format("{0:0.0}", ((double)size / GIGABYTE)) + "GB";
            if (size < GIGABYTE)
                totalSize = String.Format("{0:0.0}", ((double)size / MEGABYTE)) + "MB";
            if (size < MEGABYTE)
                totalSize = String.Format("{0:0.0}", ((double)size / KILOBYTE)) + "KB";
            if (size < KILOBYTE)
                totalSize = String.Format("{0:0.0}", ((double)size)) + "B";

            System.Console.Write("\rSize: " + totalSize + "   ");
        }

        private void CallCopyDelegate(string filename, string directory, ulong filesize, XBStatus status, BizSyncWorkList.BizWorkItem.WorkItemStatus wiStatus)
        {
            if (m_CopyDelegate != null)
            {
                m_CopyDelegate(filename, directory, filesize, status, wiStatus);
            }
        }
        private void CallScanDelegate(ulong size)
        {
            if (m_ScanDelegate != null)
            {
                m_ScanDelegate(size);
            }
        }

        private void TimerWait(int seconds)
        {
            for (int sec = seconds; sec > 0; sec-- )
            {
                Console.Write("\r" + sec + " seconds");
                System.Threading.Thread.Sleep(1000);
            }
            Console.WriteLine("\r0 seconds");
        }

        public void SetExePath(string path)
        {
            m_ExePath = path;
        }

        public void SetRetrys(int retrys)
        {
            m_NumRetrys = retrys;
        }

        public void SetWaitTime(long waitTime)
        {
            m_WaitTime = waitTime;
        }

        public void SetCopyDelegate(DelegateCopyUpdate copyDelegate)
        {
            m_CopyDelegate = copyDelegate;
        }

        public void SetScanDelegate(DelegateScanUpdate scanDelegate)
        {
            m_ScanDelegate = scanDelegate;
        }

        private bool CanResolveException(COMException ex)
        {
            switch ((UInt32)ex.ErrorCode)
            {
                case (UInt32)XBDMErrorInternal.UNDEFINED: return false;
                case (UInt32)XBDMErrorInternal.MAXCONNECT: return true;
                case (UInt32)XBDMErrorInternal.CANNOTCONNECT: return true;
                case (UInt32)XBDMErrorInternal.CONNECTIONLOST: return true;
                
                default:
                    return false;
            }
        }


        public bool SrcExists(string src)
        {
            // Get information about the source file.
            return Directory.Exists(src);
        }

        public bool DstExists(string dst)
        {
            bool result = true;
            
            try
            {
                IXboxFile file = m_XboxConsole.GetFileObject(dst);
            }
            catch
            {
                result = false;
            }
            return result;
        }


        // Connects to the default xbox.
        public bool ConnectToXbox(string consoleName)
        {
            m_XBoxName = consoleName.ToLower();

            for (int retry = 1; retry <= m_ConRetrys; retry++)
            { 
                try
                {
                    if (m_XBoxName == "default" || m_XBoxName == "")
                    {
                        if (verboseDebug)
                            System.Console.WriteLine("Connecting to default XBox console . . .");
                        m_XboxConsole = m_XboxManager.OpenConsole(m_XboxManager.DefaultConsole);
                        m_XboxConsole.FindConsole(1, 1000);
                        if (verboseDebug)
                            System.Console.WriteLine("Connected to DefaultConsole:" + m_XboxManager.DefaultConsole + "\n");
                        m_XBoxName = m_XboxManager.DefaultConsole;
                        return true;
                    }
                    else
                    {
                        if (verboseDebug)
                            System.Console.WriteLine("Connecting to " + m_XBoxName + " XBox console . . .");
                        m_XboxConsole = m_XboxManager.OpenConsole(m_XBoxName);
                        m_XboxConsole.FindConsole(1, 1000);
                        if (verboseDebug)
                            System.Console.WriteLine("Connected to: " + m_XBoxName + "\n");
                        return true;
                    }
                }
                catch
                {
                    System.Console.WriteLine("Connection failed! Retry " + retry + " of " + m_ConRetrys + " in: ");
                    TimerWait(5);
                }
            }
            System.Console.WriteLine("Connection could not be established!");
            return false;
        }


        // Creates a folder on the xbox.
        public bool CreateFolder(string dst)
        {
            try
            {
                m_XboxConsole.FindConsole(1, 1000);
            }
            catch (Exception ex)
            {
                string errorString = m_XboxConsole.XboxManager.TranslateError(((System.Runtime.InteropServices.ExternalException)(ex)).ErrorCode);
                System.Console.WriteLine("Error: " + errorString);
                ConnectToXbox(m_XBoxName);
            }

            try
            {
                IXboxFile file = m_XboxConsole.GetFileObject(dst);   
            }
            catch
            {
                try
                {
                    if (verboseDebug)
                        Console.WriteLine("Creating: " + dst);

                    if (!m_logOnly)
                    {
                        m_XboxConsole.MakeDirectory(dst);
                    }
                }
                catch
                {
                    return false;
                }
            }
            return true;
        }

        // Creates a chain of folders on the xbox.
        public bool CreateDirectory(string dst)
        {
            try
            {
                m_XboxConsole.FindConsole(1, 1000);
            }
            catch (Exception ex)
            {
                string errorString = m_XboxConsole.XboxManager.TranslateError(((System.Runtime.InteropServices.ExternalException)(ex)).ErrorCode);
                System.Console.WriteLine("Error: " + errorString);
                ConnectToXbox(m_XBoxName);
            }

            string[] strings = dst.Split(new char[] { '\\' }, StringSplitOptions.RemoveEmptyEntries);
            string path = strings[0];

            foreach (string strValue in strings)
            {
                if (strValue == "devkit:")
                    continue;

                path += "\\" + strValue;
                try
                {
                    IXboxFile file = m_XboxConsole.GetFileObject(path);
                }
                catch
                {
                    try
                    {
                        Console.WriteLine("Creating: " + path);
                        m_XboxConsole.MakeDirectory(path);
                    }
                    catch
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        public bool TransferFile(string src, string dst, bool deleteSrcFile, BizSyncWorkList.BizWorkItem.WorkItemStatus status)
        {
            // Get information about the source file.
            FileInfo srcFile = new FileInfo(src);
            for (int retry = 1; retry <= m_NumRetrys; retry++)
            {
                try
                {
                    m_XboxConsole.FindConsole(1, 1000);
                }
                catch (Exception ex)
                {
                    string errorString = m_XboxConsole.XboxManager.TranslateError(((System.Runtime.InteropServices.ExternalException)(ex)).ErrorCode);
                    Console.WriteLine("Return code: " + errorString);
                    ConnectToXbox(m_XBoxName);
                }
#if(FILE_COPY_XBCP)
                try
                {
                    CallCopyDelegate(srcFile.Name, (ulong)srcFile.Length, XBStatus.Trying, status);

                    Process xbcp = new Process();
                    string parameters = " /Y /X:" + m_XBoxName + " \"" + src + "\"" + " \"" + dst + "\"";

                    xbcp.StartInfo = new ProcessStartInfo(Environment.GetEnvironmentVariable("XEDK") + "\\bin\\win32\\xbcp.exe", parameters);
                    xbcp.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

                    if (!m_logOnly)
                    {
                        xbcp.Start();
                        xbcp.WaitForExit();
                    }

                    if (m_logOnly || xbcp.ExitCode == 0)
                    {
                        // In Move operations we remove the original file from the source
                        if (!m_logOnly && deleteSrcFile)
                            srcFile.Delete();

                        CallCopyDelegate(srcFile.Name, (ulong)srcFile.Length, XBStatus.Success, status);
                        return true;
                    }
                    else
                    {
                        CallCopyDelegate(srcFile.Name, (ulong)srcFile.Length, XBStatus.Failed, status);
                        System.Console.Write("xbcp could not copy file. Try rebooting your dev kit!");
                    }
                    return false;
                                    }
                catch (Exception ex)
                {
                    System.Console.WriteLine(ex.Message + "(" + ex.ToString() + ")");
                    return false;
                }
#else
                try
                {
                    CallCopyDelegate(srcFile.Name, srcFile.DirectoryName, (ulong)srcFile.Length, XBStatus.Trying, status);

                    if (!m_logOnly)
                    {
                        DateTime timeStart = DateTime.Now;

                        if (m_extraDebugOutput)
                            Console.WriteLine("\nSendFile Begin:\n\t'{0}' ->\n\t'{1}'", src, dst);

                        m_XboxConsole.SendFile(src, dst);

                        DateTime timeStop = DateTime.Now;
                        TimeSpan ts = timeStop - timeStart;
                        
                        if (m_extraDebugOutput)
                            Console.WriteLine("\nSendFile Took : {0} secs", ts.TotalSeconds);

                        //// Now we need to fix the time stamp
                        IXboxFile file = m_XboxConsole.GetFileObject(dst);

                        file.ChangeTime = srcFile.LastWriteTime;

                        //FileTimestamps timestamps = FileTimestamps.FromXboxFile(file);
                        //timestamps.CreationTime = srcFile.CreationTime;
                        //timestamps.ChangeTime = srcFile.LastWriteTime;
                        //timestamps.ToXboxFile(file);

                        //DateTime stampStop = DateTime.Now;
                        //TimeSpan tss = stampStop - timeStop;

                        //if (m_extraDebugOutput)
                        //    Console.WriteLine("\nTimeStamp Took: {0} secs", tss.TotalSeconds);

                        // In Move operations we remove the original file from the source
                        if (deleteSrcFile)
                            srcFile.Delete();
                    }

                    CallCopyDelegate(srcFile.Name, srcFile.DirectoryName, (ulong)srcFile.Length, XBStatus.Success, status);

                    return true;
                }
                catch (COMException ex)
                {
                    string errorString = m_XboxManager.TranslateError(ex.ErrorCode);
                    CallCopyDelegate(srcFile.Name, srcFile.DirectoryName, (ulong)srcFile.Length, XBStatus.Failed, status);
                    Console.WriteLine("Return code: " + errorString);

                    if (!CanResolveException(ex))
                        return false;
                }
#endif
            }
            
            return false;
        }

        public bool DeleteFile(string fileToDeleteName)
        {
            //FileInfo fileToDelete = new FileInfo(fileToDeleteName);

            string dirName = System.IO.Path.GetDirectoryName(fileToDeleteName);
            string fileName = System.IO.Path.GetFileName(fileToDeleteName);

            IXboxFile file = m_XboxConsole.GetFileObject(fileToDeleteName);
            
            for (int retry = 1; retry <= m_NumRetrys; retry++)
            {
                try
                {
                    m_XboxConsole.FindConsole(1, 1000);
                }
                catch (Exception ex)
                {
                    string errorString = m_XboxConsole.XboxManager.TranslateError(((System.Runtime.InteropServices.ExternalException)(ex)).ErrorCode);
                    Console.WriteLine("Return code: " + errorString);
                    ConnectToXbox(m_XBoxName);
                }

                try
                {
                    CallCopyDelegate(fileName, dirName, file.Size, XBStatus.Trying, BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra);

                    if (!m_logOnly)
                    {
                        m_XboxConsole.DeleteFile(fileToDeleteName);
                    }

                    CallCopyDelegate(fileName, dirName, file.Size, XBStatus.Success, BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra);

                    return true;
                }
                catch (COMException ex)
                {
                    string errorString = m_XboxManager.TranslateError(ex.ErrorCode);
                    CallCopyDelegate(fileName, dirName, file.Size, XBStatus.Failed, BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra);
                    Console.WriteLine("Return code: " + errorString);

                    if (!CanResolveException(ex))
                        return false;
                }
            }

            return false;
        }

        // Copys a file to the xbox.
        public bool CopyFile(string src, string dst, BizSyncWorkList.BizWorkItem.WorkItemStatus status)
        {
            return TransferFile(src, dst, false, status);
        }

        // Copys file to xbox deleting source.
        public bool MoveFile(string src, string dst, BizSyncWorkList.BizWorkItem.WorkItemStatus status)
        {
            return TransferFile(src, dst, true, status);
        }

        public void DeleteFolder(string dst)
        {
            try
            {
                if (!m_logOnly)
                    m_XboxConsole.RemoveDirectory(dst);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Warning: Could not delete Dir: " + dst);
            }
        }

        // Scans the source directory and returns a list of files/folders.
        public BizFileInfo ScanSrcDir(string src, BizProcessStatus bps)
        {
            System.Console.WriteLine("   Source : " + src);
            BizFileInfo bfiSrc;
            try
            {
                bfiSrc = new BizPCFileInfo();
                bfiSrc.ProcessFolder(src, bps, m_ScanDelegate);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
                return null;
            }
            return bfiSrc;
        }


        // Scans the destination directory and returns a list of files/folders.
        public BizFileInfo ScanDstDir(string dst, BizProcessStatus bps)
        {
            System.Console.WriteLine("     Dest : " + dst);
            BizFileInfo bfiDst;
            try
            {
                CreateFolder(dst);
                bfiDst = new BizXenonFileInfo();

                if (BizXenonFileInfo.Init( m_XboxConsole ))
                {  
                    bfiDst.ProcessFolder(dst, bps, m_ScanDelegate);
                }
            }

            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
                return null;
            }
            return bfiDst;
        }

        public void SetLogOnly(bool logOnly)
        {
            m_logOnly = logOnly;
        }

        public void SetExtraDebug(bool extraDebug)
        {
            m_extraDebugOutput = extraDebug;
        }

        // Fills in the work list dependant on the sync options
        public void CompareFolders(SyncType syncType, BizSyncWorkList workList, BizFileInfo src, BizFileInfo dst, XBSubOptions subOptions, string rootPath, string dirPath, List<string> fileSpec, List<string> dirSpec, ref Statistics statistics)
        {
            // Files
            {
                IEnumerator iLeft = src.filesList.GetEnumerator();
                IEnumerator iRight = dst.filesList.GetEnumerator();

                bool leftValid = iLeft.MoveNext();
                bool rightValid = iRight.MoveNext();

                while (leftValid || rightValid)
                {
                    BizFileInfo fiLeft = (leftValid) ? ((BizFileInfo)iLeft.Current) : null;
                    BizFileInfo fiRight = (rightValid) ? ((BizFileInfo)iRight.Current) : null;

                    System.Diagnostics.Debug.Assert((fiLeft == null) || (fiLeft.isFolder == false));
                    System.Diagnostics.Debug.Assert((fiRight == null) || (fiRight.isFolder == false));

                    // Process file/folder ignore list
                    bool ignore = false;

                    if (fiLeft != null)
                    {
                        // File ignore list.
                        for (int i = 0; i < fileSpec.Count; i++)
                        {
                            string ignorefile = fileSpec[i].ToLower().Replace('/', '\\');
                            if (fiLeft.name.Contains(ignorefile))
                            {
                                statistics.FilesSkipped++;
                                ignore = true;
                                break;
                            }
                        }
                    }

                    int nameCompare;

                    if (leftValid && rightValid)
                        nameCompare = fiLeft.name.ToLower().CompareTo(fiRight.name.ToLower());
                    else if (rightValid)
                        nameCompare = 1;
                    else
                        nameCompare = -1;

                    // File exists in source and dest.
                    if (nameCompare == 0)
                    {
                        if (!ignore)
                        {
                            long cft;

                            long lLeft = fiLeft.lastModified;
                            long lRight = fiRight.lastModified;

                            long lDiffInMilliSecs = (lLeft - lRight) / 10000;

                            // The XBox360 filesystem (and XBox for that matter) has a 2sec resolution
                            if ((lDiffInMilliSecs >= -2000) && (lDiffInMilliSecs <= 2000))
                                cft = 0;
                            else if (lDiffInMilliSecs < 0)
                                cft = -1;
                            else
                                cft = 1;

                            if (cft != 0)
                            {
                                if (cft == 1) // Src file is newer
                                {
                                    if (syncType == SyncType.CopyIfNewer || syncType == SyncType.Sync)
                                    {
                                        if (verboseDebug)
                                            System.Console.WriteLine("SourceNewer: {0} {1}", dirPath + fiLeft.name, (fiLeft.isFolder) ? "(folder)" : "");

                                        BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, false, false);
                                        newWorkItem.srcFile = fiLeft;
                                        newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.DestOlder;
                                        workList.AddWorkItem(newWorkItem);
                                    }
                                }
                                else if (cft == -1)
                                {
                                    if ((syncType == SyncType.CopyAlways) || (syncType == SyncType.Sync))
                                    {
                                        if (verboseDebug)
                                            System.Console.WriteLine("DestNewer: {0} {1} ({2} -> {3})", dirPath + fiRight.name, (fiRight.isFolder) ? "(folder)" : "", fiLeft.lastModified.ToString(), fiRight.lastModified.ToString());

                                        BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, false, false);
                                        newWorkItem.srcFile = fiLeft;
                                        newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.DestNewer;
                                        workList.AddWorkItem(newWorkItem);
                                    }
                                }
                            }
                            else
                            {
                                statistics.FilesSkipped++;
                                statistics.BytesSkipped += fiLeft.size;
                            }
                        }
                        else
                        {
                            BizSyncWorkList.BizWorkItem newWorkItem =
                                new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, true, false);
                            newWorkItem.srcFile = fiLeft;
                            workList.AddWorkItem(newWorkItem);
                        }

                        leftValid = iLeft.MoveNext();
                        rightValid = iRight.MoveNext();
                    }
                    else if (nameCompare < 0)
                    {
                        #region File only exists in source

                        if (!ignore)
                        {
                            if (syncType == SyncType.CopyIfNewer || syncType == SyncType.Sync)
                            {
                                if (verboseDebug)
                                    System.Console.WriteLine("NoMatchInDest: {0} {1}", dirPath + fiLeft.name, (fiLeft.isFolder) ? "(folder)" : "");

                                BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, false, false);
                                newWorkItem.srcFile = fiLeft;
                                newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.NewItem;

                                workList.AddWorkItem(newWorkItem);
                            }
                        }
                        else
                        {
                            BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, true, false);
                            newWorkItem.srcFile = fiLeft;
                            workList.AddWorkItem(newWorkItem);
                        }
                        // Process src until matches dst
                        leftValid = iLeft.MoveNext();
                        #endregion
                    }
                    else // nameCompare > 0
                    {
                        #region File only exists in destination

                        if (syncType == SyncType.Sync)
                        {
                            if (verboseDebug)
                                System.Console.WriteLine("NoMatchInSource: {0} {1}", dirPath + fiRight.name, (fiRight.isFolder) ? "(folder)" : "");

                            BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiRight.name, fiRight.size, fiRight.isFolder, false, true);
                            newWorkItem.srcFile = fiLeft;
                            newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra;

                            workList.AddWorkItem(newWorkItem);
                        }
                        else
                        {
                            statistics.FilesExtras++;
                            statistics.BytesExtras += fiRight.size;
                        }

                        // Process dst until matches dst
                        rightValid = iRight.MoveNext();
                        #endregion
                    }
                }
            }

            // Directories
            {
                IEnumerator iLeft = src.dirList.GetEnumerator();
                IEnumerator iRight = dst.dirList.GetEnumerator();

                bool leftValid = iLeft.MoveNext();
                bool rightValid = iRight.MoveNext();

                while (leftValid || rightValid)
                {
                    BizFileInfo fiLeft = (leftValid) ? ((BizFileInfo)iLeft.Current) : null;
                    BizFileInfo fiRight = (rightValid) ? ((BizFileInfo)iRight.Current) : null;

                    System.Diagnostics.Debug.Assert((fiLeft == null) || (fiLeft.isFolder == true));
                    System.Diagnostics.Debug.Assert((fiRight == null) || (fiRight.isFolder == true));

                    // Process file/folder ignore list
                    bool ignore = false;

                    if (fiLeft != null)
                    {
                        // Log folders with the WorkItem stuff
                        workList.RegisterFolder(dirPath + fiLeft.name);

                        // Folder ignore list.
                        for (int i = 0; i < dirSpec.Count; i++)
                        {
                            string fullname = "";
                            string ignoredir = dirSpec[i].ToLower().Replace('/', '\\');

                            if (ignoredir.Contains("\\"))
                                fullname = rootPath.ToLower() + dirPath.ToLower() + fiLeft.name.ToLower();
                            else
                                fullname = fiLeft.name.ToLower();

                            if (fullname.CompareTo(ignoredir) == 0)
                            {
                                statistics.DirsSkipped++;
                                ignore = true;
                                break;
                            }
                        }
                    }

                    int nameCompare;

                    if (leftValid && rightValid)
                        nameCompare = fiLeft.name.ToLower().CompareTo(fiRight.name.ToLower());
                    else if (rightValid)
                        nameCompare = 1;
                    else
                        nameCompare = -1;

                    // File exists in source and dest.
                    if (nameCompare == 0)
                    {
                        if (!ignore)
                        {
                            if (subOptions != XBSubOptions.NonRecursive)
                            {
                                CompareFolders(syncType, workList, fiLeft, fiRight, subOptions, rootPath, dirPath + fiLeft.name + "\\", fileSpec, dirSpec, ref statistics);
                            }

                            statistics.DirsSkipped++;
                        }
                        else
                        {
                            BizSyncWorkList.BizWorkItem newWorkItem =
                                new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, true, false);
                            newWorkItem.srcFile = fiLeft;
                            workList.AddWorkItem(newWorkItem);
                        }

                        leftValid = iLeft.MoveNext();
                        rightValid = iRight.MoveNext();
                    }
                    else if (nameCompare < 0)
                    {
                        #region File only exists in source

                        if (!ignore)
                        {
                            if (syncType == SyncType.CopyIfNewer || syncType == SyncType.Sync)
                            {
                                if (verboseDebug)
                                    System.Console.WriteLine("NoMatchInDest: {0} {1}", dirPath + fiLeft.name, (fiLeft.isFolder) ? "(folder)" : "");

                                BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, false, false);
                                newWorkItem.srcFile = fiLeft;
                                newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.NewItem;

                                // If we've just added a folder, we need to add all it's children to the worklist
                                if (subOptions != XBSubOptions.NonRecursive)
                                {
                                    // If folder isn't empty.
                                    if (fiLeft.filesList.Count > 0 || fiLeft.dirList.Count > 0)
                                    {
                                        workList.AddWorkItem(newWorkItem);

                                        BizFileInfo.TraverseFolderToCopyToDest(syncType, workList, fiLeft, subOptions, rootPath, dirPath + fiLeft.name + "\\", fileSpec, dirSpec);
                                    }
                                    else if (subOptions == XBSubOptions.CopyEmptySub)
                                    {
                                        workList.AddWorkItem(newWorkItem);
                                    }
                                }
                                else
                                {
                                    workList.AddWorkItem(newWorkItem);
                                }
                            }
                        }
                        else
                        {
                            BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, true, false);
                            newWorkItem.srcFile = fiLeft;
                            workList.AddWorkItem(newWorkItem);
                        }
                        // Process src until matches dst
                        leftValid = iLeft.MoveNext();
                        #endregion
                    }
                    else // nameCompare > 0
                    {
                        #region File only exists in destination

                        if (syncType == SyncType.Sync)
                        {
                            if (verboseDebug)
                                System.Console.WriteLine("NoMatchInSource: {0} {1}", dirPath + fiRight.name, (fiRight.isFolder) ? "(folder)" : "");

                            {
                                BizSyncWorkList.BizWorkItem newWorkItemFake = new BizSyncWorkList.BizWorkItem(dirPath + fiRight.name, fiRight.size, fiRight.isFolder, false, true);
                                newWorkItemFake.srcFile = fiLeft;
                                newWorkItemFake.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra;
                                newWorkItemFake.fakeDelete = true;

                                workList.AddWorkItem(newWorkItemFake);
                            }

                            BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiRight.name, fiRight.size, fiRight.isFolder, false, true);
                            newWorkItem.srcFile = fiLeft;
                            newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.Extra;

                            if (subOptions != XBSubOptions.NonRecursive)
                            {
                                BizFileInfo.TraverseFolderToRemoveFromDest(syncType, workList, fiRight, subOptions, dirPath + fiRight.name + "\\", fileSpec, dirSpec);
                                workList.AddWorkItem(newWorkItem);
                            }
                            else
                            {
                                workList.AddWorkItem(newWorkItem);
                            }
                        }
                        else
                        {
                            statistics.DirsExtras++;
                        }

                        // Process dst until matches dst
                        rightValid = iRight.MoveNext();
                        #endregion
                    }
                }
            }
        }
    };
}// namespace XBrobocopy.

