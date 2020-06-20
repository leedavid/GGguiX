#pragma once

#include <QtCore>
#include "engineoptiondata.h"

namespace Chess {

    /** The EngineData class contains configuration of a single engine. */
    class EngineData
    {
    public:
        enum EngineProtocol { WinBoard, UCI };
        /** Standard constructor. */
        EngineData(const QString& newName = "");
        QString name;
        QString command;
        QString options;
        QString directory;
        bool    logging;
        bool    sendHistory;
        EngineProtocol protocol;
        OptionValueList m_optionValues;
        static QString commandToName(const QString& command);
    };

}