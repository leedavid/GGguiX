#pragma once

#include "chessgame.h"

#include <QtCore>
#include <QTime>

namespace Chess {

    class MoveList : public QVector<Move>
    {
    public:
        MoveList() : QVector<Move>() {}
        inline Move& add()
        {
            push_back(Move());
            return back();
        }
    };

    /** The analysis class contains engine analysis. */
    class Analysis
    {
        Q_DECLARE_TR_FUNCTIONS(Analysis)

    public:
        Analysis();
        /** Reset values. */
        void clear();
        /** Check if analysis is valid. */
        bool isValid() const;
        /** order of the variarion in the variation list */
        int mpv() const;
        /** Set order of the variation. */
        void setNumpv(int n);
        /** Time elapsed in miliseconds. */
        int time() const;
        /** Set time in miliseconds. */
        void setTime(int msec);
        /** Evaluation in centipawns. */
        int score() const;
        /** Evaluation in pawns. */
        double fscore() const;
        /** Set evaluation in centipawns. */
        void setScore(int score);
        /** Depth in plies. */
        int depth() const;
        /** Set depth in plies. */
        void setDepth(int depth);
        void setNPS(qint64 nps);
        /** Nodes spent on analysis. */
        quint64 nodes() const;
        /** Set nodes spent on analysis. */
        void setNodes(quint64 nodes);
        /** Main variation. */
        MoveList variation() const;
        /** Set main variation. */
        void setVariation(const MoveList& variation);
        /** Is mate. */
        bool isMate() const;
        /** @return moves to mate */
        int movesToMate() const;
        /** Set moves to mate. */
        void setMovesToMate(int mate);
        /** Moves to mate. */
        /** Convert analysis to formatted text. */
        QString toStrings(const Board& board) const;
        /** Assignment operator */
        Analysis& operator=(const Analysis& rhs);
        void setBestMove(bool bestMove);
        bool bestMove() const;

        bool isAlreadyMate() const;

        int elapsedTimeMS() const;
        void setElapsedTimeMS(int elapsedTimeMS);

        bool getEndOfGame() const;
        void setEndOfGame(bool value);

        bool getBookMove() const;
        void setBookMove(bool bookMove);

        Chess::Move getTb() const;
        void setTb(const Move& value);

        int getScoreTb() const;
        void setScoreTb(int value);

    private:
        int m_numpv;
        int m_msec;
        int m_mateIn;
        int m_depth;
        int m_score;
        bool m_bestMove;
        bool m_endOfGame;
        bool m_bookMove;
        quint64 m_nodes;
        MoveList m_variation;
        int m_elapsedTimeMS;
        Move m_tb;
        int m_scoreTb;
        qint64 m_nps;                 // ËÙ¶È
    };


}



