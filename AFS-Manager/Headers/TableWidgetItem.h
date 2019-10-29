#ifndef TABLEWIDGETITEM_H
#define TABLEWIDGETITEM_H

#include <QTableWidgetItem>

class TableWidgetItem : public QTableWidgetItem
{
public:
	enum class Type
	{
		Integer, String
	} type;

	TableWidgetItem(const QString &text, Type type = Type::String);

	bool operator<(const QTableWidgetItem &other) const override;
};

#endif //TABLEWIDGETITEM_H
