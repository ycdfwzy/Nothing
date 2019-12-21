#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_frontend.h"

#ifdef _DEBUG
#undef _DEBUG
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
#define _DEBUG
#else
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
#endif

#include "GeneralManager.h"
#include <qdir.h>

class frontend : public QMainWindow
{
	Q_OBJECT

public:
	frontend(QWidget *parent = Q_NULLPTR);

private:
	Ui::frontendClass ui;
	QLabel *status_label;

	Nothing::GeneralManager* manager;
	bool* disk_added;
	
public slots:
	void setSearchPath();
	void startSearch();
	void stopSearch();
	void addDisks(QList<QPair<int, QString>> disks);
	void addDiskDialog();
	void updateStatus(int status);
};
