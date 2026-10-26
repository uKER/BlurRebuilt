using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

#pragma warning disable 0618

namespace ProfilingApplication
{
	/// <summary>
	/// Summary description for ExpressionEvaluator.
	/// </summary>
	public class ExpressionEvaluator
	{
		public ExpressionEvaluator()
		{
			//
			// TODO: Add constructor logic here
			//
		}
		public static double Calculate(string expression)
		{
			Microsoft.CSharp.CSharpCodeProvider cp = new Microsoft.CSharp.CSharpCodeProvider();   
			System.CodeDom.Compiler.ICodeCompiler ic = cp.CreateCompiler();   
			System.CodeDom.Compiler.CompilerParameters cpar = new System.CodeDom.Compiler.CompilerParameters();   
			cpar.GenerateInMemory = true;   
			cpar.GenerateExecutable = false;   
			cpar.ReferencedAssemblies.Add("system.dll");
			string fileName = Process.GetCurrentProcess().MainModule.FileName.Replace(".vshost", ""); 
			cpar.ReferencedAssemblies.Add(fileName);
			MyClassBase myobj = null;
			string src = @"using System; 
                           class myclass:ProfilingApplication.MyClassBase {
								 private double %MT_VARIABLES%;
                                 public myclass()
                                 {
                                 }
                                 public override double eval()
                                 {
									return " + expression + @";
                                 }
                           }"; 
			string variables="ans=0";
			src=src.Replace(@"%MT_VARIABLES%",variables);
			System.CodeDom.Compiler.CompilerResults cr = ic.CompileAssemblyFromSource(cpar,src);   
			foreach (System.CodeDom.Compiler.CompilerError ce in cr.Errors)
			{
				throw new Exception(ce.ErrorText);
			}
			if (cr.Errors.Count == 0 && cr.CompiledAssembly != null)   
			{    
				Type ObjType = cr.CompiledAssembly.GetType("myclass");    
				try    
				{     
					if (ObjType != null)     
					{      
						myobj = (MyClassBase)Activator.CreateInstance(ObjType);     
					}    
				}    
				catch (Exception ex)    
				{     
					throw ex;
				}    
				return myobj.eval();   
			}   
			else
			{
				throw new Exception("Failed to compile");
			}
		}
		public static bool IsCompiled(string source)
		{
			Microsoft.CSharp.CSharpCodeProvider cp = new Microsoft.CSharp.CSharpCodeProvider();   
			System.CodeDom.Compiler.ICodeCompiler ic = cp.CreateCompiler();   
			System.CodeDom.Compiler.CompilerParameters cpar = new System.CodeDom.Compiler.CompilerParameters();   
			cpar.GenerateInMemory = true;   
			cpar.GenerateExecutable = false;   
			cpar.ReferencedAssemblies.Add("system.dll");   
			System.CodeDom.Compiler.CompilerResults cr = ic.CompileAssemblyFromSource(cpar,source);   
			foreach (System.CodeDom.Compiler.CompilerError ce in cr.Errors)
			{
				return false;
			}
			if (cr.Errors.Count == 0 && cr.CompiledAssembly != null)   
			{    
				return true;
			}   
			else
			{
				return false;
			}
		}
	}

	
	public class MyClassBase 
	{  
		public MyClassBase()  
		{  
		}  
		public virtual double eval()  
		{   
			return 0.0;  
		} 
	}

}
