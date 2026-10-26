using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Xml;

namespace ActorSystem
{
	public class ActorBool : ActorSystem.ActorParameter, IActorParameter
	{
		private System.Windows.Forms.GroupBox MainGroupBox;
		private System.Windows.Forms.CheckBox checkBox;
		private System.ComponentModel.IContainer components = null;

//		public ActorBool()
		public ActorBool(Control owner, XmlNode valueNode) : base(owner, valueNode)
		{
			// This call is required by the Windows Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.MainGroupBox = new System.Windows.Forms.GroupBox();
			this.checkBox = new System.Windows.Forms.CheckBox();
			this.MainGroupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// MainGroupBox
			// 
			this.MainGroupBox.Controls.Add(this.checkBox);
			this.MainGroupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.MainGroupBox.Location = new System.Drawing.Point(0, 0);
			this.MainGroupBox.Name = "MainGroupBox";
			this.MainGroupBox.Size = new System.Drawing.Size(152, 40);
			this.MainGroupBox.TabIndex = 0;
			this.MainGroupBox.TabStop = false;
			// 
			// checkBox
			// 
			this.checkBox.Location = new System.Drawing.Point(16, 12);
			this.checkBox.Name = "checkBox";
			this.checkBox.Size = new System.Drawing.Size(104, 26);
			this.checkBox.TabIndex = 0;
			this.checkBox.Text = "checkBox";
			this.checkBox.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
			// 
			// ActorBool
			// 
			this.Controls.Add(this.MainGroupBox);
			this.Name = "ActorBool";
			this.Size = new System.Drawing.Size(152, 40);
			this.MainGroupBox.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void checkBox_CheckedChanged(object sender, System.EventArgs e)
		{
			CallParameterChanged(this);
	
		}

		#region IActorParameter Members

		public override string _ActorName
		{
			get
			{
				return this.checkBox.Text;
			}
			set
			{
				this.checkBox.Text = value;

			}
		}

		public override string _ActorValue
		{
			get
			{
				// TODO:  Add ActorInt.ActorValue getter implementation
				return this.checkBox.Checked.ToString();
			}
			set
			{
				// TODO:  Add ActorInt.ActorValue setter implementation
				bool newValue = false;
				if(value == "1" || value.ToLower() == "true" || value.ToLower() == "yes")
					newValue = true;
				this.checkBox.Checked = newValue;
			}
		}

		#endregion
	}
}

