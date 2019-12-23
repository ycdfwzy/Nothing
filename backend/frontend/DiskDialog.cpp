#include "DiskDialog.h"
#include <qdir.h>

DiskDialog::DiskDialog(QWidget *parent, bool* disk_added)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.accept_btn, SIGNAL(clicked()), this, SLOT(accept_disks()));
	connect(ui.reject_btn, SIGNAL(clicked()), this, SLOT(reject_disks()));
	
	disk_list = QDir::drives();
	if (disk_added) {
		for (int i = 0; i < disk_list.size(); i++) if (!disk_added[i]) {
			QListWidgetItem* item = new QListWidgetItem(disk_list[i].absoluteFilePath(), ui.listWidget);
			item->setData(Qt::UserRole, i);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Unchecked);
		}
	}
}

DiskDialog::~DiskDialog()
{
}

void DiskDialog::accept_disks() {
	QList<QPair<int, QString>> disks;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		if (ui.listWidget->item(i)->checkState() == Qt::Checked)
			disks.append(QPair<int, QString>(ui.listWidget->item(i)->data(Qt::UserRole).toInt(), ui.listWidget->item(i)->text()));
	}
	this->accept();
	emit(addDisk(disks));
}

void DiskDialog::reject_disks() {
	this->reject();
}
