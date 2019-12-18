#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_frontend.h"

class frontend : public QMainWindow
{
	Q_OBJECT

public:
	frontend(QWidget *parent = Q_NULLPTR);

private:
	Ui::frontendClass ui;
	
public slots:
	void setSearchPath();
	void startSearch();
};
