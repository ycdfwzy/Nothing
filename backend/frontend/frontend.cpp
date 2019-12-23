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
#include <qdatetime.h>
#include <qfileiconprovider.h>
#include "FileContent.h"
#include "ContentFileSearch.h"
#include <QMetaType>
#include "QIntTableWidgetItem.h"

frontend::frontend(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.stop_btn->setEnabled(false);

	// Table init
	ui.tableWidget->setColumnWidth(0, 50);
	ui.tableWidget->setColumnWidth(1, 150);
	ui.tableWidget->setColumnWidth(2, 400);
	ui.tableWidget->setColumnWidth(3, 100);
	ui.tableWidget->setColumnWidth(4, 200);
	ui.tableWidget->setColumnWidth(5, 50);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// signal connection
	connect(ui.tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateHits()));
	connect(ui.path_btn, SIGNAL(clicked()), this, SLOT(setSearchPath()));
	connect(ui.actionadd_disk, SIGNAL(triggered()), this, SLOT(addDiskDialog()));
	connect(ui.start_btn, SIGNAL(clicked()), this, SLOT(startSearch()));
	connect(ui.stop_btn, SIGNAL(clicked()), this, SLOT(stopSearch()));

	// Search Tool Init
	manager = Nothing::GeneralManager::getInstance();
	int disk_num = QDir::drives().size();
	disk_added = new bool[disk_num];
	for (int i = 0; i < disk_num; i++) disk_added[i] = false;
	is_searching_content = false;

	// status bar init
	status_label = new QLabel();
	status_label->setMinimumSize(150, 20);
	ui.statusBar->addWidget(status_label);
	status_label->setText(tr("Ready!"));
}

void frontend::setSearchPath() {
	QString path = QFileDialog::getExistingDirectory(this, QString("Select search directory"), "./");
	ui.lookingEdit->setText(path);
}

void frontend::startSearch() {
	// Ui Clear
	ui.tableWidget->clearContents();
	ui.tableWidget->setRowCount(0);
	ui.tableWidget->setSortingEnabled(false);
	ui.summary_browser->clear();
	ui.hits_browser->clear();

	QString q_keyword = ui.filenameEdit->text();
	QString q_content = ui.containingEdit->text();
	QString q_path = ui.lookingEdit->text().replace('/', '\\');
	std::wstring keyword = q_keyword.toStdWString();
	std::wstring content = q_content.toStdWString();
	std::wstring path = q_path.toStdWString();

	bool with_content = ui.checkBox->isChecked() && content != L"";

	// qDebug() << "search keyword: " << q_keyword << ", content: " << q_content << ", in: " << q_path;
	updateStatus(3);

	result_list.clear();
	Nothing::Result search_res;
	if (with_content) search_res = manager->search(result_list, keyword, content, path);
	else search_res = manager->search(result_list, keyword, L"", path);
	
	if (search_res != Nothing::Result::SUCCESS) {
		// search failure
		updateStatus(4);
		return;
	}

	// success search
	if (!with_content) {
		// show file info in table
		for (int i = 0; i < result_list.size(); i++)
			addFileToTable(QFileInfo(QString::fromStdWString(result_list[i].get_path())), i, 0);
		ui.tableWidget->setSortingEnabled(true);
		ui.tableWidget->sortItems(5, Qt::AscendingOrder);
		updateSummary();
		updateStatus(2);
	}
	else {
		// content search
		is_searching_content = true;
		ui.stop_btn->setEnabled(true);
		ui.start_btn->setEnabled(false);
		ui.actionadd_disk->setEnabled(false);
		ui.path_btn->setEnabled(false);
		
		// one by one search on new thread
		search_thread = new ContentFileSearch(this, manager->getContentSearch(), keyword, content);
		connect(search_thread, SIGNAL(foundFile(Nothing::SearchResult)), this, SLOT(foundFile(Nothing::SearchResult)));
		connect(search_thread, SIGNAL(finished()), this, SLOT(stopSearch()));
		search_thread->start();
	}
}

void frontend::stopSearch() {
	// only able to stop on content search
	if (!is_searching_content) return;
	search_thread->searching = false;
	is_searching_content = false;
	search_thread->wait();

	// restore ui
	ui.start_btn->setEnabled(true);
	ui.actionadd_disk->setEnabled(true);
	ui.stop_btn->setEnabled(false);
	ui.path_btn->setEnabled(true);
	ui.tableWidget->setSortingEnabled(true);
	ui.tableWidget->sortItems(5, Qt::AscendingOrder);

	// update status
	updateSummary();
	updateHits();
	updateStatus(2);
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
		// qDebug() << "addDisk " << i->first << " " << i->second;

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

void frontend::updateSummary() {
	long long total_size = 0;
	int folder_num = 0;
	int file_num = 0;
	int largest_size = 0;
	int smallest_size = INT_MAX;
	QDateTime oldest_mod = QDateTime::currentDateTime();
	QDateTime newest_mod = QDateTime::fromMSecsSinceEpoch(0); 
	QDateTime oldest_create = QDateTime::currentDateTime();
	QDateTime newest_create = QDateTime::fromMSecsSinceEpoch(0); 
	QDateTime oldest_access = QDateTime::currentDateTime();
	QDateTime newest_access = QDateTime::fromMSecsSinceEpoch(0);

	for (auto i = result_list.begin(); i != result_list.end(); i++) {
		QFileInfo info(QString::fromStdWString(i->get_path()));
		total_size += info.size();
		if (info.isDir()) folder_num++;
		if (info.isFile()) file_num++;
		if (info.size() > largest_size) largest_size = info.size();
		if (info.size() > 0 && info.size() < smallest_size) smallest_size = info.size();
		if (info.lastModified().isValid()) {
			if (info.lastModified() > newest_mod) newest_mod = info.lastModified();
			if (info.lastModified() < oldest_mod) oldest_mod = info.lastModified();
		}
		if (info.created().isValid()) {
			if (info.created() > newest_create) newest_create = info.created();
			if (info.created() < oldest_create) oldest_create = info.created();
		}
		if (info.lastRead().isValid()) {
			if (info.lastRead() > newest_access) newest_access = info.lastRead();
			if (info.lastRead() < oldest_access) oldest_access = info.lastRead();
		}
	}

	ui.summary_browser->insertPlainText("Finished!\n\n");
	ui.summary_browser->insertPlainText(QStringLiteral("Matched: %1 files\n").arg(result_list.size()));
	ui.summary_browser->insertPlainText(QStringLiteral("Size on Disk: %1 KBytes\n").arg(total_size / 1000));
	ui.summary_browser->insertPlainText(QStringLiteral("Folder Num: %1\n").arg(folder_num));
	ui.summary_browser->insertPlainText(QStringLiteral("File Num: %1\n\n").arg(file_num));
	if (result_list.size() == 0) return;
	ui.summary_browser->insertPlainText(QStringLiteral("Largest File Size: %1 Bytes\n").arg(largest_size));
	ui.summary_browser->insertPlainText(QStringLiteral("Smallest File Size£º %1 Bytes\n").arg(smallest_size));
	ui.summary_browser->insertPlainText(QStringLiteral("Oldest modified file: %1\n").arg(oldest_mod.toString(Qt::ISODateWithMs)));
	ui.summary_browser->insertPlainText(QStringLiteral("Newest modified file: %1\n").arg(newest_mod.toString(Qt::ISODateWithMs)));
	ui.summary_browser->insertPlainText(QStringLiteral("Oldest file was created: %1\n").arg(oldest_create.toString(Qt::ISODateWithMs)));
	ui.summary_browser->insertPlainText(QStringLiteral("Newest file was created: %1\n").arg(newest_create.toString(Qt::ISODateWithMs)));
	ui.summary_browser->insertPlainText(QStringLiteral("Oldest file was accessed: %1\n").arg(oldest_access.toString(Qt::ISODateWithMs)));
	ui.summary_browser->insertPlainText(QStringLiteral("Newest file was accessed: %1\n").arg(newest_access.toString(Qt::ISODateWithMs)));
}

void frontend::updateHits()
{
	ui.hits_browser->clear();
	QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems();
	int cnt = items.count();
	for (int i = 0; i < cnt; i++) if (items.at(i)->column() == 0){
		int index = items.at(i)->data(Qt::UserRole).toInt();
		ui.hits_browser->insertHtml("<B>" + QString::fromStdWString(result_list[index].get_path()) + "</B>");
		ui.hits_browser->insertPlainText("\n");
		int hit_cnt = 1;
		QString q_content = QString::fromStdWString(result_list[index].get_content());
		auto hit_list = result_list[index].get_content_results();
		for (auto p = hit_list.begin(); p != hit_list.end(); p++) {
			ui.hits_browser->insertHtml("<span>" + QStringLiteral("hits %1: ").arg(hit_cnt) + "</span>");
			ui.hits_browser->insertHtml("<span>" + QString::fromStdWString(p->first).replace('\n', "\\n") + "<span>");
			ui.hits_browser->insertHtml("<FONT style=\"BACKGROUND-COLOR: green\">" + q_content + "</FONT>");
			ui.hits_browser->insertHtml("<span>" + QString::fromStdWString(p->second).replace('\n', "\\n ") + "<span>");
			ui.hits_browser->insertPlainText("\n");
			hit_cnt++;
		}
		ui.hits_browser->insertPlainText("\n");
	}
}

void frontend::addFileToTable(QFileInfo& info, int index, int hit_num) {
	int row_cnt = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(row_cnt);
	QFileIconProvider provider;
	ui.tableWidget->setItem(row_cnt, 0, new QTableWidgetItem(provider.icon(info), ""));
	ui.tableWidget->item(row_cnt, 0)->setTextAlignment(Qt::AlignCenter);
	ui.tableWidget->setItem(row_cnt, 1, new QTableWidgetItem(info.fileName()));
	ui.tableWidget->setItem(row_cnt, 2, new QTableWidgetItem(info.absolutePath()));
	ui.tableWidget->setItem(row_cnt, 3, new QTableWidgetItem(QStringLiteral("%1 Bytes").arg(info.size())));
	ui.tableWidget->setItem(row_cnt, 4, new QTableWidgetItem(info.lastModified().toString(Qt::TextDate)));
	ui.tableWidget->setItem(row_cnt, 5, new QIntTableWidgetItem(QString::number(hit_num)));
	// set index
	ui.tableWidget->item(row_cnt, 0)->setData(Qt::UserRole, index);
}

void frontend::foundFile(Nothing::SearchResult tmp_res) {
	// qDebug() << "in frontend found file: " << tmp_res.get_name();
	addFileToTable(QFileInfo(QString::fromStdWString(tmp_res.get_path())),
		result_list.size(), tmp_res.get_content_results().size());
	result_list.push_back(tmp_res);
}