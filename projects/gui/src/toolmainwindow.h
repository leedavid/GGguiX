#pragma once


#include <QMainWindow>


class ToolMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ToolMainWindow(QWidget* parent = nullptr);

signals:

public slots:
    virtual void saveConfig();
    virtual void slotReconfigure();
};
