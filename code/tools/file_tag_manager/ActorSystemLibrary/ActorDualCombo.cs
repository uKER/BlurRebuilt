using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;

namespace ActorSystem
{
	/// <summary>
	/// Summary description for ActorDualCombo.
	/// </summary>
	public class ActorDualCombo : ActorParameter
	{
		public System.Windows.Forms.GroupBox groupBox;
		public ActorSystem.ActorDropbox classDropbox;
		public ActorSystem.ActorDropbox actorDropbox;

		public ActorParameterGroup actorParameterGroup;
		public XmlNode sourceXmlNode;
		public ActorParameterGroup nextActorParameterGroup;

		public enum State
		{
			Killing = 0,
			Normal
		}

		public State state = State.Normal;

		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components;

//		public GroupBox childGroupBox;
		public ActorDualCombo(Control owner, ArrayList strEntries1, ArrayList strEntries2, XmlNode classNode, XmlNode actorNode)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// Add any initialization after the InitializeComponent call
			this.classDropbox.OwnerForm = owner;
			this.actorDropbox.OwnerForm = owner;
			this.classDropbox.ValueNode = classNode;
			this.actorDropbox.ValueNode = actorNode;

			foreach(string strEntry in strEntries1)
				this.classDropbox.comboBox.Items.Add(strEntry);
			foreach(string strEntry in strEntries2)
				this.actorDropbox.comboBox.Items.Add(strEntry);
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
//			Debug.WriteLine("Disposed DualCombo.");

		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.groupBox = new System.Windows.Forms.GroupBox();
			this.actorDropbox = new ActorSystem.ActorDropbox();
			this.classDropbox = new ActorSystem.ActorDropbox();
			this.groupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox
			// 
			this.groupBox.Controls.Add(this.actorDropbox);
			this.groupBox.Controls.Add(this.classDropbox);
			this.groupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox.Location = new System.Drawing.Point(0, 0);
			this.groupBox.Name = "groupBox";
			this.groupBox.Size = new System.Drawing.Size(376, 72);
			this.groupBox.TabIndex = 0;
			this.groupBox.TabStop = false;
			// 
			// actorDropbox
			// 
			this.actorDropbox._ActorName = "Actor";
			this.actorDropbox._ActorValue = null;
			this.actorDropbox.ActorName = "Actor";
			this.actorDropbox.ActorValue = null;
			this.actorDropbox.Location = new System.Drawing.Point(192, 16);
			this.actorDropbox.Name = "actorDropbox";
			this.actorDropbox.OwnerForm = null;
			this.actorDropbox.Size = new System.Drawing.Size(176, 48);
			this.actorDropbox.TabIndex = 1;
			this.actorDropbox.ValueNode = null;
			// 
			// classDropbox
			// 
			this.classDropbox._ActorName = "Class";
			this.classDropbox._ActorValue = null;
			this.classDropbox.ActorName = "Class";
			this.classDropbox.ActorValue = null;
			this.classDropbox.Location = new System.Drawing.Point(8, 16);
			this.classDropbox.Name = "classDropbox";
			this.classDropbox.OwnerForm = null;
			this.classDropbox.Size = new System.Drawing.Size(176, 48);
			this.classDropbox.TabIndex = 0;
			this.classDropbox.ValueNode = null;
			// 
			// ActorDualCombo
			// 
			this.Controls.Add(this.groupBox);
			this.Name = "ActorDualCombo";
			this.Size = new System.Drawing.Size(376, 72);
			this.groupBox.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion





//		public string ActorName{get{return this.groupBox.Text;}set{this.groupBox.Text = value;}}

		public override string _ActorName
		{
			get
			{
				return this.groupBox.Text;
			}
			set
			{
				this.groupBox.Text = value;
			}
		}
/*
		public string _ActorValue
		{
			get
			{
				// TODO:  Add ActorDualCombo.ActorValue getter implementation
				return null;
			}
			set
			{
				// TODO:  Add ActorDualCombo.ActorValue setter implementation
			}
		}
*/
	}
}
