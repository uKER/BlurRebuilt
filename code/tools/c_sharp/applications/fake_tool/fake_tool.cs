using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace FakeTool
{
	public partial class FakeTool : Form
	{
		public FakeTool()
		{
			InitializeComponent();
		}
		public FakeTool(string[] args)
		{
			InitializeComponent();

			if(args.Length == 1)
			{
				outputTextBox.AppendText("Opening file '" + args[0] + "'\n\n");

				try
				{
					StreamReader sr = new StreamReader(args[0]);
					string strTemp = sr.ReadToEnd();
					outputTextBox.AppendText("The file contains the following text:\n" + strTemp + "\n");
				}
				catch(Exception ex)
				{
					outputTextBox.AppendText("Failed to open the file. The system reports:\n" + ex.Message + "\n");
				}
			}
			else
			{
				outputTextBox.AppendText("Expected a filename as the first and only argument.\n\n");
			}
		}
	}
}