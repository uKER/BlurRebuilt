using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Collections;
using System.IO;
using System.Runtime.InteropServices;

using System.ComponentModel;

using System.Management;

using Bizarre.Misc;

/*
 * Set the depth to go before we do checks and compress
 * 
 */ 

namespace DirCompactAndDelete
{
    class Program
    {
        static string rootFolder;

        static int folderDepth = 1;

        [DllImport("kernel32.dll")]
        public static extern int DeviceIoControl(IntPtr hDevice, int dwIoControlCode, ref short lpInBuffer, int nInBufferSize, IntPtr lpOutBuffer, int nOutBufferSize, ref int lpBytesReturned, IntPtr lpOverlapped);

        [DllImport("kernel32.dll", SetLastError = true, EntryPoint = "GetCompressedFileSize")]
        static extern uint GetCompressedFileSizeAPI(string lpFileName, out uint lpFileSizeHigh);

        static void CompressFolder(string dirName)
        {
            DirectoryInfo di = new DirectoryInfo(dirName);

            foreach (var d in di.GetDirectories())
            {
                CompressFolder(d.FullName);
            }

            string objPath = "Win32_Directory.Name=" + "\"" + dirName.Replace("\\", "\\\\") + "\"";

            try
            {
                using (ManagementObject dir = new ManagementObject(objPath))
                {
                    ManagementBaseObject outParams = dir.InvokeMethod("Compress", null, null);

                    uint ret = (uint)(outParams.Properties["ReturnValue"].Value);

                    if (ret != 0)
                    {
                        Console.WriteLine("Failed with error code: {0}", ret);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("CompressFolder: Exception: " + ex.Message);
                Console.WriteLine("CompressFolder: Tried: '" + objPath + "'");
            }
        }

        static void DeleteFolder(string dirName)
        {
            DirectoryInfo di = new DirectoryInfo(dirName);

            foreach (var d in di.GetDirectories())
            {
                DeleteFolder(d.FullName);
            }

            foreach (var f in di.GetFiles())
            {
                f.Attributes = FileAttributes.Normal;
                File.Delete(f.FullName);
            }

            di.Attributes = FileAttributes.Normal;
            Directory.Delete(dirName);
        }

        //static void CompressFile(string fileName)
        //{
        //    //string fileName = @"C:\Temp\test.mdb";
        //    int lpBytesReturned = 0;
        //    int FSCTL_SET_COMPRESSION = 0x9C040;
        //    short COMPRESSION_FORMAT_DEFAULT = 1;

        //    FileStream f = File.Open(fileName, System.IO.FileMode.Open, System.IO.FileAccess.ReadWrite, System.IO.FileShare.None);

        //    int result = DeviceIoControl(f.Handle, FSCTL_SET_COMPRESSION,
        //        ref COMPRESSION_FORMAT_DEFAULT, 2 /*sizeof(short)*/, IntPtr.Zero, 0,
        //        ref lpBytesReturned, IntPtr.Zero);
        //}

        public static ulong GetCompressedFileSize(string filename)
        {
            uint high;
            uint low;
            low = GetCompressedFileSizeAPI(filename, out high);
            int error = Marshal.GetLastWin32Error();
            if (high == 0 && low == 0xFFFFFFFF && error != 0)
                throw new Win32Exception(error);
            else
                return ((ulong)high << 32) + low;
        }

        public static void DirSize(DirInfo info, DirectoryInfo d)
        {
            // Add file sizes.
            FileInfo[] fis = d.GetFiles();
            foreach (FileInfo fi in fis)
            {
                info.csize += GetCompressedFileSize(fi.FullName);
                info.size += (ulong)fi.Length;
                info.files++;
            }

            // Add subdirectory sizes.
            DirectoryInfo[] dis = d.GetDirectories();
            foreach (DirectoryInfo di in dis)
            {
                DirSize(info, di);
                info.folders++;
            }
        }

        public static DirInfo DirSize(string folderName)
        {
            DirInfo info = new DirInfo();

            DirSize(info, new DirectoryInfo(folderName));

            return info;
        }

        static DirInfo ScanFolders(WorkInfo workInfo, string baseFolder, int depth, List<DirInfo> dirInfo)
        {
            DirInfo dirinf = new DirInfo();

            DirectoryInfo di = new DirectoryInfo(baseFolder);

            foreach (var d in di.GetDirectories())
            {
                string friendlyName = d.FullName.Substring(rootFolder.Length + 1);
                // If we're at the right depth
                if (depth == 0)
                {
                    // Find out how long this folder has been here
                    var diff = DateTime.Now - d.CreationTime;

                    bool isCompressed = (d.Attributes & FileAttributes.Compressed) == FileAttributes.Compressed;

                    bool deleted = false;

                    if (diff.Days > workInfo.AgeToDelete)
                    {
                        System.Console.WriteLine("Del   : " + friendlyName);

                        if (!workInfo.JustShowActions)
                        {
                            DeleteFolder(d.FullName);
                            deleted = true;
                        }
                    }
                    else if (diff.Days > workInfo.AgeToCompress && !isCompressed)
                    {
                        System.Console.WriteLine("Shrink: {0} - Compress", friendlyName);

                        if (!workInfo.JustShowActions)
                        {
                            CompressFolder(d.FullName);
                        }
                    }

                    if (!deleted && workInfo.CalculateSizes)
                    {
                        DirInfo info = DirSize(d.FullName);

                        if (info.csize < info.size)
                        {
                            System.Console.WriteLine("Folder: '{0}' - Size {1} - Shrunk({2})", friendlyName, SizeToString(info.csize), SizeToString(info.size - info.csize));
                        }
                        else
                        {
                            System.Console.WriteLine("Folder: '{0}' - Size {1}", friendlyName, SizeToString(info.size));
                        }

                        dirinf = dirinf + info;
                    }
                }
                else
                {
                    System.Console.WriteLine("Scan  : '{0}'", friendlyName);

                    DirInfo r = ScanFolders(workInfo, d.FullName, depth - 1, dirInfo);

                    if (depth == 1)
                    {
                        dirInfo.Add(r);
                    }
                }
            }

            dirinf.name = baseFolder;

            return dirinf;
        }

        static string SizeToString(ulong size)
        {
            if (size > 9)
            {
                if (size > 1024 * 1024 * 1024)
                {
                    return (size / (1024.0 * 1024.0 * 1024.0)).ToString("#,###.##") + " GB";
                }
                else if (size > 1024 * 1024)
                {
                    return (size / (1024.0 * 1024.0)).ToString("#,###.##") + " MB";
                }
                else if (size > 1024)
                {
                    return (size / 1024.0).ToString("#,###.##") + "k";
                }
                else
                {
                    return size.ToString("#,###.#");
                }
            }
            else
            {
                return size.ToString();
            }
        }

        static void Main(string[] args)
        {
            WorkInfo workInfo = new WorkInfo();

            //rootFolder = @"c:\testdata";
            rootFolder = @"E:\SymbolServer\Source";

            List<DirInfo> dirInfo = new List<DirInfo>();

            System.Console.WriteLine("Root Folder: " + rootFolder);

            ScanFolders(workInfo, rootFolder, folderDepth, dirInfo);

            DirInfo di = new DirInfo();

            int maxNameLen = 5;
            ulong maxFile = 1;
            ulong maxFolder = 1;
            int maxSizeLen = 0;

            foreach (var d in dirInfo)
            {
                int nlen = d.name.Substring(rootFolder.Length + 1).Length;
                int fslen = SizeToString(d.csize).Length;

                if (nlen > maxNameLen)
                    maxNameLen = nlen;

                if (fslen > maxSizeLen)
                    maxSizeLen = fslen;

                if (d.files > maxFile)
                    maxFile = d.files;

                if (d.folders > maxFolder)
                    maxFolder = d.folders;

                di = di + d;
            }

            if (SizeToString(di.csize).Length > maxSizeLen)
                maxSizeLen = SizeToString(di.csize).Length;
           
            if (di.files > maxFile)
                maxFile = di.files;

            if (di.folders > maxFolder)
                maxFolder = di.folders;

            int maxFileLen = maxFile.ToString("#,###.#").Length;
            int maxFolderLen = maxFolder.ToString("#,###.#").Length;

            if (5 > maxFileLen)
                maxFileLen = 5;

            if (7 > maxFolderLen)
                maxFolderLen = 7;

            string separatorLine = new string('-', maxNameLen + maxSizeLen + maxFileLen + maxFolderLen + 13);

            string formatString = "| {0,-" + maxNameLen + "} | {1," + maxSizeLen + "} | {2," + maxFileLen + "} | {3," + maxFolderLen + "} |";

            System.Console.WriteLine(separatorLine);
            System.Console.WriteLine(formatString, "Path", "Size", "Files", "Folders");
            System.Console.WriteLine(separatorLine);

            foreach (var d in dirInfo)
            {
                System.Console.WriteLine(formatString, d.name.Substring(rootFolder.Length + 1), SizeToString(d.csize), d.files > 0 ? d.files.ToString("#,###.#") : "0", d.folders > 0 ? d.folders.ToString("#,###.#") : "0");
            }

            System.Console.WriteLine(separatorLine);
            System.Console.WriteLine(formatString, "Total", SizeToString(di.csize), di.files.ToString("#,###.#"), di.folders.ToString("#,###.#"));
            System.Console.WriteLine(separatorLine);            
        }

        public class WorkInfo
        {
            public uint AgeToDelete = 90;
            public uint AgeToCompress = 5;

            public bool CalculateSizes = true;

            public bool JustShowActions = false;
        }

        public class DirInfo
        {
            public string name;
            public ulong size = 0;
            public ulong csize = 0;
            public ulong files = 0;
            public ulong folders = 0;

            public static DirInfo operator +(DirInfo d1, DirInfo d2)
            {
                DirInfo r = new DirInfo();

                r.name = d1.name + "+" + d2.name;
                r.size = d1.size + d2.size;
                r.csize = d1.csize + d2.csize;
                r.files = d1.files + d2.files;
                r.folders = d1.folders + d2.folders;

                return r;
            }
        }
    }
}
