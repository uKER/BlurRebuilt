using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
//using FreeImageAPI;
using System.Runtime.InteropServices;
using System.Threading;

namespace file_tag_manager_2
{
	public partial class ImageViewer : Form
	{
		private UInt32 fi;
		EventWaitHandle m_newImage_Wait;
		Thread m_viewerThread;

		object m_filenameLock = new object();
		object m_paintLock = new object();
		long m_paintIsAllowed = 0;				// Long behaving like a bool, so it can be managed through the Interlock class.

		string[] m_strNextFilename = new string[2] { null, null };
		int m_nextFilenameIndex = 0;

		public ImageViewer()
		{
			InitializeComponent();
		}

		private void image_viewer_Load(object sender, EventArgs e)
		{
			m_newImage_Wait = new EventWaitHandle(false, EventResetMode.AutoReset);
			m_viewerThread = new Thread(new ThreadStart(ViewerThread));
			m_viewerThread.Name = "Image viewer.";
			m_viewerThread.Priority = ThreadPriority.BelowNormal;
			m_viewerThread.Start();
		}

		#region Imports ...
		[DllImport("Gdi32.dll")]
		private static extern int SetStretchBltMode(IntPtr HDC, int iStretchMode);

		[DllImport("Gdi32.dll")]
		private static extern int StretchDIBits(
			IntPtr HDC,
			int XDest,                    // x-coord of destination upper-left corner
			int YDest,                    // y-coord of destination upper-left corner
			int nDestWidth,               // width of destination rectangle
			int nDestHeight,              // height of destination rectangle
			int XSrc,                     // x-coord of source upper-left corner
			int YSrc,                     // y-coord of source upper-left corner
			int nSrcWidth,                // width of source rectangle
			int nSrcHeight,               // height of source rectangle
			IntPtr lpBits,								// bitmap bits
			IntPtr lpBitsInfo,				// bitmap data
			//			BITMAPINFO lpBitsInfo,				// bitmap data
			uint iUsage,                  // usage options
			int dwRop											// raster operation code
			);
		#endregion Imports.


		public bool SetImage(string strFilename)
		{
			lock (m_filenameLock)			// Stop the viewer thread mucking about with the filenames ...
			{
				m_strNextFilename[m_nextFilenameIndex] = strFilename;
			}
			m_newImage_Wait.Set();

			return true;
		}

		void ViewerThread()
		{
			string strNextFilename;

			while (true)
			{
				m_newImage_Wait.WaitOne();

				lock (m_filenameLock)			// Stop the main thread mucking about with the filenames ...
				{
					strNextFilename = m_strNextFilename[m_nextFilenameIndex];		// Get the next filename.
					m_nextFilenameIndex = 1 - m_nextFilenameIndex;					// Set where to put new filenames.
				}

				// Wait until we are not in the paint callback.
				// Ensure we do not re-enter the paint callback.
				// Make 'fi' unavailable to the paint callback. Do not block the paint callback.
				lock (m_paintLock)
				{
					Interlocked.Exchange(ref m_paintIsAllowed, 0);
				}

				if (fi != 0)
				{
					FreeImage.Unload(fi);
					this.panelPicture.Invalidate();
					fi = 0;
				}

				if(strNextFilename != null)
				{
					fi = LoadImage(strNextFilename);



				}
				// Make 'fi' available to the paint callback.
				lock (m_paintLock)
				{
					Interlocked.Exchange(ref m_paintIsAllowed, 1);
				}
				this.panelPicture.Invalidate();
			}
		}


		public UInt32 LoadImage(string strImage)
		{
			UInt32 fi = 0;

			FREE_IMAGE_FORMAT fif = FREE_IMAGE_FORMAT.FIF_UNKNOWN;
			// check the file signature and deduce its format
			// (the second argument is currently not used by FreeImage)
			fif = FreeImage.GetFileType(strImage, 0);
			if(fif == FREE_IMAGE_FORMAT.FIF_UNKNOWN)
			{
				// no signature ?
				// try to guess the file format from the file extension
				fif = FreeImage.GetFIFFromFilename(strImage);
			}
			// check that the plugin has reading capabilities ...
			if((fif != FREE_IMAGE_FORMAT.FIF_UNKNOWN) && FreeImage.FIFSupportsReading(fif))
			{
				// ok, let's load the file
				fi = FreeImage.Load(fif, strImage, 0);
				// unless a bad file format, we are done !
			}

			return fi;
		}




		private void panelPicture_Paint(object sender, PaintEventArgs e)
		{
			lock (m_paintLock)
			{
				if (Interlocked.Read(ref m_paintIsAllowed) != 0)
				{
					if (fi != 0)
					{
						IntPtr hdc = e.Graphics.GetHdc();

						int r = SetStretchBltMode(hdc, 3 /* COLORONCOLOR */);
						r = StretchDIBits(hdc,
							0, 0, this.ClientSize.Width, this.ClientSize.Height,
							0, 0, (int)FreeImage.GetWidth(this.fi), (int)FreeImage.GetHeight(this.fi),
							FreeImage.GetBits(this.fi),

							FreeImage.FreeImage_GetInfo(this.fi),
							0 /* DIB_RGB_COLORS */, 0x00CC0020 /* SRCCOPY */);

						e.Graphics.ReleaseHdc(hdc);
					}
					else
						e.Graphics.Clear(Color.Bisque);

				}
				else
					e.Graphics.Clear(Color.Black);
			}
		}

		private void image_viewer_Resize(object sender, EventArgs e)
		{
			this.panelPicture.Invalidate();
		}

		private void image_viewer_FormClosing(object sender, FormClosingEventArgs e)
		{
			if(e.CloseReason == CloseReason.UserClosing)
			{
				this.Visible = false;
				e.Cancel = true;
			}
		}

		internal void Unload(uint fi)
		{
			FreeImage.Unload(fi);
		}



		internal UInt32 GetWidth(uint fi)
		{
			if(fi != 0)
			{
				return FreeImage.GetWidth(fi);
			}
			else
				return 0;
		}

		internal UInt32 GetHeight(uint fi)
		{
			if(fi != 0)
			{
				return FreeImage.GetHeight(fi);
			}
			else
				return 0;
		}




		/*
		 * 
		 * 					int w = (int)FreeImage.GetWidth(fi);
					int h = (int)FreeImage.GetHeight(fi);

					System.Drawing.Imaging.PixelFormat format = System.Drawing.Imaging.PixelFormat.DontCare;
					uint bpp = FreeImage.GetBPP(fi);
					if (bpp == 24)
						format = System.Drawing.Imaging.PixelFormat.Format24bppRgb;
					else if (bpp == 32)
						format = System.Drawing.Imaging.PixelFormat.Format32bppArgb;

					Debug.WriteLine("Pitch: " + FreeImage.GetPitch(fi).ToString());
					Debug.WriteLine("BPP: " + FreeImage.GetBPP(fi).ToString()); ;

					int pitch = (int)FreeImage.GetPitch(fi);

					if (format != System.Drawing.Imaging.PixelFormat.DontCare)
						this.pictureBox.Image = new Bitmap(w, h, pitch, format, FreeImage.GetBits(fi));
		 * 
		 * */




		internal void KillThread()
		{
			this.m_viewerThread.Abort();
		}
	}
}