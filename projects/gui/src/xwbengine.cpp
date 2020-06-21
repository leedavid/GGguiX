#include "xwbengine.h"
#include "enginedata.h"



namespace Chess {

    WBEngine::WBEngine(const QString& name,
        const QString& command,
        bool bTestMode,
        const QString& directory,
        bool log, bool sendHistory) : Engine(name, command, bTestMode, directory, log, sendHistory)
    {
        m_analyze = false;
        m_setboard = false;		// We do not support version 1 xboard protocol, so this _must_ be set true by feature discovery
        m_bHasSentAnalyze = false;
        m_invertBlack = true;
    }

    void WBEngine::go()
    {
        send("post");
        if (m_moveTime.tm == EngineParameter::TIME_GONG)
        {
            if (m_moveTime.ms_totalTime > 0)
            {
                if (m_moveTime.searchDepth < 0)
                    send(QString("st %1").arg(m_moveTime.ms_totalTime / 1000));
                else
                    send(QString("sd %1").arg(m_moveTime.searchDepth));
                send("go");
                m_bHasSentAnalyze = false;
            }
            else
            {
                send("analyze");
                m_bHasSentAnalyze = true;
            }
        }
        else if (m_moveTime.tm == EngineParameter::TIME_SUDDEN_DEATH)
        {
            QTime t(0, 0);
            t = t.addMSecs(m_board->sideToMove() == Side::White ? m_moveTime.ms_white : m_moveTime.ms_black);
            send(QString("level 0 %1 %2").arg(t.toString("HH:mm:ss")).arg(m_moveTime.ms_increment / 1000));
            send("go");
            m_bHasSentAnalyze = false;
        }
    }

    void WBEngine::setStartPos(const Board&)
    {

    }

    bool WBEngine::startAnalysis(const Board& board, int nv, const EngineParameter& mt, bool /*bNewGame*/, QString /* line */)
    {
        Engine::setMoveTime(mt);
        m_mpv = nv;

        if (isActive() && *m_board == board)
        {
            return true;
        }
        stopAnalysis();
        delete m_board;
        m_board = board.copy();
        if (m_analyze && isActive() && m_setboard)
        {
            send("setboard " + board.fenString());
            go();
            setAnalyzing(true);
            return true;
        }
        return false;
    }

    void WBEngine::stopAnalysis()
    {
        if (isAnalyzing())
        {
            if (m_bHasSentAnalyze) send("exit");
            setAnalyzing(false);
        }
    }

    void WBEngine::protocolStart()
    {
        send("xboard");
        send("protover 2");
        QTimer::singleShot(2000, this, SLOT(featureTimeout()));
    }

    void WBEngine::protocolEnd()
    {
        stopAnalysis();
        send("quit");
        setActive(false);
        m_board->clear();
    }

    void WBEngine::processMessage(const QString& message)
    {
        QString trim(message);

        // GNU Chess always prompts ...
        if (trim.startsWith("White (1) : "))
        {
            trim = message.mid(12);
        }

        trim = trim.trimmed();

        if (!isActive() && trim.startsWith("Crafty v"))
        {
            m_invertBlack = false;
        }

        //determine command
        QString command = trim.section(" ", 0, 0);

        //identify and process the command
        if (command == "feature")
        {
            feature(trim);
        }
        else if (command == "move")
        {
            parseBestMove(trim);
        }
        else if ((command == "resign") || (command == "result") || (command == "1-0") || (command == "0-1") || (command == "1/2-1/2"))
        {
            parseEndOfGame(command, trim);
        }
        else if (isAnalyzing())
        {
            parseAnalysis(trim);
        }
    }

    void WBEngine::feature(const QString& command)
    {
        //break up command into individual features
        int index = command.indexOf(' ') + 1;
        int equalsIndex;
        int endIndex;
        QString feature;
        QString value;

        while (index < (int)command.length())
        {

            //feature name terminates with an =
            equalsIndex = command.indexOf('=', index);
            feature = command.mid(index, equalsIndex - index);

            //string values are delimited by ", others end with whitespace
            if (command[equalsIndex + 1] == '"')
            {
                endIndex = command.indexOf('"', equalsIndex + 2);
                value = command.mid(equalsIndex + 2, (endIndex - equalsIndex) - 2);
                index = endIndex + 2;
            }
            else
            {
                endIndex = command.indexOf(' ', equalsIndex + 1);
                value = command.mid(equalsIndex + 1, (endIndex - equalsIndex) - 1);
                index = endIndex + 1;
            }

            //if endIndex is -1 then the end of the string has been reached
            if (endIndex == -1)
            {
                index = command.length();
            }

            //process feature/value pair
            if (feature == "setboard")
            {
                m_setboard = (bool)value.toInt();
                send("accepted " + feature);
            }
            else if (feature == "analyze")
            {
                m_analyze = (bool)value.toInt();
                send("accepted " + feature);
            }
            else if (feature == "done")
            {
                send("accepted " + feature);
                featureDone((bool)value.toInt());
            }
            else
            {
                //unknown feature, reject it
                send("rejected " + feature);
            }

            QString name = feature;
            EngineOptionData option;
            option.m_name = name;
            option.m_defVal = value;
            option.m_type = OPT_TYPE_SPIN;

            m_options.append(option);
        }
    }

    void WBEngine::featureDone(bool done)
    {
        // We've received a "done" feature offer from engine,
        // so it supports V2 or better of the xboard protocol

        // No need to wait any longer wondering if we're talking to a V1 engine

        if (done)
        {
            setActive(true);
        }

        // The engine will send done=1, when its ready to go,
        //  and done=0 if it needs more than 2 seconds to start.
    }

    void WBEngine::featureTimeout()
    {
        if (!isActive())
        {
            v1TurnOffPondering();
            setActive(true);
        }
    }

    void WBEngine::parseBestMove(const QString& message)
    {
        QString bestMove = message.section(' ', 1, 1, QString::SectionSkipEmpty);

        if (!bestMove.isEmpty())
        {
            Analysis analysis;
            //Move move = m_board.parseMove(bestMove);
            auto move = m_board->moveFromString(bestMove);
            MoveList moves;
            //if (move.isLegal())
            if (!move.isNull())
            {
                moves.append(move);
            }
            analysis.setVariation(moves);
            analysis.setBestMove(true);
            sendAnalysis(analysis);
        }
    }

    void WBEngine::parseEndOfGame(const QString& command, const QString& message)
    {
        Analysis analysis;
        analysis.setEndOfGame(true);
        if (command == "1-0" || message.contains("1-0"))
        {
            analysis.setScore(30000);
        }
        else if (command == "0-1" || message.contains("0-1"))
        {
            analysis.setScore(-30000);
        }
        else if (command == "1/2-1/2")
        {
            analysis.setScore(0);
        }
        else if (message.isEmpty())
        {
            analysis.setScore(m_board->sideToMove() == Side::White ? 30000 : -30000);
        }
        sendAnalysis(analysis);
    }

    void WBEngine::parseAnalysis(const QString& message)
    {
        QString trimmed = message.simplified();
        Analysis analysis;
        bool ok;
        bool timeInSeconds = false;

        //Depth
        QString depth = trimmed.section(' ', 0, 0);
        analysis.setDepth(depth.toInt(&ok));
        if (!ok)
        {
            depth.truncate(depth.length() - 1);
            analysis.setDepth(depth.toInt(&ok));
            if (!ok)
            {
                return;
            }
            timeInSeconds = true;
        }

        //Score
        int score = trimmed.section(' ', 1, 1).toInt(&ok);
        if (!ok)
        {
            return;
        }
        if (m_invertBlack && m_board->sideToMove() == Side::Black)
        {
            score = -score;
        }
        analysis.setScore(score);

        //Time
        int time = trimmed.section(' ', 2, 2).toInt(&ok);
        if (!ok)
        {
            return;
        }
        if (timeInSeconds)
        {
            time = time * 1000;
        }
        else
        {
            time *= 10;
        }
        analysis.setTime(time);

        //Node
        analysis.setNodes(trimmed.section(' ', 3, 3).toLongLong(&ok));
        if (!ok)
        {
            return;
        }

        //Variation
        Board* board = m_board->copy();
        QString sanMove;
        MoveList moves;
        int section = 4;
        while ((sanMove = trimmed.section(' ', section, section)) != "")
        {
            if (sanMove.startsWith("("))
            {
                break;
            }
            // SBE -- What the heck is "<HT>" and why does crafty send it?
            if (!sanMove.endsWith(".") && sanMove != "<HT>")
            {
                //Move move = board.parseMove(sanMove);
                auto move = board->moveFromString(sanMove);
                //if (!move.isLegal())
                if (move.isNull())
                {
                    break;
                }
                //board.doMove(move);
                board->makeMove(move);
                moves.append(move);
            }
            section++;
        }
        analysis.setVariation(moves);

        if (analysis.isValid())
        {
            sendAnalysis(analysis);
        }
        delete board;
    }

    void WBEngine::v1TurnOffPondering()
    {
        send("hard");
        send("easy");
    }

}