namespace MenuSystemLibrary
{
    partial class MenuSystem
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
			this.rootMenuContainer = new MenuSystemLibrary.ContainerRoot();
			this.directorySearcher1 = new System.DirectoryServices.DirectorySearcher();
			this.SuspendLayout();
			// 
			// rootMenuContainer
			// 
			this.rootMenuContainer.AutoScroll = true;
			this.rootMenuContainer.BackColor = System.Drawing.SystemColors.Control;
			this.rootMenuContainer.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
			this.rootMenuContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.rootMenuContainer.Location = new System.Drawing.Point(0, 0);
			this.rootMenuContainer.Name = "rootMenuContainer";
			this.rootMenuContainer.Size = new System.Drawing.Size(615, 425);
			this.rootMenuContainer.TabIndex = 0;
			// 
			// directorySearcher1
			// 
			this.directorySearcher1.ClientTimeout = System.TimeSpan.Parse("-00:00:01");
			this.directorySearcher1.ServerPageTimeLimit = System.TimeSpan.Parse("-00:00:01");
			this.directorySearcher1.ServerTimeLimit = System.TimeSpan.Parse("-00:00:01");
			// 
			// MenuSystem
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.AutoValidate = System.Windows.Forms.AutoValidate.Disable;
			this.BackColor = System.Drawing.SystemColors.Control;
			this.CausesValidation = false;
			this.Controls.Add(this.rootMenuContainer);
			this.Name = "MenuSystem";
			this.Size = new System.Drawing.Size(615, 425);
			this.ResumeLayout(false);

        }
        #endregion

		//		public panel_derived rootMenuContainer;
		//public panel_derived rootMenuContainer;
		public ContainerRoot rootMenuContainer;
		private System.DirectoryServices.DirectorySearcher directorySearcher1;

    }
}
