#pragma once

#include <QDockWidget>

class DockWidgetEx : public QDockWidget
{
    Q_OBJECT
public:
    explicit DockWidgetEx(const QString& title, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

signals:

public slots:

protected:
    void showEvent(QShowEvent* event);
};

