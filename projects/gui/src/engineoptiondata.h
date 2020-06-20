#pragma once

#include <QtCore>
#include <QList>
#include <QMap>

namespace Chess {

    enum OptionType
    {
        OPT_TYPE_CHECK,
        OPT_TYPE_SPIN,
        OPT_TYPE_COMBO,
        OPT_TYPE_BUTTON,
        OPT_TYPE_STRING
    };

    typedef QMap<QString, QVariant> OptionValueList;

    class EngineOptionData
    {
    public:
        EngineOptionData();

        QString m_name;
        QString m_defVal;
        QString m_minVal;
        QString m_maxVal;
        QStringList m_varVals;
        OptionType m_type;

        static EngineOptionData* FindInList(QString name, QList<EngineOptionData>& list);
    };

}
