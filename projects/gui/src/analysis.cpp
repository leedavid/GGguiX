
#include "analysis.h"
#include "board/boardfactory.h"

namespace Chess {

	Analysis::Analysis()
	{
		clear();
	}

    Analysis& Analysis::operator=(const Analysis& rhs)
    {
        if (this != &rhs)
        {
            m_score = rhs.m_score;
            m_msec = rhs.m_msec;
            m_depth = rhs.m_depth;
            m_mateIn = rhs.m_mateIn;
            m_nodes = rhs.m_nodes;
            m_numpv = rhs.m_numpv;
            m_bestMove = rhs.m_bestMove;
            m_endOfGame = rhs.m_endOfGame;
            m_variation = rhs.m_variation;
            m_bookMove = rhs.m_bookMove;
            m_tb = rhs.m_tb;
            m_scoreTb = rhs.m_scoreTb;
            m_elapsedTimeMS = rhs.m_elapsedTimeMS;
        }
        return *this;
    }

    void Analysis::clear()
    {
        m_mateIn = 99999;
        m_score = m_msec = m_depth = 0;
        m_nodes = 0;
        m_numpv = 1;
        m_elapsedTimeMS = 0;
        m_bestMove = false;
        m_bookMove = false;
        m_endOfGame = false;
        m_variation.clear();
        m_tb.setNullMove();
        m_scoreTb = 0;
    }

    bool Analysis::isValid() const
    {
        return isAlreadyMate() || (!m_variation.isEmpty() && m_depth > 0 && m_msec > 0);
    }

    int Analysis::mpv() const
    {
        return m_numpv;
    }

    void Analysis::setNumpv(int n)
    {
        m_numpv = n;
    }

    int Analysis::time() const
    {
        return m_msec;
    }

    void Analysis::setTime(int msec)
    {
        m_msec = msec;
    }

    int Analysis::score() const
    {
        return m_score;
    }

    double Analysis::fscore() const
    {
        return (double)m_score / 100.0;
    }

    void Analysis::setScore(int score)
    {
        m_score = score;
    }

    int Analysis::depth() const
    {
        return m_depth;
    }

    void Analysis::setDepth(int depth)
    {
        m_depth = depth;
    }

    quint64 Analysis::nodes() const
    {
        return m_nodes;
    }

    void Analysis::setNodes(quint64 nodes)
    {
        m_nodes = nodes;
    }

    MoveList Analysis::variation() const
    {
        return m_variation;
    }

    void Analysis::setVariation(const MoveList& variation)
    {
        m_variation = variation;
    }

    void Analysis::setBestMove(bool bestMove)
    {
        m_bestMove = bestMove;
    }

    bool Analysis::bestMove() const
    {
        return m_bestMove;
    }

    bool Analysis::isMate() const
    {
        return m_mateIn < 99999;
    }

    bool Analysis::isAlreadyMate() const
    {
        return m_mateIn == 0;
    }

    int Analysis::elapsedTimeMS() const
    {
        return m_elapsedTimeMS;
    }

    void Analysis::setElapsedTimeMS(int elapsedTimeMS)
    {
        m_elapsedTimeMS = elapsedTimeMS;
    }

    bool Analysis::getEndOfGame() const
    {
        return m_endOfGame;
    }

    void Analysis::setEndOfGame(bool value)
    {
        m_endOfGame = value;
    }

    bool Analysis::getBookMove() const
    {
        return m_bookMove;
    }

    void Analysis::setBookMove(bool bookMove)
    {
        m_bookMove = bookMove;
    }

    Move Analysis::getTb() const
    {
        return m_tb;
    }

    void Analysis::setTb(const Move& value)
    {
        m_tb = value;
    }

    int Analysis::getScoreTb() const
    {
        return m_scoreTb;
    }

    void Analysis::setScoreTb(int value)
    {
        m_scoreTb = value;
    }

    int Analysis::movesToMate() const
    {
        return m_mateIn;
    }

    void Analysis::setMovesToMate(int mate)
    {
        m_mateIn = mate;
    }

    QString Analysis::toString(const Board& board) const
    {
        //return (" QString Analysis::toString(const Board& board) const");
        //
        ///*
        
        Board* testBoard = board.copy();
        QString out;

        bool whiteToMove = testBoard->sideToMove() == Side::White;

        QString cw = "00cc99";
        QString cb = "ff3300";

        if (getEndOfGame())
        {
            QString color = whiteToMove ? cw : cb;
            QString text = tr("Resigns");
            out = QString("<font color=\"#%1\"><b>%2</b></font> ").arg(color).arg(text);
        }
        else if (isAlreadyMate())
        {
            QString color = whiteToMove ? cw : cb;
            QString text = tr("Mate");
            out = QString("<font color=\"#%1\"><b>%2</b></font> ").arg(color).arg(text);
        }
        else if (isMate())
        {
            int score = movesToMate();

            QString color = ((whiteToMove && (score > 0)) || (!whiteToMove && (score < 0))) ? cw : cb;
            QString text = QString(tr("Mate in %1").arg(abs(score)));
            out = QString("<font color=\"#%1\"><b>%2</b></font> ").arg(color).arg(text);
        }
        else if (!bestMove())
        {
            if (score() > 0)
            {
                out = QString("<font color=\"#%1\"><b>+%2</b></font> ").arg(cw).arg(score() / 100.0, 0, 'f', 2);
            }
            else
            {
                out = QString("<font color=\"#%1\"><b>%2</b></font> ").arg(cb).arg(score() / 100.0, 0, 'f', 2);
            }
        }

        int moveNo = testBoard->moveNumber();

        QString moveText;
        foreach(Move move, variation())
        {
            if (whiteToMove)
            {
                moveText += QString::number(moveNo++) + ". ";
            }
            else  if (moveText.isEmpty())
            {
                moveText += QString::number(moveNo++) + "... ";
            }
            //moveText += testBoard.moveToSan(move, true);
            moveText += testBoard->moveToSan(move);
            moveText += " ";
            //testBoard.doMove(move);
            testBoard->makeMove(move);
            whiteToMove = !whiteToMove;
        }
        if (!moveText.isEmpty())
        {
            out += " <a href=\"" + QString::number(-m_numpv) + "\" title=\"Click to add move to game\">[+]</a> ";
            if (!bestMove())
            {
                out += " <a href=\"" + QString::number(m_numpv) + "\" title=\"Click to add variation to game\">[*]</a> ";
            }
            out += moveText;
        }

        if (!bestMove())
        {
            QTime t(0, 0, 0, 0);
            t = t.addMSecs(time());
            QString elapsed = t.toString("h:mm:ss");
            out += tr(" (depth %1, %2)").arg(depth()).arg(elapsed);
        }
        else
        {
            if (!out.isEmpty())
            {
                out += tr(" (suggested move)");
            }
        }

        delete testBoard;
        return out;

        //*/
    }

}