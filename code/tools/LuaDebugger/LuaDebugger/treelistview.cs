using System;
using System.Windows.Controls;
using System.Windows;


namespace LuaDebugger
{
    public class TreeListView : TreeView
    {
        protected override DependencyObject GetContainerForItemOverride()
        {
            return new VariableInfo();
        }

        protected override bool IsItemItsOwnContainerOverride(object item)
        {
            return item is VariableInfo;
        }
    }

    public class VariableInfo : TreeViewItem
    {
        public VariableInfo()
        {
            Header = "";
        }

        /// <summary>
        /// Item's hierarchy in the tree
        /// </summary>
        public int Level
        {
            get
            {
                if (_level == -1)
                {
                    VariableInfo parent = ItemsControl.ItemsControlFromItemContainer(this) as VariableInfo;
                    
                    _level = (parent != null) ? parent.Level + 1 : 0;
                }
                return _level;
            }
        }

        protected override DependencyObject GetContainerForItemOverride()
        {
            return new VariableInfo();
        }

        protected override bool IsItemItsOwnContainerOverride(object item)
        {
            return item is VariableInfo;
        }

        private int _level = -1;

        public string VariableName { get; set; }
        public string Type { get; set; }
        public string Value { get; set; }
    }

}
