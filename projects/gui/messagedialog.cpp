#include "messagedialog.h"
#include <QtGui>
#include <QMessageBox>
#include <QApplication>

namespace Chess {


    void MessageDialog::error(const QString& text, const QString& title)
    {
        QMessageBox::critical(QApplication::activeWindow(), title, text);
    }

    void MessageDialog::warning(const QString& text, const QString& title)
    {
        QMessageBox::warning(QApplication::activeWindow(), title, text);
    }

    void MessageDialog::information(const QString& text, const QString& title)
    {
        QMessageBox::information(QApplication::activeWindow(), title, text);
    }

    bool MessageDialog::yesNo(const QString& text, const QString& title, const QString& yes,
        const QString& no)
    {
        QMessageBox mb(QApplication::activeWindow());
        mb.QDialog::setWindowTitle(title);
        mb.setText(text);
        mb.setIcon(QMessageBox::Question);
        QPushButton* y = mb.addButton(yes, QMessageBox::YesRole);
        mb.setDefaultButton(y);
        mb.addButton(no, QMessageBox::NoRole);
        mb.exec();
        return mb.clickedButton() == (QAbstractButton*)y;
    }

    bool MessageDialog::okCancel(const QString& text, const QString& title, const QString& ok,
        const QString& cancel)
    {
        QMessageBox mb(QApplication::activeWindow());
        mb.QDialog::setWindowTitle(title);
        mb.setText(text);
        mb.setIcon(QMessageBox::Question);
        QPushButton* o = mb.addButton(ok, QMessageBox::ActionRole);
        mb.setDefaultButton(o);
        QPushButton* c = mb.addButton(cancel, QMessageBox::RejectRole);
        mb.setEscapeButton((QAbstractButton*)c);
        mb.exec();
        return mb.clickedButton() == (QAbstractButton*)o;
    }

    int MessageDialog::yesNoCancel(const QString& text, const QString& title, const QString& yes,
        const QString& no, const QString& cancel)
    {
        QMessageBox mb(QApplication::activeWindow());
        mb.QDialog::setWindowTitle(title);
        mb.setText(text);
        mb.setIcon(QMessageBox::Question);
        QPushButton* y = mb.addButton(yes, QMessageBox::ActionRole);
        mb.setDefaultButton(y);
        QPushButton* n = mb.addButton(no, QMessageBox::NoRole);
        QPushButton* c = mb.addButton(cancel, QMessageBox::RejectRole);
        mb.setEscapeButton((QAbstractButton*)c);
        mb.exec();
        if (mb.clickedButton() == (QAbstractButton*)y)
        {
            return Yes;
        }
        else if (mb.clickedButton() == (QAbstractButton*)n)
        {
            return No;
        }
        else
        {
            return Cancel;
        }
    }
}