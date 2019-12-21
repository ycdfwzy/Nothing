#pragma once

#include <QDialog>
#include <qdir.h>
#include "ui_DiskDialog.h"

class DiskDialog : public QDialog
{
	Q_OBJECT

public:
	DiskDialog(QWidget *parent = Q_NULLPTR, bool *disk_added = nullptr);
	~DiskDialog();

private:
	Ui::DiskDialog ui;
	QFileInfoList disk_list;

public slots:
	void accept_disks();
	void reject_disks();

signals:
	void addDisk(QList<QPair<int, QString>> disks);
};
