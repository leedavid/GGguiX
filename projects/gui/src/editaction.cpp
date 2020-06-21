
#include "editaction.h"

namespace Chess {
    EditAction::EditAction(Type type)
        : m_type(type), m_move(NO_MOVE)
    {

    }

    EditAction::EditAction(Type type, const QVariant& data)
        : m_type(type), m_data(data), m_move(NO_MOVE)
    {

    }

    EditAction::Type EditAction::type() const
    {
        return m_type;
    }

    MoveId EditAction::move() const
    {
        return m_move;
    }

    void EditAction::setMove(MoveId move)
    {
        m_move = move;
    }

    QVariant EditAction::data() const
    {
        return m_data;
    }

}