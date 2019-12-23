#pragma once

#include <QTableWidgetItem>

class QIntTableWidgetItem : public QTableWidgetItem
{

public:
	QIntTableWidgetItem (QString str):QTableWidgetItem(str){ }

	bool QIntTableWidgetItem::operator< (const QTableWidgetItem& other) const {
		return (this->text().toInt() < other.text().toInt());
	}

};
