using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace application
{
	public partial class TestGadget : application.BaseGadget
	{
		public TestGadget()
		{
			InitializeComponent();
		}

		private void buttonHello_Click(object sender, EventArgs e)
		{
			MessageBox.Show("Hello! :-)");
		}

		int count = 0;

		private void timerTest_Tick(object sender, EventArgs e)
		{
			this.label1.Text = count.ToString();
			count++;
		}
	}
}

