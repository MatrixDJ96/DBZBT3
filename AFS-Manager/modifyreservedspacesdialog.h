#ifndef MODIFYRESERVEDSPACESDIALOG_H
#define MODIFYRESERVEDSPACESDIALOG_H

#include <cinttypes>
#include <QDialog>

namespace Ui {
class ModifyReservedSpacesDialog;
}

class ModifyReservedSpacesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyReservedSpacesDialog(const uint32_t previous, const uint32_t following, QWidget *parent = 0);
    ~ModifyReservedSpacesDialog();

public:
    uint32_t spaceFromPrevious;
    uint32_t spaceFromFollowing;

private slots:
    void on_pushButtonCancel_clicked();
    void on_pushButtonSave_clicked();
    void on_spinBoxSpacePrevious_editingFinished();
    void on_spinBoxSpaceFollowing_editingFinished();

private:
    Ui::ModifyReservedSpacesDialog *ui;
};

#endif // MODIFYRESERVEDSPACESDIALOG_H
