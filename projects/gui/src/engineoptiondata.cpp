#include "engineoptiondata.h"


namespace Chess {

    EngineOptionData::EngineOptionData() : m_type(OPT_TYPE_STRING)
    {
    }

    EngineOptionData* EngineOptionData::FindInList(QString name, QList<EngineOptionData>& list)
    {
        QList<EngineOptionData>::iterator it = list.begin();
        while (it != list.end())
        {
            if ((*it).m_name == name)
            {
                return &(*it);
            }
            ++it;
        }
        return nullptr;
    }

}