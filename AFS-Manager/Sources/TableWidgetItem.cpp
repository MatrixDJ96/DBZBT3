#include <TableWidgetItem.h>

TableWidgetItem::TableWidgetItem(const QString &text, TableWidgetItem::Type type) : QTableWidgetItem(text), type(type)
{
}

bool TableWidgetItem::operator<(const QTableWidgetItem &other) const
{
	if (type == Type::Integer) {
		return text().toInt() < other.text().toInt();
	}
	else {
		return QTableWidgetItem::operator<(other);
	}
}
