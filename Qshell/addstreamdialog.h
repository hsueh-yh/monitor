#ifndef ADDSTREAMDIALOG_H
#define ADDSTREAMDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QPushButton>

namespace Ui {
class AddStreamDialog;
}

class AddStreamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddStreamDialog(QWidget *parent = 0);
    ~AddStreamDialog();

signals:
    void addedStream(std::string stream);

private:
    Ui::AddStreamDialog *ui;

private slots:

    void on_buttonBox_clicked(QAbstractButton *button);
};

#endif // ADDSTREAMDIALOG_H
