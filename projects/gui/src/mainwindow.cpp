/*
    This file is part of GGzero Chess.

    GGzero Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GGzero Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GGzero Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include "openingbook.h"
#include "polyglotbook.h"
#include "gamewindow.h"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QTreeView>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QClipboard>
#include <QWindow>
#include <QSettings>
#include <QDesktopWidget>
#include <qtoolbutton.h>
#include <QLabel>


#include <board/boardfactory.h>
#include <chessgame.h>
#include <timecontrol.h>
#include "dockwidgetex.h"
#include <enginemanager.h>
#include <gamemanager.h>
#include <playerbuilder.h>
#include <chessplayer.h>
#include <humanbuilder.h>
#include <enginebuilder.h>
#include <tournament.h>

#include "analysiswidget.h"
#include "cutechessapp.h"
#include "gameviewer.h"
#include "movelist.h"
#include "newgamedlg.h"
#include "newtournamentdialog.h"
#include "chessclock.h"
#include "plaintextlog.h"
#include "gamedatabasemanager.h"
#include "pgntagsmodel.h"
#include "gametabbar.h"
#include "evalhistory.h"
#include "evalwidget.h"
#include "boardview/boardscene.h"
#include "tournamentresultsdlg.h"
#include <engineconfiguration.h>
#include "preferences.h"
#include "settings.h"
//#include "result.h"

#include "BoardEditor.h"
#include "random.h"

#include <pgnstream.h>
#include <pgngameentry.h>
#include "chessbrowser.h"

#include "TrainFenToServer.h"
#include "ChessDB.h"

#ifdef USE_SOUND
#include <QSound>
#endif
#ifdef USE_SPEECH
#include <QTextToSpeech>
#endif

#ifdef QT_DEBUG
#include <modeltest.h>
#endif

MainWindow::TabData::TabData(ChessGame* game, Tournament* tournament)
	: m_id(game),
	  m_game(game),
	  m_pgn(game->pgn()),
	  m_tournament(tournament),
	  m_finished(false)	{}

MainWindow::MainWindow(ChessGame* game)
	: m_game(nullptr),
	m_closing(false),
	m_readyToClose(false),
	m_firstTabAutoCloseEnabled(true),
	m_myClosePreTab(false),
	m_pcap(nullptr),
	m_autoClickCap(nullptr),
	m_now_is_match(false),
	m_ct(nullptr)
	//m_ChessDB(nullptr)
	//m_bAutomaticLinking(false)
{

	//static const int M_MAX_PLAYER = 10;
	//ChessPlayer* m_my_ChessPlayer[M_MAX_PLAYER];       // 最多10个引擎 player
	//ChessPlayer* m_my_ChessPlayerHumen;                // 人类player 	
	for (auto &p : m_my_ChessPlayer) {
		p = nullptr;
	}
	m_my_ChessPlayerHumen = nullptr;

	//this->setContextMenuPolicy(Qt::CustomContextMenu);  // 右键菜单

	setAttribute(Qt::WA_DeleteOnClose, true);
	setDockNestingEnabled(true);

	m_gameViewer = new GameViewer(Qt::Horizontal, nullptr, true);
	for (int i = 0; i < 2; i++)
	{
		Chess::Side side = Chess::Side::Type(i);
		m_gameViewer->chessClock(side)->setPlayerName(side.toString());
	}
	m_gameViewer->setContentsMargins(6, 6, 6, 6);

	m_moveList = new MoveList(this);
	m_tagsModel = new PgnTagsModel(this);
#ifdef QT_DEBUG
	new ModelTest(m_tagsModel, this);
#endif

	m_evalHistory = new EvalHistory(this);					// 历史曲线窗口
	//m_evalWidgets[0] = new EvalWidget(this);				// PV 路径窗口
	//m_evalWidgets[1] = new EvalWidget(this);

	m_AnalysisWidget[0] = new Chess::AnalysisWidget(this);
	m_AnalysisWidget[0]->setIsMainEngine(true);             // 主引擎窗口

	m_AnalysisWidget[1] = new Chess::AnalysisWidget(this);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(m_gameViewer);

	// The content margins look stupid when used with dock widgets
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* mainWidget = new QWidget(this);
	mainWidget->setLayout(mainLayout);
	setCentralWidget(mainWidget);

	

	createActions();
	createMenus();
	createToolBars();
	createDockWindows();

	/* Setup the dimensions of all widgets and the main board */
	slotReconfigure();	
	//-----------------------------------------------------------------------------------------------------------------

	connect(m_moveList, SIGNAL(moveClicked(int,bool)),			     // 点击棋谱走步
	        m_gameViewer, SLOT(viewMove(int,bool)));
	connect(m_moveList, SIGNAL(commentClicked(int, QString)),        // 点击棋步说明
		this, SLOT(editMoveComment(int, QString)));
	connect(m_gameViewer, SIGNAL(moveSelected(int)),
		m_moveList, SLOT(selectMove(int)));

	connect(m_gameViewer->boardScene(), SIGNAL(MouseRightClicked()),
		this, SLOT(onMouseRightClicked()));   // 棋盘右键菜单


	connect(CuteChessApplication::instance()->gameManager(),
		SIGNAL(finished()), this, SLOT(onGameManagerFinished()),
		Qt::QueuedConnection);

	readSettings();
	addGame(game);
	createStatus();

	slotDatabaseChanged();

	if (isMinimized())
	{
		showNormal();
	}

	this->mainToolbar->setVisible(true);   // by LGL

	// m_ChessDB;
	
}

MainWindow::~MainWindow()
{
	if (m_pcap != nullptr) {
		while (m_pcap->isRunning()) {
			m_pcap->on_stop();
			wait(1);
		}
		delete m_pcap;
	}
	if (m_autoClickCap != nullptr) {
		while (m_autoClickCap->isRunning()) {
			m_autoClickCap->on_stop();
			wait(1);
		}
		delete m_autoClickCap;
	}
	
	if (m_ct != nullptr) {
		delete m_ct;
	}

	
	
}


void MainWindow::readSettings()
{
	// https://blog.csdn.net/liang19890820/article/details/50513695

	QSettings s;
	s.beginGroup("ui");
	s.beginGroup("mainwindow");

	restoreGeometry(s.value("geometry").toByteArray());
	// Workaround for https://bugreports.qt.io/browse/QTBUG-16252
	if (isMaximized())
		setGeometry(QApplication::desktop()->availableGeometry(this));
	restoreState(s.value("window_state").toByteArray());

	s.endGroup();
	s.endGroup();
}

void MainWindow::writeSettings()
{
	QSettings s;
	s.beginGroup("ui");
	s.beginGroup("mainwindow");

	s.setValue("geometry", saveGeometry());
	s.setValue("window_state", saveState());

	s.endGroup();
	s.endGroup();

	// 当前连线方式
	int sel = this->cbtnLinkBoard->currentIndex();
	QSettings().setValue("ui/linkboard_curSel", sel);


	// 当前主引擎
	sel = this->getSelEngineIndex(true);
	QSettings().setValue("ui/linkboard_curEngineFirst", sel);  // 当前选择的引擎

	// 当前副引擎
	sel = this->getSelEngineIndex(false);
	QSettings().setValue("ui/linkboard_curEngineSecond", sel);  // 当前选择的引擎

	//sel = this->cbtnLinkEngine->currentIndex();
	//QSettings().setValue("ui/linkboard_curEngine", sel);  // 当前选择的引擎
	
}

void MainWindow::addGame(ChessGame* game)
{
	Tournament* tournament = qobject_cast<Tournament*>(QObject::sender());
	
	TabData tab(game, tournament);


	if (tournament)
	{
		int index = tabIndex(tournament, true);
		if (index != -1)
		{
			delete m_tabs[index].m_pgn;
			m_tabs[index] = tab;

			m_tabBar->setTabText(index, genericTitle(tab));
			if (!m_closing && m_tabBar->currentIndex() == index)
				setCurrentGame(tab);

			return;
		}
	}
	else
		connect(game, SIGNAL(finished(ChessGame*)),
			this, SLOT(onGameFinished(ChessGame*)));

	m_tabs.append(tab);
	m_tabBar->setCurrentIndex(m_tabBar->addTab(genericTitle(tab)));

	// close initial tab if unused and if enabled by settings
	if (m_tabs.size() >= 2
	&&  m_firstTabAutoCloseEnabled)
	{
		if (QSettings().value("ui/close_unused_initial_tab", true).toBool()
		&&  !m_tabs[0].m_game.isNull()
		&&  m_tabs[0].m_game.data()->moves().isEmpty())
			slotCloseTab(0);

		m_firstTabAutoCloseEnabled = false;
		//m_myClosePreTab = true;
	}
	else {
		if (m_myClosePreTab) {
			slotCloseTab(0);
			m_myClosePreTab = false;
		}
	}

	if (m_tabs.size() >= 2)
		m_tabBar->parentWidget()->show();

	// 
	if (m_sliderSpeed != nullptr) {
		m_sliderSpeed->setValue(0);
		connect(m_sliderSpeed, SIGNAL(translatedValueChanged(int)), game, SLOT(onAdjustTimePerMove(int)));
	}
	
}

void MainWindow::removeGame(int index)
{
	//if (index == -1) {
	//	return;
	//}
	
	Q_ASSERT(index != -1);

	m_tabs.removeAt(index);
	m_tabBar->removeTab(index);

	if (m_tabs.size() == 1)
		m_tabBar->parentWidget()->hide();
}

void MainWindow::slotDestroyGame(ChessGame* game)
{
	Q_ASSERT(game != nullptr);

	int index = tabIndex(game);

	if (index == -1) {
		return;  // by LGL
	}

	Q_ASSERT(index != -1);
	TabData tab = m_tabs.at(index);

	removeGame(index);

	if (tab.m_tournament == nullptr)
		game->deleteLater();

	delete tab.m_pgn;

	if (m_tabs.isEmpty())
		close();
}

void MainWindow::setCurrentGame(const TabData& gameData)
{
	if (gameData.m_game == m_game && m_game != nullptr)
		return;

	for (int i = 0; i < 2; i++)
	{
		ChessPlayer* player(m_players[i]);
		if (player != nullptr)
		{
			disconnect(player, nullptr, m_engineDebugLog, nullptr);
			disconnect(player, nullptr,
			           m_gameViewer->chessClock(Chess::Side::White), nullptr);
			disconnect(player, nullptr,
			           m_gameViewer->chessClock(Chess::Side::Black), nullptr);
		}
	}

	if (m_game != nullptr)
	{
		m_game->pgn()->setTagReceiver(nullptr);
		m_gameViewer->disconnectGame();
		disconnect(m_game, nullptr, m_moveList, nullptr);

		ChessGame* tmp = m_game;
		m_game = nullptr;

		// QObject::disconnect() is not atomic, so we need to flush
		// all pending events from the previous game before switching
		// to the next one.
		tmp->lockThread();
		CuteChessApplication::processEvents();
		tmp->unlockThread();

		// If the call to CuteChessApplication::processEvents() caused
		// a new game to be selected as the current game, then our
		// work here is done.
		if (m_game != nullptr)
			return;
	}

	m_game = gameData.m_game;

	lockCurrentGame();

	m_engineDebugLog->clear();

	m_moveList->setGame(m_game, gameData.m_pgn);
	m_evalHistory->setGame(m_game);

	if (m_game == nullptr)
	{
		m_gameViewer->setGame(gameData.m_pgn);
		m_evalHistory->setPgnGame(gameData.m_pgn);

		for (int i = 0; i < 2; i++)
		{
			Chess::Side side = Chess::Side::Type(i);
			auto clock = m_gameViewer->chessClock(side);
			clock->stop();
			clock->setInfiniteTime(true);
			QString name = nameOnClock(gameData.m_pgn->playerName(side),
						   side);
			clock->setPlayerName(name);
		}

		m_tagsModel->setTags(gameData.m_pgn->tags());

		slotUpdateWindowTitle();
		slotUpdateMenus();

		//for (auto evalWidget : m_evalWidgets)
		//	evalWidget->setPlayer(nullptr);

		for (auto evalWidget : m_AnalysisWidget)
			evalWidget->setPlayer(nullptr);

		return;
	}
	else
		m_gameViewer->setGame(m_game);

	m_tagsModel->setTags(gameData.m_pgn->tags());
	gameData.m_pgn->setTagReceiver(m_tagsModel);


	if (m_now_is_match) {       // 引擎比赛   
		for (int i = 0; i < 2; i++)
		{
			Chess::Side side = Chess::Side::Type(i);
			ChessPlayer* player(m_game->player(side));
			m_players[i] = player;

			connect(player, SIGNAL(debugMessage(QString)),
				m_engineDebugLog, SLOT(appendPlainText(QString)));

			auto clock = m_gameViewer->chessClock(side);

			clock->stop();
			QString name = nameOnClock(player->name(), side);
			clock->setPlayerName(name);
			connect(player, SIGNAL(nameChanged(QString)),
				clock, SLOT(setPlayerName(QString)));

			clock->setInfiniteTime(player->timeControl()->isInfinite());

			if (player->state() == ChessPlayer::Thinking)
				clock->start(player->timeControl()->activeTimeLeft());
			else
				clock->setTime(player->timeControl()->timeLeft());

			connect(player, SIGNAL(startedThinking(int)),
				clock, SLOT(start(int)));
			connect(player, SIGNAL(stoppedThinking()),
				clock, SLOT(stop()));
			    
			m_AnalysisWidget[i]->setPlayer(player);
		}
	}
	else {
		for (int i = 0; i < 2; i++)
		{
			Chess::Side side = Chess::Side::Type(i);
			ChessPlayer* player(m_game->player(side));
			m_players[i] = player;

			connect(player, SIGNAL(debugMessage(QString)),
				m_engineDebugLog, SLOT(appendPlainText(QString)));

			auto clock = m_gameViewer->chessClock(side);

			clock->stop();
			QString name = nameOnClock(player->name(), side);
			clock->setPlayerName(name);
			connect(player, SIGNAL(nameChanged(QString)),
				clock, SLOT(setPlayerName(QString)));

			clock->setInfiniteTime(player->timeControl()->isInfinite());

			if (player->state() == ChessPlayer::Thinking)
				clock->start(player->timeControl()->activeTimeLeft());
			else
				clock->setTime(player->timeControl()->timeLeft());

			connect(player, SIGNAL(startedThinking(int)),
				clock, SLOT(start(int)));
			connect(player, SIGNAL(stoppedThinking()),
				clock, SLOT(stop()));

			
			if (player->isHuman() == false) {    // 二个引擎都输到一起
				m_AnalysisWidget[0]->setPlayer(player);
			}			
		}
	}

	

	if (m_game->boardShouldBeFlipped())
		m_gameViewer->boardScene()->flip();

	slotUpdateMenus();
	slotUpdateWindowTitle();
	unlockCurrentGame();
}

int MainWindow::tabIndex(ChessGame* game) const
{
	Q_ASSERT(game != nullptr);

	for (int i = 0; i < m_tabs.size(); i++)
	{
		if (m_tabs.at(i).m_id == game)
			return i;
	}

	return -1;
}

int MainWindow::tabIndex(Tournament* tournament, bool freeTab) const
{
	Q_ASSERT(tournament != nullptr);

	for (int i = 0; i < m_tabs.size(); i++)
	{
		const TabData& tab = m_tabs.at(i);

		if (tab.m_tournament == tournament
		&&  (!freeTab || (tab.m_game == nullptr || tab.m_finished)))
			return i;
	}

	return -1;
}

void MainWindow::onTabChanged(int index)
{
	if (index == -1 || m_closing)
		m_game = nullptr;
	else
		setCurrentGame(m_tabs.at(index));
}

void MainWindow::onTabCloseRequested(int index)
{
	const TabData& tab = m_tabs.at(index);

	if (tab.m_tournament && tab.m_game)
	{
		auto btn = QMessageBox::question(this, tr("结束锦标赛"),
			   tr("您真的要停止当前锦标赛吗?"));
		if (btn != QMessageBox::Yes)
			return;
	}

	slotCloseTab(index);
}

void MainWindow::slotCloseTab(int index)
{
	const TabData& tab = m_tabs.at(index);

	if (tab.m_game == nullptr)
	{
		delete tab.m_pgn;
		removeGame(index);

		if (m_tabs.isEmpty())
			close();

		return;
	}

	//if (m_bAutomaticLinking) {
	//	destroyGame(tab.m_game);
	//	return;
	//}

	if (tab.m_finished)
		slotDestroyGame(tab.m_game);
	else
	{
		connect(tab.m_game, SIGNAL(finished(ChessGame*)),
			this, SLOT(slotDestroyGame(ChessGame*)));
		QMetaObject::invokeMethod(tab.m_game, "stop", Qt::QueuedConnection);
	}
}

void MainWindow::closeCurrentGame()
{
	slotCloseTab(m_tabBar->currentIndex());
}

/*
void MainWindow::setupAnalysisWidget(DockWidgetEx* analysisDock, Chess::AnalysisWidget* analysis)
{
	//return;
	analysisDock->setWidget(analysis);
	// addDockWidget(Qt::RightDockWidgetArea, analysisDock);
	//connect(analysis, SIGNAL(addVariation(Analysis, QString)),
	//	SLOT(slotGameAddVariation(Analysis, QString)));
	//connect(analysis, SIGNAL(addVariation(QString)),
	//	SLOT(slotGameAddVariation(QString)));
//	connect(this, SIGNAL(boardChange(const Board&, const QString&)), analysis, SLOT(setPosition(const Board&, QString)));
	connect(this, SIGNAL(reconfigure()), analysis, SLOT(slotReconfigure()));
	// Make sure engine is disabled if dock is hidden
	connect(analysisDock, SIGNAL(visibilityChanged(bool)),
		analysis, SLOT(slotVisibilityChanged(bool)));
	//m_menuView->addAction(analysisDock->toggleViewAction());
	m_viewMenu->addAction(analysisDock->toggleViewAction());
	analysisDock->hide();
	//connect(this, SIGNAL(signalGameLoaded(const Board&)), analysis, SLOT(slotUciNewGame(const Board&)));
	//connect(this, SIGNAL(signalGameModeChanged(bool)), analysis, SLOT(setDisabled(bool)));
	//connect(this, SIGNAL(signalUpdateDatabaseList(QStringList)), analysis, SLOT(slotUpdateBooks(QStringList)));
	//connect(analysis, SIGNAL(signalSourceChanged(QString)), this, SLOT(slotUpdateOpeningBook(QString)));
	//connect(this, SIGNAL(signalGameModeChanged(bool)), analysis, SLOT(setGameMode(bool)));
}
*/


void MainWindow::slotEditBoard() {
	BoardEditorDlg dlgEditBoard(m_tabs.at(m_tabBar->currentIndex()).m_game->board(), this);
	if (dlgEditBoard.exec() != QDialog::Accepted)
		return;

	QString fen = dlgEditBoard.fenString();
	qDebug() << "编辑fen: " << fen;

	QString variant = m_game.isNull() || m_game->board() == nullptr ?
		"standard" : m_game->board()->variant();

	auto board = Chess::BoardFactory::create(variant);
	board->setFenString(fen);

	//board->legalMoves();
	auto game = new ChessGame(board, new PgnGame());
	game->setTimeControl(TimeControl("inf"));
	game->setStartingFen(fen);
	game->pause();

	connect(game, &ChessGame::initialized, this, &MainWindow::addGame);
	connect(game, &ChessGame::startFailed, this, &MainWindow::onGameStartFailed);

	CuteChessApplication::instance()->gameManager()->newGame(game,
		new HumanBuilder(CuteChessApplication::userName()),
		new HumanBuilder(CuteChessApplication::userName()));
}

void MainWindow::slotNewGame()  // 新建一局游戏
{
	EngineManager* engineManager = CuteChessApplication::instance()->engineManager();
	NewGameDialog dlg(engineManager, this);
	if (dlg.exec() != QDialog::Accepted)             // 如果是取消
		return;

	auto game = dlg.createGame();
	if (!game)
	{
		QMessageBox::critical(this, tr("Could not initialize game"),
				      tr("The game could not be initialized "
					 "due to an invalid opening."));
		return;
	}

	PlayerBuilder* builders[2] = {
		dlg.createPlayerBuilder(Chess::Side::White),
		dlg.createPlayerBuilder(Chess::Side::Black)
	};

	if (builders[game->board()->sideToMove()]->isHuman())
		game->pause();

	// Start the game in a new tab
	connect(game, SIGNAL(initialized(ChessGame*)),
		this, SLOT(addGame(ChessGame*)));
	connect(game, SIGNAL(startFailed(ChessGame*)),
		this, SLOT(onGameStartFailed(ChessGame*)));
	CuteChessApplication::instance()->gameManager()->newGame(game,             // 将这个新棋局添加到Tab表中
		builders[Chess::Side::White], builders[Chess::Side::Black]);
}

void MainWindow::onGameStartFailed(ChessGame* game)
{
	QMessageBox::critical(this, tr("Game Error"), game->errorString());
}

void MainWindow::onGameFinished(ChessGame* game)
{
	int tIndex = tabIndex(game);
	if (tIndex == -1)
		return;

	auto& tab = m_tabs[tIndex];
	tab.m_finished = true;
	QString title = genericTitle(tab);
	m_tabBar->setTabText(tIndex, title);
	if (game == m_game)
	{
		// Finished tournament games are destroyed immediately
		// so we can't touch the game object any more.
		if (tab.m_tournament)
			m_game = nullptr;
		slotUpdateWindowTitle();
		slotUpdateMenus();
	}

	// 保存fen文件，方便导入学习
	//static std::string directory =
	//	CommandLine::BinaryDirectory() + "/data-" + Random::Get().GetString(12);
	// It's fine if it already exists.
	//CreateDirectory(directory.c_str());

	/*
	if (!game->pgn()->isNull()
		&& game->pgn()->moves().length() > 12) {  // 至少5步才保存

		QString runPath = QCoreApplication::applicationDirPath();
		QString expDir = runPath + "/pgngame/";

		QDir* tempDir = new QDir;
		if (!tempDir->exists(expDir)) {
			if (!tempDir->mkdir(expDir)) {		
				QMessageBox::warning(this, tr("错误"), expDir + tr(" 不能新建目录"));
				return ;
			}
		}
		QString fenFile = expDir + Chess::Random::Get().GetString(12) + ".pgn";		
		game->pgn()->writeOnePgnGame(fenFile);
	}
	*/


	// save game notation of non-tournament games to default PGN file
	if (!tab.m_tournament
	&&  !game->pgn()->isNull()
	&&  	(  !game->pgn()->moves().isEmpty()   // ignore empty games
		|| !game->pgn()->result().isNone())) // without adjudication
	{
		QString fileName = QSettings().value("games/default_pgn_output_file", QString())
					      .toString();

		

		if (!fileName.isEmpty()) {

			// 
			QStringList fs = fileName.split(".");
			QDateTime dateTime(QDateTime::currentDateTime());
			QString qStr = dateTime.toString("yyy-MM-dd");

			if (fs.count() >= 2) {

				QString newName = fs[0] + " " + qStr + ".pgn";	
				game->pgn()->write(newName);
			}
		}
			//TODO: reaction on error
	}

	// 联赛结束要不要自动上传
	QSettings s;
	bool autoUpload = s.value("trainFen/AutoUploadFen").toBool();
	if (autoUpload) {
		if (tab.m_tournament) {
			if (tab.m_tournament->playerCount() == 2) {
				//TournamentPlayer fcp = tab.m_tournament->playerAt(0);
				//TournamentPlayer scp = tab.m_tournament->playerAt(1);

				//QString fcp_name = tab.m_tournament->playerAt(0).name();
				//QString scp_name = tab.m_tournament->playerAt(1).name();

				QString w_name = game->pgn()->playerName(Chess::Side::White);
				QString b_name = game->pgn()->playerName(Chess::Side::Black);
					//playerName
				

				switch (game->result().winner()) {
				case Chess::Side::White:
					if (b_name.contains("ggzero", Qt::CaseInsensitive)) {
						this->TrainFenAddLostGame(game);
					}
					break;
				case Chess::Side::Black:
					if (w_name.contains("ggzero", Qt::CaseInsensitive)) {
						this->TrainFenAddLostGame(game);
					}					
					break;			
				default:
					break;
				}

				if (game->result().isDraw()) {
					this->TrainFenAddDrawTooHigh(game);
				}

				//if (game->result().isDrawTruly()){  // 这个是真和棋
				//	this->TrainFenAddDrawTooHigh(game);
				//}
			}
		}
	}

	// 连线结束要不要自动上传
	bool autoUploadLX  = s.value("trainFen/AutoUploadFen").toBool();
	if (autoUploadLX) {
		// 如果当是连线
		//if (game->getIsLinkBoard()) {
		if(game->pgn()->GetLinkCatName() != ""){
			QString w_name = game->pgn()->playerName(Chess::Side::White);
			QString b_name = game->pgn()->playerName(Chess::Side::Black);
			//playerName


			switch (game->result().winner()) {
			case Chess::Side::White:
				if (b_name.contains("ggzero", Qt::CaseInsensitive)) {
					this->TrainFenAddLostGame(game);
				}
				break;
			case Chess::Side::Black:
				if (w_name.contains("ggzero", Qt::CaseInsensitive)) {
					this->TrainFenAddLostGame(game);
				}
				break;
			default:
				break;
			}

			if (game->result().isDraw()) {
				this->TrainFenAddDrawTooHigh(game);
			}
			else if (game->board()->reversibleMoveCount() >= 120) {
				if (!game->board()->GetSendHighScoreDraw()) {
					this->TrainFenAddDrawTooHigh(game);
					game->board()->SetSendHighScoreDraw(true);
				}
			}
		}
	}
}

void MainWindow::slotOpenPgnGame()
{
	
	QString filePGN = QFileDialog::getOpenFileName(this,
		"打开 PGN 格式的棋局",
		QString(),
		tr("PGN 格式 (*.pgn);;All Files (*.*)"));
	

	QFile file(filePGN);
	QFileInfo fileInfo(filePGN);
	if (!fileInfo.exists())
	{
		QMessageBox::information(this, "出错了", "PGN 文件不存在！");
		return;
	}

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::information(this, "出错了", "PGN 不能正常打开！");
		return;
	}

	PgnGame* pgnGame = new PgnGame();
	{
		PgnStream in(&file);		
		if (!pgnGame->read(in)) {
			return;
		}
	}

	QString variant = m_game.isNull() || m_game->board() == nullptr ?
		"standard" : m_game->board()->variant();

	auto board = Chess::BoardFactory::create(variant);
	board->setFenString(board->defaultFenString());

	//auto game = new ChessGame(board, new PgnGame());
	auto game = new ChessGame(board, pgnGame);
	game->setTimeControl(TimeControl("inf"));
	game->pause();

	connect(game, &ChessGame::initialized, this, &MainWindow::addGame);
	connect(game, &ChessGame::startFailed, this, &MainWindow::onGameStartFailed);

	CuteChessApplication::instance()->gameManager()->newGame(game,
		new HumanBuilder(CuteChessApplication::userName()),
		new HumanBuilder(CuteChessApplication::userName()));
}

void MainWindow::slotNewTournament()
{
	NewTournamentDialog dlg(CuteChessApplication::instance()->engineManager(), this);
	if (dlg.exec() != QDialog::Accepted)
		return;

	GameManager* manager = CuteChessApplication::instance()->gameManager();

	Tournament* t = dlg.createTournament(manager);
	auto resultsDialog = CuteChessApplication::instance()->tournamentResultsDialog();
	connect(t, SIGNAL(finished()),
		this, SLOT(onTournamentFinished()));
	connect(t, SIGNAL(gameStarted(ChessGame*, int, int, int)),
		this, SLOT(addGame(ChessGame*)));
	connect(t, SIGNAL(gameFinished(ChessGame*, int, int, int)),
		resultsDialog, SLOT(update()));
	connect(t, SIGNAL(gameFinished(ChessGame*, int, int, int)),
		this, SLOT(onGameFinished(ChessGame*)));
	t->start();

	connect(m_stopTournamentAct, &QAction::triggered, [=]()
	{
		auto btn = QMessageBox::question(this, tr("Stop tournament"),
			   tr("Do you really want to stop the ongoing tournament?"));
		if (btn != QMessageBox::Yes)
		{
			m_closing = false;
			return;
		}

		t->stop();
	});
	m_newTournamentAct->setEnabled(false);
	m_stopTournamentAct->setEnabled(true);
	resultsDialog->setTournament(t);

	m_now_is_match = true;

	//m_AnalysisWidget[i]->setPlayer(player);

	m_whiteEvalDock->setWindowTitle("红方引擎");
	m_blackEvalDock->setWindowTitle("黑方引擎");

	// 清空引擎列表再说
	m_AnalysisWidget[0]->HideAllControl();
	m_AnalysisWidget[1]->HideAllControl();

	// 关闭工具条
	//this->mainToolbar->setVisible(false);

}

void MainWindow::onTournamentFinished()
{
	Tournament* tournament = qobject_cast<Tournament*>(QObject::sender());
	Q_ASSERT(tournament != nullptr);

	m_stopTournamentAct->disconnect();

	QString error = tournament->errorString();
	QString name = tournament->name();

	tournament->deleteLater();
	m_newTournamentAct->setEnabled(true);
	m_stopTournamentAct->setEnabled(false);

	if (m_closing)
	{
		slotCloseAllGames();
		return;
	}

	m_showTournamentResultsAct->trigger();

	if (!error.isEmpty())
	{
		QMessageBox::critical(this,
				      tr("Tournament error"),
				      tr("Tournament \"%1\" finished with an error.\n\n%2")
				      .arg(name, error));
	}

	CuteChessApplication::alert(this);
}

void MainWindow::onWindowMenuAboutToShow()
{
	m_windowMenu->clear();

	addDefaultWindowMenu();
	m_windowMenu->addSeparator();

	const QList<MainWindow*> gameWindows =
		CuteChessApplication::instance()->gameWindows();

	for (int i = 0; i < gameWindows.size(); i++)
	{
		MainWindow* gameWindow = gameWindows.at(i);

		QAction* showWindowAction = m_windowMenu->addAction(
			gameWindow->windowListTitle(), this, SLOT(slotShowGameWindow()));
		showWindowAction->setData(i);
		showWindowAction->setCheckable(true);

		if (gameWindow == this)
			showWindowAction->setChecked(true);
	}
}

void MainWindow::slotShowGameWindow()
{
	if (QAction* action = qobject_cast<QAction*>(sender()))
		CuteChessApplication::instance()->showGameWindow(action->data().toInt());
}

void MainWindow::slotUpdateWindowTitle()
{
	// setWindowTitle() requires "[*]" (see docs)
	const TabData& gameData(m_tabs.at(m_tabBar->currentIndex()));
	setWindowTitle(genericTitle(gameData) + QLatin1String("[*]"));
}

QString MainWindow::windowListTitle() const
{
	const TabData& gameData(m_tabs.at(m_tabBar->currentIndex()));

	#ifndef Q_OS_MAC
	if (isWindowModified())
		return genericTitle(gameData) + QLatin1String("*");
	#endif

	return genericTitle(gameData);
}

bool MainWindow::isMoveValid(const Chess::GenericMove& move){
	return m_gameViewer->isMoveValid(move);
}

//Chess::ChessDB* MainWindow::GetChessDB()
//{
//	if (this->m_ChessDB == nullptr) {
//		QSettings s;
//		if (s.value("ChessDB/useEndGame", true).toBool()) {
//			this->m_ChessDB = new Chess::ChessDB(this);
//			m_ChessDB->on_start();
//			return this->m_ChessDB;
//		}
//		else {
//			return nullptr;
//		}
//	}	
//	
//	return this->m_ChessDB;
//}
	

QString MainWindow::genericTitle(const TabData& gameData) const
{
	QString white;
	QString black;
	Chess::Result result;
	if (gameData.m_game)
	{
		white = gameData.m_game->player(Chess::Side::White)->name();
		black = gameData.m_game->player(Chess::Side::Black)->name();
		result = gameData.m_game->result();
	}
	else
	{
		white = gameData.m_pgn->playerName(Chess::Side::White);
		black = gameData.m_pgn->playerName(Chess::Side::Black);
		result = gameData.m_pgn->result();
	}

	if (result.isNone())
		return tr("%1 vs %2").arg(white, black);
	else
		return tr("%1 vs %2 (%3)")
		       .arg(white, black, result.toShortString());
}

void MainWindow::slotUpdateMenus()
{
	QPointer<ChessPlayer> white = m_players[Chess::Side::White];
	QPointer<ChessPlayer> black = m_players[Chess::Side::Black];
	bool isHumanGame =  (!white.isNull() && white->isHuman())
			 || (!black.isNull() && black->isHuman());
	bool gameOn = (!m_game.isNull() && !m_game->isFinished());
	m_adjudicateBlackWinAct->setEnabled(gameOn);
	m_adjudicateWhiteWinAct->setEnabled(gameOn);
	m_adjudicateDrawAct->setEnabled(gameOn);
	m_resignGameAct->setEnabled(gameOn && isHumanGame);
}

QString MainWindow::nameOnClock(const QString& name, Chess::Side side) const
{
	QString text = name;
	bool displaySide = QSettings().value("ui/display_players_sides_on_clocks", false)
				      .toBool();
	if (displaySide)
		text.append(QString(" (%1)").arg(side.toString()));
	return text;
}

void MainWindow::editMoveComment(int ply, const QString& comment)
{
	bool ok;
	QString text = QInputDialog::getMultiLineText(this, tr("Edit move comment"),
						      tr("Comment:"), comment, &ok);
	if (ok && text != comment)
	{
		lockCurrentGame();
		PgnGame* pgn(m_tabs.at(m_tabBar->currentIndex()).m_pgn);
		PgnGame::MoveData md(pgn->moves().at(ply));
		md.comment = text;
		pgn->setMove(ply, md);
		unlockCurrentGame();

		m_moveList->setMove(ply, md.move, md.moveString, text);
	}
}

// 棋盘上右键菜单
void MainWindow::onMouseRightClicked(/*QGraphicsSceneContextMenuEvent* event*/)
{
	QMenu mymenu;
	mymenu.addAction(m_newGameAct);
	mymenu.addAction(m_saveGameAct);

	mymenu.addSeparator();

	mymenu.addAction(m_copyFenAct);
	mymenu.addAction(m_pasteFenAct);

	mymenu.addSeparator();

	mymenu.addAction(m_changeBoardChessAct);
	mymenu.addAction(m_changeBoardPicAct);	
	mymenu.addAction(m_changeBoardBackGroundAct);

	mymenu.addSeparator();
	
	//mymenu.addAction(tbtnLinkAuto->defaultAction());

	mymenu.exec(QCursor::pos());
	
	//m_viewMenu->addAction(moveListDock->toggleViewAction());
}

void MainWindow::copyFen()
{
	QClipboard* cb = CuteChessApplication::clipboard();
	QString fen(m_gameViewer->board()->fenString());
	if (!fen.isEmpty())
		cb->setText(fen);
}

void MainWindow::pasteFen()
{
	auto cb = CuteChessApplication::clipboard();
	if (cb->text().isEmpty())
		return;

	QString fen = cb->text().trimmed();
	QStringList stFList = fen.split("fen");		// by LGL
	if (stFList.length() > 1) {
		fen = stFList[1].trimmed();		
	}

	QString variant = m_game.isNull() || m_game->board() == nullptr ?
				"standard" : m_game->board()->variant();

	auto board = Chess::BoardFactory::create(variant);
	if (!board->setFenString(fen))
	{
		QMessageBox msgBox(QMessageBox::Critical,
				   tr("FEN error"),
				   tr("Invalid FEN string for the \"%1\" variant:")
				   .arg(variant),
				   QMessageBox::Ok, this);
		msgBox.setInformativeText(cb->text());
		msgBox.exec();

		delete board;
		return;
	}

	//board->legalMoves();

	auto game = new ChessGame(board, new PgnGame());
	game->setTimeControl(TimeControl("inf"));
	game->setStartingFen(fen);
	game->pause();

	connect(game, &ChessGame::initialized, this, &MainWindow::addGame);
	connect(game, &ChessGame::startFailed, this, &MainWindow::onGameStartFailed);

	CuteChessApplication::instance()->gameManager()->newGame(game,
		new HumanBuilder(CuteChessApplication::userName()),
		new HumanBuilder(CuteChessApplication::userName()));
}

void MainWindow::showSettingsDialogX(int page)
{
	Chess::PreferencesDialog dlg(this);
	if (page >= 0)
	{
		dlg.setCurrentIndex(page);
	}
	connect(&dlg, SIGNAL(reconfigure()), SLOT(slotReconfigure()));
	dlg.exec();
}

// 关于菜单
void MainWindow::showAboutDialog()
{
	this->mainToolbar->setVisible(true);
	
	QString html;
	html += "<h3>" + QString("佳佳界面 %1")
		.arg(CuteChessApplication::applicationVersion()) + "</h3>";
	html += "<p>" + tr("Qt 版本 %1").arg(qVersion()) + "</p>";
	html += "<p>" + tr("版本所有 2019-2020 ") + "</p>";
	html += "<p>" + tr("作者 Lee David") + "</p>";
	html += "<p>" + tr("感谢您使用佳佳象棋界面") + "</p>";
	html += "<a href=\"http://elo.ggzero.cn\">GGzero训练网站</a><br>";
	html += "<a href=\"http://bbs.ggzero.cn\">官方论坛</a><br>";
	html += "<a href=\"https://jq.qq.com/?_wv=1027&k=5FxO79E\">加入QQ群</a><br>";
	html += "<br>感谢: 小雪 SuJinpei 蓝天 及其它棋友 <br>";
	QMessageBox::about(this, tr("关于佳佳界面"), html);	
}

// 
void MainWindow::lockCurrentGame()
{
	if (m_game != nullptr)
		m_game->lockThread();
}

void MainWindow::unlockCurrentGame()
{
	if (m_game != nullptr)
		m_game->unlockThread();
}

bool MainWindow::slotSave()
{
	if (m_currentFile.isEmpty())
		return saveAs();

	return saveGame(m_currentFile);
}

bool MainWindow::saveAs()
{
	const QString fileName = QFileDialog::getSaveFileName(
		this,
		tr("保存对局"),
		QString(),
		tr("PGN 格式 (*.pgn);;All Files (*.*)"),
		nullptr,
		QFileDialog::DontConfirmOverwrite);
	if (fileName.isEmpty())
		return false;

	return saveGame(fileName);
}

bool MainWindow::saveGame(const QString& fileName)
{
	lockCurrentGame();
	bool ok = m_tabs.at(m_tabBar->currentIndex()).m_pgn->write(fileName);
	unlockCurrentGame();

	if (!ok)
		return false;

	m_currentFile = fileName;
	setWindowModified(false);

	return true;
}

void MainWindow::copyPgn()
{
	QString str("");
	QTextStream s(&str);
	PgnGame* pgn = m_tabs.at(m_tabBar->currentIndex()).m_pgn;
	if (pgn == nullptr)
		return;
	s << *pgn;

	QClipboard* cb = CuteChessApplication::clipboard();
	cb->setText(s.readAll());
}

void MainWindow::onGameManagerFinished()
{
	m_readyToClose = true;
	close();
}

void MainWindow::slotCloseAllGames()
{
	auto app = CuteChessApplication::instance();
	app->closeDialogs();

	for (int i = m_tabs.size() - 1; i >= 0; i--)
		slotCloseTab(i);

	if (m_tabs.isEmpty())
		app->gameManager()->finish();
}

void MainWindow::slotReconfigure()
{

	Chess::PreferencesDialog::setupIconInMenus(this);

	if (Chess::AppSettings->getValue("/MainWindow/VerticalTabs").toBool())
	{
		setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::VerticalTabs);
	}
	else
	{
		setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks | QMainWindow::AllowNestedDocks);
	}
#ifdef Q_OS_WINXX
	if (AppSettings->getValue("/MainWindow/StayOnTop").toBool())
	{
		SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos((HWND)winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#endif
	//m_recentFiles.restore();
	emit reconfigure(); 	// Re-emit for children
	//delete m_output;
	//m_output = new Output(Output::NotationWidget);
	//UpdateGameText()

}



void MainWindow::closeEvent(QCloseEvent* event)
{
	if (m_readyToClose)
	{
		writeSettings();
		return QMainWindow::closeEvent(event); // clazy:exclude=returning-void-expression
	}

	if (askToSave())
	{
		m_closing = true;

		if (m_stopTournamentAct->isEnabled())
			m_stopTournamentAct->trigger();
		else
			slotCloseAllGames();
	}

	event->ignore();
}

bool MainWindow::askToSave()
{
	if (isWindowModified())
	{
		QMessageBox::StandardButton result;
		result = QMessageBox::warning(this, QApplication::applicationName(),
			tr("对局已有变动.\n你要保存吗?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

		if (result == QMessageBox::Save)
			return slotSave();
		else if (result == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void MainWindow::slotAdjudicateDraw()
{
	adjudicateGame(Chess::Side::NoSide);
}

void MainWindow::slotAdjudicateWhiteWin()
{
	adjudicateGame(Chess::Side::White);
}

void MainWindow::slotAdjudicateBlackWin()
{
	adjudicateGame(Chess::Side::Black);
}

void MainWindow::adjudicateGame(Chess::Side winner)
{
	if (!m_game)
		return;

	auto result = Chess::Result(Chess::Result::Adjudication,
				    winner,
				    tr("用户裁决"));
	QMetaObject::invokeMethod(m_game, "onAdjudication",
				  Qt::QueuedConnection,
				  Q_ARG(Chess::Result, result));
}



OpeningBook* MainWindow::GetOpeningBook(int& depth) const   // 得到开局库的信息
{
	QString file = QSettings().value("games/opening_book/file").toString();
	if (file.isEmpty())
		return nullptr;
	auto mode = OpeningBook::BookRandom;

	bool bestM = QSettings().value("games/opening_book/disk_access").toBool();
	if (bestM) {
		mode = OpeningBook::BookBest;
	}

	depth = QSettings().value("games/opening_book/depth").toInt();

	auto book = new PolyglotBook(mode);

	if (!book->read(file))
	{
		delete book;
		return nullptr;
	}

	return book;
}



QString MainWindow::getUCIHistory() const
{
	return QString();
	/*
	QString line;
	//Game gx = game();
	auto gx = *m_game;
	gx.moveToStart();
	gx.dbMoveToId(m_game->currentMove(), &line);
	return line;
	*/
}

void MainWindow::slotResignGame()
{
	if (m_game.isNull() || m_game->isFinished())
		return;

	ChessPlayer * player = m_game->playerToMove();
	if (!player->isHuman())
	{
		player = m_game->playerToWait();
		if (!player->isHuman())
			return;
	}
	Chess::Side side = player->side();
	auto result = Chess::Result(Chess::Result::Resignation,
				    side.opposite());
	QMetaObject::invokeMethod(m_game, "onResignation",
				  Qt::QueuedConnection,
				  Q_ARG(Chess::Result, result));
}



void MainWindow::slotMoveIntervalChanged(int)
{
}

void MainWindow::slotSetSliderText(int interval)
{
	if (interval < 0)
	{
		//if (m_comboEngine->currentIndex() == 0)
		//{
			interval = m_sliderSpeed->translatedValue();
		//}
		//else
		//{
		//	interval = m_sliderSpeed->value();
		//}
	}
	if (!interval)
	{
		m_sliderText->setText(QString("0s/") + tr("无限"));
	}
	else
	{
		//if (m_comboEngine->currentIndex() == 0)
		//{
			QTime t = QTime::fromMSecsSinceStartOfDay(interval);
			m_sliderText->setText(t.toString("mm:ss"));
		//}
		//else
		//{
		//	m_sliderText->setText(QString::number(interval));
		//}
	}
}

void MainWindow::addDefaultWindowMenu()
{
	m_windowMenu->addAction(m_minimizeAct);
	m_windowMenu->addSeparator();
	m_windowMenu->addAction(m_showGameWallAct);
	m_windowMenu->addSeparator();
	m_windowMenu->addAction(m_showPreviousTabAct);
	m_windowMenu->addAction(m_showNextTabAct);
}
