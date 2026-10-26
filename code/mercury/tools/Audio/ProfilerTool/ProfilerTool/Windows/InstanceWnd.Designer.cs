namespace ProfilerTool.Windows
{
    partial class InstanceWnd
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // InstanceWnd
            // 
            //this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            //this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(292, 266);
            this.Name = "InstanceWnd";
            this.Text = "Instance";
            //this.Activated += new System.EventHandler(this.Page_Activated);
            //this.Closing += new System.ComponentModel.CancelEventHandler(this.Page_Closing);
            this.Resize += new System.EventHandler(this.Page_Resized);
            this.ResumeLayout(false);

        }

        #endregion


        //private System.Windows.Forms.ToolStrip tbInstance;
    }
}