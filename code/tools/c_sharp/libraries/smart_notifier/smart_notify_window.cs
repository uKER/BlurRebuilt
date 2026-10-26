using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;

namespace global
{
	public partial class SmartNotifyWindow : Form
	{

		public int m_iVisibility;
		System.Threading.Timer m_timer = null;

		public SmartNotifyWindow(string strMessage)
		{
			InitializeComponent();
		}



		public delegate void MyDelegateCallback(float opacity);

		public void OpacityCallback(float opacity)
		{
			Trace.WriteLine("Opacity : " + opacity);
			this.Text = "Opacity: " + opacity.ToString();
			this.Opacity = opacity;
//			snw.Invalidate();
			this.Update();
		}


	}
}