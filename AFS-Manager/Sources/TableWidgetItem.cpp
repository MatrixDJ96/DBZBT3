#include <TableWidgetItem.h>
#include <Shared.h>

using namespace Shared;

TableWidgetItem::TableWidgetItem(const QString &text, TableWidgetItem::Type type) : QTableWidgetItem(text), type(type)
{
}

bool TableWidgetItem::operator<(const QTableWidgetItem &other) const
{
	if (type == Type::Integer) {
		return getSize(text().toStdString()) < getSize(other.text().toStdString());
	}
	else {
		return QTableWidgetItem::operator<(other);
	}
}
