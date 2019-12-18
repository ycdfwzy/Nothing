#include "frontend.h"
#include <qwidgetaction.h>
#include <qfiledialog.h>
#include <qtablewidget.h>

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
	
	connect(ui.path_btn, SIGNAL(clicked()), this, SLOT(setSearchPath()));

	// test
	ui.tableWidget->insertRow(0);
	ui.tableWidget->setItem(0, 0, new QTableWidgetItem(QString("Rua")));
}

void frontend::setSearchPath() {
	QString path = QFileDialog::getExistingDirectory(this, "请选择要查询的文件夹", "./");
	ui.lookingEdit->setText(path);
}

void frontend::startSearch() {
	QString keyword = ui.filenameEdit->text();
	if (ui.checkBox->isChecked()) {
		QString content = ui.containingEdit->text();
		QString path = ui.lookingEdit->text();
	}
	else {
		QString path = ui.lookingEdit->text();
	}
}
