#include "addstreamdialog.h"
#include "ui_addstreamdialog.h"
#include <iostream>

AddStreamDialog::AddStreamDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddStreamDialog)
{
    ui->setupUi(this);
}

AddStreamDialog::~AddStreamDialog()
{
    delete ui;
}

void
AddStreamDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->button(QDialogButtonBox::Ok) == button )
    {
        if(ui->name_edit->text().isEmpty())
        {
            ui->name_edit->setStyleSheet("background-color:rgb(255,0,0)");
            return;
        }
        else if(ui->face_edit->text().isEmpty())
        {
            ui->face_edit->setStyleSheet("background-color:rgb(255,0,0)");
            return;
        }
        else
        {
            QString *stream = new QString(ui->name_edit->text());
            stream->append(":");
            stream->append(new QString(ui->face_edit->text()));

            //std::cout << stream->toStdString() <<std::endl;

            emit addedStream(*stream);
        }
    }
    else
    {
        return;
    }
}
