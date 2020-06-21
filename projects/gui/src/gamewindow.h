#pragma once

#include "game.h"
#include "nag.h"
#include "toolmainwindow.h"




namespace Ui {
    class GameWindow;
}

namespace Chess {

    class ChessBrowser;

    class GameWindow : public ToolMainWindow
    {
        Q_OBJECT

    public:
        explicit GameWindow(QWidget* parent = nullptr);
        ~GameWindow();

        ChessBrowser* browser();

    public slots:
        void saveConfig();
        void slotReconfigure();

    protected:
        void setupSpacers();

    private:
        Ui::GameWindow* ui;
        void setupToolBox();
        void setupNagInToolBox(Nag nag, QAction* action);
        void addActionAtPage(int page, QAction* action);
    };
}


