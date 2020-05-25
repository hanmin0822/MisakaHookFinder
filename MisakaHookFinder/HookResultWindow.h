#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HookResultWindow.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "texthost.h"

class HookResultWindow : public QWidget
{
	Q_OBJECT

public:
	HookResultWindow(QWidget *parent = Q_NULLPTR);
	~HookResultWindow();

public:
	DWORD processID;

private:
	Ui::HookResultWindow ui;

public slots:
	void FindNextBtn_Click();
	void AddCustomBtn_Click();
};
