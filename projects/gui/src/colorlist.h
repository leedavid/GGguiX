#pragma once

#include <QListWidget>


namespace Chess {
    class ColorList : public QListWidget
    {
        Q_OBJECT
    public:
        /** Standard constructor. */
        ColorList(QWidget* parent);
        /** Standard constructor. */
        virtual ~ColorList();
        /** @return color of given item. */
        QColor color(QListWidgetItem*) const;
        /** @return color of given item. */
        QColor color(int index) const;
        /** Create new item. */
        virtual void addItem(const QString& text, const QColor& color);
    protected:
        /** Edit color on left click. */
        virtual void mousePressEvent(QMouseEvent* event);
        /** Edit color on Enter. */
        virtual void keyPressEvent(QKeyEvent* event);
        /** Set item color. */
        void setItemColor(QListWidgetItem* item, const QColor& color);
        /** Edit item color. */
        void editItemColor(QListWidgetItem*);
    };
}