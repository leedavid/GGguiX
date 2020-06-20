#pragma once

#include <QtCore>
#include "enginedata.h"

namespace Chess {

    /** The EngineList class contains configuration of all installed engines. */
    class EngineList : public QList<EngineData>
    {
    public:
        /** Standard constructor. */
        EngineList();
        /** Restore configuration from INI file. */
        void restore();
        /** Store configuration in INI file. */
        void save();
        /** Retrieve list of engine names in current order. */
        QStringList names() const;

    protected:
        /** Set the content of the default engine directories into the list of engines */
        void restoreEmpty();
        /** Set the content of the given directory into the list of engines */
        void restoreEmptyFromPath(const QString& path, EngineData::EngineProtocol protocol);
    };

}
