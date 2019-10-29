#include <TableWidgetItem.h>
#include <Shared.h>

using namespace Shared;

TableWidgetItem::TableWidgetItem(const QString &text, TableWidgetItem::Type type) : QTableWidgetItem(text), type(type)
{
}

bool TableWidgetItem::operator<(const QTableWidgetItem &other) const
{
	if (type == Type::Integer) {
		return getSize(text().toLocal8Bit().toStdString()) < getSize(other.text().toLocal8Bit().toStdString());
	}
	else {
		return QTableWidgetItem::operator<(other);
	}
}
