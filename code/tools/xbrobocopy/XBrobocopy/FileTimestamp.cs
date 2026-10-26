#region File Information
//-----------------------------------------------------------------------------
// FileTimestamps.cs
//
// Developed by Microsoft XNA Developer Connection
// Copyright (c) 2007 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Runtime.InteropServices;
using ComTypes = System.Runtime.InteropServices.ComTypes;
using XDevkit;

namespace XDevkitWorkaround
{
    /// <summary>
    /// This class stores the creation time and last modified times for a file.
    /// </summary>
    public class FileTimestamps
    {
        #region Internal structures, classes and methods

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        private struct DM_FILE_ATTRIBUTES
        {
            public const int NameLength = 256;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = NameLength)]
            public string Name;

            public ComTypes.FILETIME CreationTime;
            public ComTypes.FILETIME ChangeTime;

            public uint SizeHigh;
            public uint SizeLow;
            public uint Attributes;
        }

        private class Kernel
        {
            public const string KernelDll = "kernel32.dll";

            [DllImport(KernelDll)]
            public static extern bool FileTimeToLocalFileTime(
                [In] ref ComTypes.FILETIME filetime,
                [Out] out ComTypes.FILETIME localfiletime
            );

            [DllImport(KernelDll)]
            public static extern bool LocalFileTimeToFileTime(
                [In] ref ComTypes.FILETIME localfiletime,
                [Out] out ComTypes.FILETIME filetime
            );
        }

        private class Xbdm
        {
            public const string XbdmDll = "xbdm.dll";

            [DllImport(XbdmDll, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
            public static extern IntPtr DmGetFileAttributes(
                [In] string filename,
                [Out] out DM_FILE_ATTRIBUTES attributes
            );

            [DllImport(XbdmDll, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
            public static extern IntPtr DmSetFileAttributes(
                [In] string filename,
                [In] ref DM_FILE_ATTRIBUTES attributes
            );
        }

        private static DateTime FileTimeToDateTime(ComTypes.FILETIME filetime)
        {
            DateTime DateTimeBias = new DateTime(1601, 1, 1);
            
            long ticks = (((long)filetime.dwHighDateTime) << 32) | (uint)filetime.dwLowDateTime;

            DateTime datetime = new DateTime(ticks + DateTimeBias.Ticks);

            //if (!datetime.IsDaylightSavingTime())
            //    ticks -= 1 * System.TimeSpan.TicksPerHour;
            
            return new DateTime(ticks + DateTimeBias.Ticks);
        }

        private static ComTypes.FILETIME DateTimeToFileTime(DateTime datetime)
        {
            DateTime DateTimeBias = new DateTime(1601, 1, 1);
            ComTypes.FILETIME filetime;
            long ticks = datetime.Ticks - DateTimeBias.Ticks;

            if (!datetime.IsDaylightSavingTime())
                ticks += 1 * System.TimeSpan.TicksPerHour;

            filetime.dwHighDateTime = (int)(ticks >> 32);
            filetime.dwLowDateTime = (int)(ticks & 0xffffffffL);
            return filetime;
        }

        private DateTime creationTime;
        private DateTime changeTime;

        #endregion

        /// <summary>
        /// The time the file was created.
        /// </summary>
        public DateTime CreationTime { get { return creationTime; } set { creationTime = value; } }

        /// <summary>
        /// The time the file was last modified.
        /// </summary>
        public DateTime ChangeTime { get { return changeTime; } set { changeTime = value; } }

        /// <summary>
        /// Gets the timestamps for the specified file.
        /// </summary>
        public static FileTimestamps FromXboxFile(IXboxFile file)
        {
            FileTimestamps timestamps = new FileTimestamps();

            DM_FILE_ATTRIBUTES attribs;
            Xbdm.DmGetFileAttributes(file.Name, out attribs);

            ComTypes.FILETIME local;
            Kernel.FileTimeToLocalFileTime(ref attribs.CreationTime, out local);
            timestamps.CreationTime = FileTimeToDateTime(local);

            Kernel.FileTimeToLocalFileTime(ref attribs.ChangeTime, out local);
            timestamps.ChangeTime = FileTimeToDateTime(local);

            return timestamps;
        }

        /// <summary>
        /// Sets the timestamps for the specified file.
        /// </summary>
        public void ToXboxFile(IXboxFile file)
        {
            DM_FILE_ATTRIBUTES attribs;
            Xbdm.DmGetFileAttributes(file.Name, out attribs);

            ComTypes.FILETIME local;
            local = DateTimeToFileTime(creationTime);
            Kernel.LocalFileTimeToFileTime(ref local, out attribs.CreationTime);

            local = DateTimeToFileTime(changeTime);
            Kernel.LocalFileTimeToFileTime(ref local, out attribs.ChangeTime);

            Xbdm.DmSetFileAttributes(file.Name, ref attribs);
        }
    }
}
