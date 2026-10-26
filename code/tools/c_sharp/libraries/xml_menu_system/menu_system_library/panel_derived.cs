using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Windows.Message;

namespace MenuSystemLibrary
{
	public partial class panel_derived : Panel
	{
		public panel_derived()
		{
			InitializeComponent();
			//SetStyle(ControlStyles.Selectable, false);		// Disallow receiving focus.
		}


		/// <summary>
		/// Bugfix to stop the annoying scroll jump when focus changes in the panel.
		/// </summary>
		/// <param name="activeControl">The control that has gained focus.</param>
		/// <returns>A position to scroll to.</returns>
		protected override Point ScrollToControl(Control activeControl)
		{
			return this.AutoScrollPosition;
		}
	}
}
