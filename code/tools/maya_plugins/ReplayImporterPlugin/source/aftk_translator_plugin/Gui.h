#pragma once

void GuiDisplayOutput(DebugLevel::Level channel, const std::string& text);

void GuiDisplayMessageBox(const std::string& title, const std::string& message);

void GuiDisplayExportSummary();

void GuiDisplayEmailPrompt();

void GuiDebugMessage(const std::string& text);

void GuiResetCounts();