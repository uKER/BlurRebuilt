using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FileTagManager3;
using Alienbrain;
using FileTagManager3.LoadOnDemand;
using DataLib.DataModel;
using System.Collections.ObjectModel;

namespace FileTagManager3.LoadOnDemand
{
	public class ProjectViewModel
	{
		readonly ReadOnlyCollection<AbDataViewModel> _children;

		public ProjectViewModel(List<AbData> projects)
		{
			List<AbDataViewModel> listDataView = new List<AbDataViewModel>(projects.Count);
			foreach(AbData abData in projects)
			{
				listDataView.Add(new AbDataViewModel(abData, null));
			}

			_children = new ReadOnlyCollection<AbDataViewModel>(listDataView);
		}

		public ReadOnlyCollection<AbDataViewModel> Children
		{
			get { return _children; }
		}
	}
}