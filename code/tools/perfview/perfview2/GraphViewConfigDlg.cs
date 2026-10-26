using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ProfilingApplication
{
	public partial class GraphViewConfigDlg : Form
	{
		void SetSettings(GraphViewConfig settings)
		{
			bgDarkColourButton.CenterColor = ColorFromColour(settings.bgDark);
			bgLightColourButton.CenterColor = ColorFromColour(settings.bgLight);
			gridDarkColourButton.CenterColor = ColorFromColour(settings.gridDark);
			gridLightColourButton.CenterColor = ColorFromColour(settings.gridLight);

			gridSpacingNumeric.Value = settings.iGridSpacing;
			highlightEveryNumeric.Value = settings.iHighlightEvery;
			minNumeric.Value = (decimal)settings.fMin;
			maxNumeric.Value = (decimal)settings.fMax;
			showGridCheck.CheckState = settings.bGridOn?CheckState.Checked:CheckState.Unchecked;
			autoScaleCheck.CheckState = settings.bAutoScale?CheckState.Checked:CheckState.Unchecked;
			multiGraphCheck.CheckState = settings.bMultipleGraphs?CheckState.Checked:CheckState.Unchecked;
			updatesPerSecNumeric.Value = (decimal)settings.iUpdatesPerSec;
			numValuesNumeric.Value = (decimal)settings.iMaxValues;
			marginCheckBox.Checked = settings.bMargin;

			graphChartPreview.SetSettings(settings);
			ProcessValueChanged();
		}

		public GraphViewConfigDlg(GraphViewConfig settings)
		{
			InitializeComponent();

			SetSettings(settings);
		}

		private Color ColorFromColour(Colour colour)
		{
			return Color.FromArgb(255,colour.R, colour.G, colour.B);
		}

		private Colour ColourFromColor(Color color)
		{
			return new ProfilingApplication.Colour(color.R, color.G, color.B);
		}

		public GraphViewConfig GetSettings()
		{
			GraphViewConfig settings = new GraphViewConfig();

			settings.bgDark = ColourFromColor(bgDarkColourButton.CenterColor);
			settings.bgLight = ColourFromColor(bgLightColourButton.CenterColor);
			settings.gridDark = ColourFromColor(gridDarkColourButton.CenterColor);
			settings.gridLight = ColourFromColor(gridLightColourButton.CenterColor);

			settings.iGridSpacing = (int)gridSpacingNumeric.Value;
			settings.iHighlightEvery = (int)highlightEveryNumeric.Value;
			settings.fMin = (float)minNumeric.Value;
			settings.fMax = (float)maxNumeric.Value;
			settings.bGridOn = showGridCheck.CheckState == CheckState.Checked;
			settings.bAutoScale = autoScaleCheck.CheckState == CheckState.Checked;
			settings.bMultipleGraphs = multiGraphCheck.CheckState == CheckState.Checked;
			settings.iUpdatesPerSec = (int)updatesPerSecNumeric.Value;
			settings.iMaxValues = (int)numValuesNumeric.Value;
			settings.bMargin = marginCheckBox.Checked;

			return settings;
		}

		private void resetButton_Click(object sender, EventArgs e)
		{
			SetSettings( new GraphViewConfig() );
		}

		// if any of the parameters in the dialog box change, update the grid preview
		private void ProcessValueChanged()
		{
			graphChartPreview.SetSettings(GetSettings());

			gridSpacingNumeric.Enabled = showGridCheck.Checked;
			highlightEveryNumeric.Enabled = showGridCheck.Checked;
			gridDarkColourButton.Enabled = showGridCheck.Checked;
			gridLightColourButton.Enabled = showGridCheck.Checked;

			minNumeric.Enabled = !autoScaleCheck.Checked;
			maxNumeric.Enabled = !autoScaleCheck.Checked;
		}

		private void valueChanged(object sender, EventArgs e)
		{
			ProcessValueChanged();
		}

		private void colourButton_Click(object sender, EventArgs e)
		{
			ColorButton button = (ColorButton)sender;

			Point p = new Point(groupBox1.Left + button.Left, groupBox1.Top + button.Top + button.Height);
			p = PointToScreen( p);
			    		
			ColorPaletteDlg clDlg = new ColorPaletteDlg(p.X, p.Y);

			clDlg.ShowDialog();
			
			if(clDlg.DialogResult == DialogResult.OK)
				button.CenterColor = clDlg.Color;
			
			Invalidate();
			
			clDlg.Dispose();

			base.OnClick(e);

			ProcessValueChanged();
		}
	}
}
