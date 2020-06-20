#pragma once

#include "enginedata.h"
namespace Chess {

    EngineData::EngineData(const QString& newName) : name(newName), logging(false), sendHistory(false), protocol(UCI)
    {
    }


    QString EngineData::commandToName(const QString& command)
    {
        QString name = command.section('/', -1, -1);
        if (!name.isEmpty())
        {
            name[0] = name[0].toUpper();
        }
        if (name.endsWith(".exe"))
        {
            name.truncate(name.length() - 4);
        }
        return name; 
    }


}