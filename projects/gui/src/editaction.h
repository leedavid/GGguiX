#pragma once

#include "chessgame.h"
#include <QtCore>

namespace Chess {

    /** The EditAction is a ChessBrowser action. */
    //class EditAction
    //{
    //public:
    //    enum Type {
    //        None, RemovePreviousMoves, RemoveNextMoves, RemoveVariation, PromoteVariation,
    //        EnumerateVariations1, EnumerateVariations2, VariationUp, VariationDown,
    //        EditGameComment, EditPrecomment, EditComment, AddNag, ClearNags, AddNullMove, CopyHtml, CopyText,
    //        Uncomment, RemoveVariations
    //    };
    //    /** Standard constructor. */
    //    EditAction(Type type = None);
    //    /** Standard constructor with additional data. */
    //    EditAction(Type type, const QVariant& data);
    //    /** @return action type. */
    //    Type type() const;
    //    /** @return move associated with the action. */
    //    MoveId move() const;
    //    /** Set associated move. */
    //    void setMove(MoveId move);
    //    /** @return additonal data. */
    //    QVariant data() const;
    //private:
    //    Type m_type;
    //    QVariant m_data;
    //    MoveId m_move;
    //};


}