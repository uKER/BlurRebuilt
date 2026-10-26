#pragma once

class GuiProgressCallback : public Aftk::I_Progress
{
public:
	void StartProgress()
	{
		if (MGlobal::mayaState() != MGlobal::kInteractive)
			return;

		MStatus status;
		bool reserved = MProgressWindow::reserve();
		status = MProgressWindow::setProgressRange(0,100);
		status = MProgressWindow::setInterruptable(false);
		MProgressWindow::setProgress(0);
		status = MProgressWindow::startProgress();
		MProgressWindow::setTitle("");
	}

	void EndProgress()
	{
		if (MGlobal::mayaState() != MGlobal::kInteractive)
			return;

		MProgressWindow::endProgress();
	}

	void SetTitle(const std::string& title)
	{
		if (MGlobal::mayaState() != MGlobal::kInteractive)
			return;

		MProgressWindow::setTitle(title.c_str());
	}

	//Returning true will eventually abort the current operation. For the moment this is however
	//not implemented.
	bool OnProgress(F32 percent)
	{
		if (MGlobal::mayaState() != MGlobal::kInteractive)
			return false;

		MProgressWindow::setProgress((int)percent);
		return false;
	}

	void OnPrintProgress(const std::string& text)
	{
		if (MGlobal::mayaState() != MGlobal::kInteractive)
			return;

		MProgressWindow::setTitle(text.c_str());
	}

private:
};

