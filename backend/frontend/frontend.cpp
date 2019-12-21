#include "frontend.h"
#include <qwidgetaction.h>
#include <qfiledialog.h>
#include <qtablewidget.h>
#include <qdir.h>
#include "GeneralManager.h"
#include "DiskDialog.h"
#include <qdebug.h>
#include <vector>
#include "SearchResult.h"

frontend::frontend(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// Table init
	ui.tableWidget->setColumnWidth(0, 50);
	ui.tableWidget->setColumnWidth(1, 150);
	ui.tableWidget->setColumnWidth(2, 400);
	ui.tableWidget->setColumnWidth(3, 100);
	ui.tableWidget->setColumnWidth(4, 100);

	// status bar init
	status_label = new QLabel();
	status_label->setMinimumSize(150, 20);
	ui.statusBar->addWidget(status_label);
	
	connect(ui.path_btn, SIGNAL(clicked()), this, SLOT(setSearchPath()));
	connect(ui.actionadd_disk, SIGNAL(triggered()), this, SLOT(addDiskDialog()));
	connect(ui.start_btn, SIGNAL(clicked()), this, SLOT(startSearch()));
	connect(ui.stop_btn, SIGNAL(clicked()), this, SLOT(stopSearch()));

	// test
	// ui.tableWidget->insertRow(0);
	// ui.tableWidget->setItem(0, 0, new QTableWidgetItem(QString("Rua")));

	// Search Tool Init
	manager = Nothing::GeneralManager::getInstance();
	int disk_num = QDir::drives().size();
	disk_added = new bool[disk_num];
	for (int i = 0; i < disk_num; i++) disk_added[i] = false;
	
	status_label->setText(tr("Ready!"));
}

void frontend::setSearchPath() {
	QString path = QFileDialog::getExistingDirectory(this, QString("Select search directory"), "./");
	ui.lookingEdit->setText(path);
}

void frontend::startSearch() {
	QString keyword = ui.filenameEdit->text();
	QString content;
	QString path = ui.lookingEdit->text();
	bool with_content = ui.checkBox->isChecked();
	if (with_content) QString content = ui.containingEdit->text();

	qDebug() << "search keyword: " << keyword << ", content: " << content << ", in: " << path;
	updateStatus(3);

	std::vector<Nothing::SearchResult> res;
	res.clear();
	Nothing::Result search_res = manager->search(res, keyword.toStdWString(), content.toStdWString(), path.toStdWString());
	
	if (search_res != Nothing::Result::SUCCESS) {
		updateStatus(4);
		return;
	}
	for (auto& p : res) qDebug() << "found: " << p.get_path();
}

void frontend::stopSearch() {

}

void frontend::addDiskDialog() {
	DiskDialog *d = new DiskDialog(this, this->disk_added);
	d->setModal(true);
	connect(d, SIGNAL(addDisk(QList<QPair<int, QString>>)), this, SLOT(addDisks(QList<QPair<int, QString>>)));
	d->show();
	d->exec();
}

void frontend::addDisks(QList<QPair<int, QString>> disks) {
	updateStatus(1);
	for (auto i = disks.begin(); i != disks.end(); i++) {
		this->disk_added[i->first] = true;
		qDebug() << "addDisk " << i->first << " " << i->second;

		manager->addDisk(i->second.toStdString()[0]);
	}
	updateStatus(2);
}

void frontend::updateStatus(int status) {
	switch (status)
	{
	case 1:
		status_label->setText(tr("Pending"));
		break;
	case 2:
		status_label->setText(tr("Ready!"));
		break;
	case 3:
		status_label->setText(tr("Searching..."));
		break;
	case 4:
		status_label->setText(tr("Wrong Input, Please Check."));
		break;
	default:
		status_label->setText(tr("Ready!"));
		break;
	}
	status_label->repaint();
	qApp->processEvents();
}

