using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Text.RegularExpressions;

namespace ProfilingApplication
{
	public partial class CustomCounterDlg : Form
	{
		public CustomCounterDlg()
		{
			InitializeComponent();
		}

		public CustomCounter GetCustomCounter()
		{
			CustomCounter rtn = new CustomCounter();
			rtn.sName = nameTextBox.Text;
			rtn.sExpression = expressionTextBox.Text;
			return rtn;
		}

		private void button2_Click(object sender, EventArgs e)
		{
			Regex macroRegex = new Regex(@"\$\([^\)]+\)");
			string s = macroRegex.Replace(expressionTextBox.Text, "0");

			try
			{
				ExpressionEvaluator.Calculate(s);
			}
			catch (Exception x)
			{
				MessageBox.Show("The entered expression is invalid:\n\n" + x.Message, "Expression Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}

			DialogResult = DialogResult.OK;
			Close();
		}
	}
}