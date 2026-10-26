using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Xml;

namespace ActorSystem
{
	/// <summary>
	/// Summary description for ActorParameterGroup.
	/// </summary>
	public class ActorParameterGroup : ActorParameter
	{
		private System.Windows.Forms.GroupBox groupBox;
		private System.ComponentModel.Container components = null;

		public int m_idealHeight;

		private void InitializeComponent()
		{
			this.groupBox = new System.Windows.Forms.GroupBox();
			this.SuspendLayout();
			// 
			// groupBox
			// 
			this.groupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox.Location = new System.Drawing.Point(0, 0);
			this.groupBox.Name = "groupBox";
			this.groupBox.Size = new System.Drawing.Size(504, 384);
			this.groupBox.TabIndex = 0;
			this.groupBox.TabStop = false;
			this.groupBox.Text = "Properties";
			// 
			// ActorParameterGroup
			// 
			this.Controls.Add(this.groupBox);
			this.Name = "ActorParameterGroup";
			this.Size = new System.Drawing.Size(504, 384);
			this.ResumeLayout(false);

		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

	
		public ActorParameterGroup()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
			//
			// TODO: Add constructor logic here
			//
		}





		#region IActorParameter Members

		public override string _ActorName
		{
			get
			{
				return this.groupBox.Name;
			}
			set
			{
				this.groupBox.Name = value;
			}
		}

		public override string _ActorValue
		{
			get
			{
				return null;
			}
		}

		public UserControl.ControlCollection ChildControls
		{
			get
			{
				return this.groupBox.Controls;
			}
		}
		#endregion
	}
}
