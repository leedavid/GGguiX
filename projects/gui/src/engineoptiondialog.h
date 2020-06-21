#pragma once

#include <QDialog>

#include "engineoptiondata.h"
#include "enginelist.h"


class QAbstractButton;

namespace Ui
{
    class EngineOptionDialog;
}

namespace Chess {

    class Engine;

   

    class EngineOptionDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit EngineOptionDialog(QWidget* parent, EngineList& engineList, int index);
        ~EngineOptionDialog();
        OptionValueList GetResults() const;

    public slots:
        void accept();
        void reject();

    protected slots:
        void engineActivated();
        void optionReset();
        void restoreLayout();

        void buttonClicked(QAbstractButton* button);
    private:
        Ui::EngineOptionDialog* ui;
        Engine* m_engine;
        int m_index;
    };


}