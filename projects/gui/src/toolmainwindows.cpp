#include "settings.h"
#include "toolmainwindow.h"
#include <QtCore>


ToolMainWindow::ToolMainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    setWindowFlags(Qt::Widget);
}

void ToolMainWindow::saveConfig()
{
    Chess::AppSettings->setLayout(this);
}

void ToolMainWindow::slotReconfigure()
{
    Chess::AppSettings->layout(this);
}
