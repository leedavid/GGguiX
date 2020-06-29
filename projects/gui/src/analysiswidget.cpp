
#include "settings.h"
#include "analysis.h"
#include "enginelist.h"
#include "messagedialog.h"
#include "analysiswidget.h"
#include "playerbuilder.h"
#include "mainwindow.h"
#include "cutechessapp.h"
#include "enginemanager.h"
#include "enginebuilder.h"
#include <chessplayer.h>
#include "moveevaluation.h"

namespace Chess {
    void AnalysisWidget::GetBulderCute()
    {
        int index = ui.engineList->currentIndex();   // 引擎选择
        if (index != -1) {
            if (m_buildersEngine == nullptr || index != m_bullderNum) {           


                if (m_buildersEngine)
                {
   /*                 m_buildersEngin->deactivate();
                    m_buildersEngin->deleteLater();*/
                    //m_buildersEngine->e
                    //m_buildersEngin.clear();
                    //m_buildersEngine->q
                    //m_buildersEngine->disconnect();
                }

                EngineManager* engineManager = CuteChessApplication::instance()->engineManager();
                auto config = engineManager->engineAt(index);
                QSettings s;
                s.beginGroup("games");
                bool ponder = s.value("pondering").toBool();
                s.endGroup();
                config.setPondering(ponder);

                this->m_buildersEngine = new EngineBuilder(config);

                m_bullderNum = index;
            }
        }
    }
    void AnalysisWidget::setPlayer(ChessPlayer* player)
    {
        if (player != m_player || !player)
            clear();
        if (m_player)
            m_player->disconnect(this);
        m_player = player;
        if (!player)
            return;

        connect(player, SIGNAL(startedThinking(int)),
            this, SLOT(clear()));
        connect(player, SIGNAL(thinking(MoveEvaluation)),
            this, SLOT(onEval(MoveEvaluation)));
    }
    AnalysisWidget::AnalysisWidget(QWidget* parent)
        : QWidget(parent),
        m_moveTime(0),
        m_bUciNewGame(true),
        m_onHold(false),
        m_gameMode(false),
        m_buildersEngine(nullptr),
        m_bullderNum(0),
        m_player(nullptr),
        m_isMainEngine(false)
    {
        pMain = (MainWindow*)parent;
        
        ui.setupUi(this);
        connect(ui.engineList, SIGNAL(activated(int)), SLOT(toggleAnalysis()));
        connect(ui.bookList, SIGNAL(currentIndexChanged(int)), SLOT(bookActivated(int)));
        connect(ui.analyzeButton, SIGNAL(clicked(bool)), SLOT(toggleAnalysis()));

        connect(ui.variationText, SIGNAL(anchorClicked(QUrl)),
            SLOT(slotLinkClicked(QUrl)));
        connect(ui.vpcount, SIGNAL(valueChanged(int)), SLOT(slotMpvChanged(int)));
        connect(ui.btPin, SIGNAL(clicked(bool)), SLOT(slotPinChanged(bool)));
        ui.analyzeButton->setFixedHeight(ui.engineList->sizeHint().height());


        m_board = Chess::BoardFactory::create("");       
        m_NextBoard = Chess::BoardFactory::create("");       
        m_startPos = Chess::BoardFactory::create("");

        m_board->setFenString(m_board->defaultFenString());
        m_NextBoard->setFenString(m_NextBoard->defaultFenString());
        m_startPos->setFenString(m_startPos->defaultFenString());

        


        //m_tablebase = new OnlineTablebase;   // 在线开局库
        //connect(m_tablebase, SIGNAL(bestMove(QList<Move>, int)), this, SLOT(showTablebaseMove(QList<Move>, int)));
    }

    AnalysisWidget::~AnalysisWidget()
    {
        stopEngine();
        //delete m_tablebase;

        delete m_board;
        delete m_NextBoard;
        delete m_startPos;

        if (m_buildersEngine != nullptr)
            delete m_buildersEngine;
    }

    void AnalysisWidget::startEngine()
    {
       
        // 得到当前选择的引擎
        //this->GetBulderCute();
        /*
        updateBookMoves();

        int index = ui.engineList->currentIndex();
        stopEngine();
        m_onHold = false;
        if (index != -1)
        {
            if (parentWidget() && !parentWidget()->isVisible())
            {
                parentWidget()->show();
            }
            ui.variationText->clear();
            m_engine = Engine::newEngine(index);
            ui.vpcount->setEnabled(m_engine->providesMvp());
            ui.label->setEnabled(m_engine->providesMvp());
            if (!m_engine->providesMvp())
            {
                ui.vpcount->setValue(1);
            }

            connect(m_engine, SIGNAL(activated()), SLOT(engineActivated()));
            connect(m_engine, SIGNAL(error(QProcess::ProcessError)), SLOT(engineError(QProcess::ProcessError)));
            connect(m_engine, SIGNAL(deactivated()), SLOT(engineDeactivated()));
            connect(m_engine, SIGNAL(analysisUpdated(const Analysis&)),
                SLOT(showAnalysis(Analysis)));
            m_engine->setMoveTime(m_moveTime);
            m_engine->activate();
            QString key = QString("/") + objectName() + "/Engine";
            AppSettings->setValue(key, ui.engineList->itemText(index));
        }
        */
    }

    void AnalysisWidget::stopEngine()
    {
       /*
        engineDeactivated();
        if (m_engine)
        {
            m_engine->deactivate();
            m_engine->deleteLater();
            m_engine.clear();
        }
        */
    }

    void AnalysisWidget::slotVisibilityChanged(bool visible)
    {
        if (isEngineRunning() && !visible && !parentWidget()->isVisible())
        {
            stopEngine();
        }
    }

    bool AnalysisWidget::isEngineRunning() const
    {
        return m_engine && ui.analyzeButton->isChecked();
    }

    bool AnalysisWidget::isEngineConfigured() const
    {
        int index = ui.engineList->currentIndex();
        return (index != -1);
    }

    void AnalysisWidget::engineActivated()
    {
        ui.analyzeButton->setChecked(true);
        ui.analyzeButton->setText(tr("停止"));
        m_analyses.clear();
        updateBookMoves(); // Delay this to here so that engine process is up
        if (!sendBookMove())
        {
            m_engine->setStartPos(*m_startPos);
            m_engine->startAnalysis(*m_board, ui.vpcount->value(), m_moveTime, true, m_line);
            m_lastEngineStart.start();
            m_bUciNewGame = false;
        }
    }

    void AnalysisWidget::engineError(QProcess::ProcessError e)
    {
        MessageDialog::warning(tr("There was an error (%1) running engine <b>%2</b>.")
            .arg(e)
            .arg(ui.engineList->currentText()));
        stopEngine();
    }

    void AnalysisWidget::engineDeactivated()
    {
        ui.analyzeButton->setChecked(false);
        ui.analyzeButton->setText(tr("使用"));
    }

    void AnalysisWidget::toggleAnalysis()
    {
        if (!isAnalysisEnabled())
        {
            stopEngine();
        }
        else
        {
            startEngine();
        }
    }

    void AnalysisWidget::bookActivated(int index)
    {
        m_pBookDatabase.clear();
        emit signalSourceChanged(index >= 0 ? ui.bookList->itemData(index).toString() : "");
        updateBookMoves();
        updateAnalysis();
    }

    void AnalysisWidget::slotPinChanged(bool pinned)
    {
        if (!pinned && isAnalysisEnabled())
        {
            if (m_board != m_NextBoard)
            {
                setPosition(*m_NextBoard, m_NextLine);
            }
        }
        else
        {
            if (isEngineRunning())
            {
                m_engine->setMoveTime(0);
            }
        }
    }

    void AnalysisWidget::onEval(const MoveEvaluation& eval)
    {
        int elapsed = m_lastEngineStart.elapsed();
        //int mpv = analysis.mpv() - 1;
        int mpv = eval.pvNumber() - 1;
        //bool bestMove = analysis.bestMove();
        bool bestMove = eval.isBestMove();
        if (bestMove)
        {
            if (m_eval.count() && m_eval.last().isBestMove())
            {
                m_eval.removeLast();
            }
            m_eval.append(eval);
        }
        else if (mpv < 0 || mpv > m_eval.count() || mpv >= ui.vpcount->value())
        {
            return;
        }
        else if (mpv == m_eval.count())
        {
            m_eval.append(eval);
        }
        else
        {
            m_eval[mpv] = eval;
        }
        //updateComplexity();
        updateEval();
        /*
        Analysis c = analysis;
        if (bestMove && analysis.variation().count())
        {
            foreach(Analysis a, m_analyses)
            {
                if (a.variation().count())
                {
                    if (a.variation().at(0) == analysis.variation().at(0))
                    {
                        c = a;
                        c.setBestMove(true);
                        break;
                    }
                }
            }
        }

        c.setTb(m_tb);
        c.setScoreTb(m_score_tb);
        if (bestMove)
        {
            analysis.setElapsedTimeMS(elapsed);
            emit receivedBestMove(c);
        }
        else if (c.getEndOfGame())
        {
            emit receivedBestMove(c);
        }
        */
    }

    void AnalysisWidget::clear()
    {
    }

    int Chess::AnalysisWidget::getCurEngineIndex()
    {
        return ui.engineList->currentIndex();
    }

    void AnalysisWidget::slotReconfigure()
    {       
        // 填充所有的引擎，选择上次保存的引擎
        EngineManager* m_engineManager =
            CuteChessApplication::instance()->engineManager();
        for (int i = 0; i < m_engineManager->engineCount(); i++) {
            ui.engineList->addItem(m_engineManager->engineAt(i).name());
        }

        int sel = 0;
        // 是不是主引擎
        if (this->getIsMainEngine()) {
            sel = QSettings().value("ui/linkboard_curEngineFirst").toInt();
        }
        else {
            sel = QSettings().value("ui/linkboard_curEngineSecond").toInt();
        }
        ui.engineList->setCurrentIndex(sel);

        /*
        QString oldEngineName = ui.engineList->currentText();
        if (oldEngineName.isEmpty())
        {
            QString key = QString("/") + objectName() + "/Engine";
            oldEngineName = AppSettings->getValue(key).toString();
        }

        EngineList enginesList;
        enginesList.restore();
        QStringList names = enginesList.names();
        ui.engineList->clear();
        ui.engineList->insertItems(0, names);
        int index = names.indexOf(oldEngineName);
        if (index != -1)
        {
            ui.engineList->setCurrentIndex(index);
        }
        else
        {
            ui.engineList->setCurrentIndex(0);
            stopEngine();
        }

        int fontSize = AppSettings->getValue("/General/ListFontSize").toInt();
        fontSize = std::max(fontSize, 8);
        QFont f = ui.variationText->font();
        f.setPointSize(fontSize);
        setFont(f);
        ui.variationText->setFont(f);
        */
    }

    void AnalysisWidget::saveConfig()
    {
        AppSettings->beginGroup(objectName());
        AppSettings->setValue("LastBook", ui.bookList->currentText());
        AppSettings->endGroup();
    }

    void AnalysisWidget::restoreBook()
    {
        AppSettings->beginGroup(objectName());
        QString lastBook = AppSettings->value("LastBook", "").toString();
        AppSettings->endGroup();
        int index = ui.bookList->findText(lastBook);
        if (index >= 0)
        {
            ui.bookList->setCurrentIndex(index);
        }
    }

    //void AnalysisWidget::slotUpdateBooks(QStringList files)
    //{
    //    QString current = ui.bookList->currentText();
    //    ui.bookList->clear();
    //    ui.bookList->addItem("-", QVariant(QString()));
    //    foreach(QString filename, files)
    //    {
    //        QFileInfo fi(filename);
    //        QString baseName = fi.baseName();
    //        //if (DatabaseInfo::IsBook(filename))
    //        //{
    //        //    ui.bookList->addItem(baseName, QVariant(filename));
    //        //}
    //    }
    //    int index = ui.bookList->findText(current);
    //    if (index < 0) index = 0;
    //    ui.bookList->setCurrentIndex(index);
    //}

    void AnalysisWidget::setGameMode(bool gameMode)
    {
        m_gameMode = gameMode;
        if (!m_gameMode)
        {
            updateBookMoves();
        }
    }

    void AnalysisWidget::showAnalysis(Analysis analysis)
    {
        int elapsed = m_lastEngineStart.elapsed();
        int mpv = analysis.mpv() - 1;
        bool bestMove = analysis.bestMove();
        if (bestMove)
        {
            if (m_analyses.count() && m_analyses.last().bestMove())
            {
                m_analyses.removeLast();
            }
            m_analyses.append(analysis);
        }
        else if (mpv < 0 || mpv > m_analyses.count() || mpv >= ui.vpcount->value())
        {
            return;
        }
        else if (mpv == m_analyses.count())
        {
            m_analyses.append(analysis);
        }
        else
        {
            m_analyses[mpv] = analysis;
        }
        updateComplexity();
        updateAnalysis();
        Analysis c = analysis;
        if (bestMove && analysis.variation().count())
        {
            foreach(Analysis a, m_analyses)
            {
                if (a.variation().count())
                {
                    if (a.variation().at(0) == analysis.variation().at(0))
                    {
                        c = a;
                        c.setBestMove(true);
                        break;
                    }
                }
            }
        }

        c.setTb(m_tb);
        c.setScoreTb(m_score_tb);
        if (bestMove)
        {
            analysis.setElapsedTimeMS(elapsed);
            emit receivedBestMove(c);
        }
        else if (c.getEndOfGame())
        {
            emit receivedBestMove(c);
        }
    }

    void AnalysisWidget::setPosition(const Board& board, QString line)
    {
        if (ui.btPin->isChecked())
        {   
            delete m_NextBoard;
            m_NextBoard = board.copy();
            m_NextLine = line;
            return;
        }
        if (!(*m_board == board))
        {
            delete m_board;
            m_board = board.copy();
            delete m_NextBoard;
            m_NextBoard = board.copy();
            m_NextLine = line;
            m_line = line;
            m_analyses.clear();
            //m_tablebase->abortLookup();
            m_tablebaseEvaluation.clear();
            m_tablebaseMove.clear();
            m_tb.setNullMove();
            m_score_tb = 0;

            updateBookMoves();

            if (AppSettings->getValue("/General/onlineTablebases").toBool())
            {
                //if (!(m_board.isStalemate() || m_board.isCheckmate() || m_board.chess960()))
                //if(m_board->result() != Result())
                //{
                //    if (objectName() == "Analysis")
                //    {
                //        m_tbBoard = m_board;
                //        m_tablebase->getBestMove(m_board.toFen());
                //    }
                //}
            }

            m_lastDepthAdded = 0;
            updateAnalysis();
            if (m_engine && m_engine->isActive() && !onHold() && !sendBookMove())
            {
                if (m_bUciNewGame)
                {
                    m_engine->setStartPos(*m_startPos);
                }
                m_engine->startAnalysis(*m_board, ui.vpcount->value(), m_moveTime, m_bUciNewGame, line);
                m_lastEngineStart.start();
                m_bUciNewGame = false;
            }
        }
    }

    bool AnalysisWidget::sendBookMove()
    {
        if (moveList.count() && m_moveTime.allowBook)
        {
            QTimer::singleShot(500, this, SLOT(sendBookMoveTimeout()));
            return true;
        }
        return false;
    }


    void Chess::AnalysisWidget::updateEval()
    {
        QString text;
        if (ui.btPin->isChecked())
        {
            unsigned int moveNr = m_board->moveNumber();
            text = tr("Analysis pinned to move %1").arg(moveNr) + "<br>";
        }
        foreach(auto a, m_eval)
        {
            QString s = a.toStrings();
            if (!s.isEmpty()) text.append(s + "<br>");
        }
        if (!m_tablebaseEvaluation.isEmpty())
        {
            text.append(QString("<a href=\"0\" title=\"%1\">[+]</a> <b>%2:</b> ").arg(tr("Click to add move to game")).arg(tr("Tablebase")) + m_tablebaseEvaluation);
        }
        if (m_lastDepthAdded == 17)
        {
            text.append(QString("<br><b>%1:</b> %2/%3<br>").arg(tr("Complexity")).arg(m_complexity).arg(m_complexity2));
        }
        else if (m_lastDepthAdded >= 12)
        {
            text.append(tr("<br><b>Complexity:</b> %1<br>").arg(m_complexity));
        }

        if (moveList.count())
        {
            QString bookLine = tr("<i>Book:</i>");
            foreach(MoveData move, moveList)
            {
                bookLine.append(" ");
                bookLine.append(move.localsan);
            }
            text.append(bookLine);
        }
        ui.variationText->setText(text);
    }

    void AnalysisWidget::sendBookMoveTimeout()
    {
        if (moveList.count() && m_moveTime.allowBook)
        {
            Analysis analysis;
            analysis.setElapsedTimeMS(0);
            MoveList moves;
            int index = 0;
            if (m_moveTime.bookMove == 1)
            {
                index = rand() % moveList.count();
            }
            else if (m_moveTime.bookMove == 2)
            {
                index = moveList.count() - 1;
                int randomPos = rand() % games;
                for (int i = 0; i < moveList.count(); ++i)
                {
                    randomPos -= moveList.at(i).count;
                    if (randomPos < 0)
                    {
                        index = i;
                        break;
                    }
                }
            }
            moves.append(moveList.at(index).move);
            analysis.setVariation(moves);
            analysis.setBestMove(true);
            analysis.setBookMove(true);
            analysis.setTb(m_tb);
            analysis.setScoreTb(m_score_tb);
            emit receivedBestMove(analysis);
        }
    }

    void AnalysisWidget::slotLinkClicked(const QUrl& url)
    {
        if (m_NextBoard != m_board)
        {
            return; // Pinned and user moved somewhere else
        }
        int mpv = url.toString().toInt() - 1;
        if (mpv >= 0 && mpv < m_analyses.count())
        {
            emit addVariation(m_analyses[mpv], "");
        }
        else if (mpv == -1)
        {
            emit addVariation(m_tablebaseMove);
        }
        else
        {
            mpv = (-mpv) - 2;
            if (mpv < m_analyses.count())
            {
                if (!m_analyses[mpv].variation().isEmpty())
                {
                    emit addVariation(m_analyses[mpv].variation().at(0).toAlgebraic());
                }
            }
        }
    }

    void AnalysisWidget::setMoveTime(EngineParameter mt)
    {
        m_moveTime = mt;
        if (isEngineRunning() && !ui.btPin->isChecked())
        {
            m_engine->setMoveTime(mt);
        }
    }

    void AnalysisWidget::setMoveTime(int n)
    {
        EngineParameter par(n);
        par.analysisMode = true;
        setMoveTime(par);
    }

    void AnalysisWidget::setDepth(int n)
    {
        m_moveTime.searchDepth = n;
        m_moveTime.analysisMode = true;
        setMoveTime(m_moveTime);
    }

    void AnalysisWidget::slotMpvChanged(int mpv)
    {
        //isEngineRunning()
       
        /*
        if (isEngineRunning())
        {
            while (m_analyses.count() > mpv)
            {
                m_analyses.removeLast();
            }
            m_engine->setMpv(mpv);  // 引擎改变 multiPV 
        }
        */
    }

    bool AnalysisWidget::isAnalysisEnabled() const
    {
        if (!parentWidget())
        {
            return false;
        }
        if (!parentWidget()->isVisible() || !ui.analyzeButton->isChecked())
        {
            return false;
        }
        return true;
    }

    void AnalysisWidget::showTablebaseMove(QList<Move> bestMoves, int score)
    {
        (void)score;
        (void)bestMoves;
        /*if (m_tbBoard == m_board)
        {
            bool first = true;
            QStringList also;
            foreach(Move move, bestMoves)
            {
                if (first)
                {
                    first = false;
                    QString result;

                    bool dtz = false;
                    if (abs(score) & 0x800)
                    {
                        dtz = true;
                    }
                    m_score_tb = 0;
                    if (score == 0)
                    {
                        result = tr("Draw");
                    }
                    else
                    {
                        if (!dtz)
                        {
                            if ((score < 0) == (m_board.toMove() == Black))
                            {
                                result = tr("White wins in %n moves", "", qAbs(score));
                                m_score_tb = 1;
                            }
                            else
                            {
                                result = tr("Black wins in %n moves", "", qAbs(score));
                                m_score_tb = -1;
                            }
                        }
                        else
                        {
                            if ((score < 0) == (m_board.toMove() == Black))
                            {
                                result = tr("White wins");
                                m_score_tb = 1;
                            }
                            else
                            {
                                result = tr("Black wins");
                                m_score_tb = -1;
                            }
                        }
                    }
                    Move move1 = m_board.prepareMove(move.from(), move.to());
                    if (move.isPromotion())
                    {
                        move1.setPromoted(pieceType(move.promotedPiece()));
                    }
                    m_tablebaseEvaluation = QString("%1 - %2").arg(m_board.moveToFullSan(move1, true)).arg(result);
                    m_tablebaseMove = m_board.moveToFullSan(move1);
                    m_tb = move1;
                    m_lastDepthAdded = 0;
                }
                else
                {
                    Move move1 = m_board.prepareMove(move.from(), move.to());
                    if (move.isPromotion())
                    {
                        move1.setPromoted(pieceType(move.promotedPiece()));
                    }
                    also.append(m_board.moveToFullSan(move1, true));
                }
            }
            if (!also.isEmpty())
            {
                m_tablebaseEvaluation.append(QString(" === %1").arg(also.join(" ")));
            }
            updateAnalysis();
        }*/
    }

    void AnalysisWidget::updateAnalysis()
    {
        QString text;
        if (ui.btPin->isChecked())
        {
            unsigned int moveNr = m_board->moveNumber();
            text = tr("Analysis pinned to move %1").arg(moveNr) + "<br>";
        }
        foreach(Analysis a, m_analyses)
        {
            QString s = a.toStrings(*m_board);
            if (!s.isEmpty()) text.append(s + "<br>");
        }
        if (!m_tablebaseEvaluation.isEmpty())
        {
            text.append(QString("<a href=\"0\" title=\"%1\">[+]</a> <b>%2:</b> ").arg(tr("Click to add move to game")).arg(tr("Tablebase")) + m_tablebaseEvaluation);
        }
        if (m_lastDepthAdded == 17)
        {
            text.append(QString("<br><b>%1:</b> %2/%3<br>").arg(tr("Complexity")).arg(m_complexity).arg(m_complexity2));
        }
        else if (m_lastDepthAdded >= 12)
        {
            text.append(tr("<br><b>Complexity:</b> %1<br>").arg(m_complexity));
        }

        if (moveList.count())
        {
            QString bookLine = tr("<i>Book:</i>");
            foreach(MoveData move, moveList)
            {
                bookLine.append(" ");
                bookLine.append(move.localsan);
            }
            text.append(bookLine);
        }
        ui.variationText->setText(text);
    }



    void AnalysisWidget::updateComplexity()
    {
        if (!m_analyses[0].variation().isEmpty())
        {
            Move bestMove = m_analyses[0].variation().first();
            if (m_analyses[0].depth() == 2)
            {
                m_lastBestMove = bestMove;
                m_complexity = 0.0;
                m_lastDepthAdded = 2;
            }
            if ((m_analyses.size() >= 2) && !m_analyses[1].bestMove())
            {
                if ((m_lastDepthAdded + 1 == m_analyses[0].depth()) && !m_analyses[0].isMate())
                {
                    if (m_analyses[0].depth() <= 12)
                    {
                        m_lastDepthAdded = m_analyses[0].depth();
                        if (m_lastBestMove != bestMove)
                        {
                            if (abs(m_analyses[0].score()) < 200)
                            {
                                m_lastBestMove = bestMove;
                                m_complexity += fabs(double(m_analyses[0].score() - m_analyses[1].score())) / 100.0;
                            }
                        }
                        m_complexity2 = m_complexity;
                    }
                    else if ((m_analyses[0].depth() > 12) && (m_analyses[0].depth() <= 17))
                    {
                        m_lastDepthAdded = m_analyses[0].depth();
                        if (m_lastBestMove != bestMove)
                        {
                            if (abs(m_analyses[0].score()) < 200)
                            {
                                m_lastBestMove = bestMove;
                                m_complexity2 += fabs(double(m_analyses[0].score() - m_analyses[1].score())) / 100.0;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            m_lastDepthAdded = 0;
        }
    }

    Analysis AnalysisWidget::getMainLine() const
    {
        Analysis a;
        if (!m_analyses.isEmpty())
        {
            a = m_analyses.first();
        }
        return a;
    }

    bool AnalysisWidget::hasMainLine() const
    {
        return (!m_analyses.isEmpty());
    }

    QString AnalysisWidget::displayName() const
    {
        return ui.engineList->currentText();
    }

    void AnalysisWidget::unPin()
    {
        if (ui.btPin->isChecked())
        {
            ui.btPin->setChecked(false);
        }
    }

    void AnalysisWidget::slotUciNewGame(const Board& b)
    {
        m_bUciNewGame = true;
        delete m_startPos;
        m_startPos = b.copy();
    }

    bool AnalysisWidget::onHold() const
    {
        return m_onHold;
    }

    void AnalysisWidget::setOnHold(bool onHold)
    {
        m_onHold = onHold;
        if (!onHold && (!m_engine || !m_engine->isActive()))
        {
            startEngine();
        }
    }

    QString AnalysisWidget::engineName() const
    {
        return ui.engineList->currentText();
    }

  

    //void AnalysisWidget::updateBookFile(Database* pgdb)
    //{
    //    m_pBookDatabase = pgdb;
    //}

    void AnalysisWidget::updateBookMoves()
    {
        //QMap<Move, MoveData> moves;
        //games = 0;

        //if (m_pBookDatabase && !m_gameMode)
        //{
        //    games = m_pBookDatabase->getMoveMapForBoard(m_board, moves);
        //}

        //moveList.clear();
        //for (QMap<Move, MoveData>::iterator it = moves.begin(); it != moves.end(); ++it)
        //{
        //    moveList.append(it.value());
        //}

        //std::sort(moveList.begin(), moveList.end());
    }



}
