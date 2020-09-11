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
#include "random.h"

void MainWindow::createDockWindows()
{
	// Engine debug
	QDockWidget* engineDebugDock = new QDockWidget(tr("引擎调试"), this);
	engineDebugDock->setObjectName("EngineDebugDock");
	m_engineDebugLog = new PlainTextLog(engineDebugDock);
	engineDebugDock->setWidget(m_engineDebugLog);
	engineDebugDock->close();
	addDockWidget(Qt::BottomDockWidgetArea, engineDebugDock);

	// Evaluation history
	auto evalHistoryDock = new QDockWidget(tr("历史评估曲线"), this);
	evalHistoryDock->setObjectName("EvalHistoryDock");
	evalHistoryDock->setWidget(m_evalHistory);
	addDockWidget(Qt::BottomDockWidgetArea, evalHistoryDock);


	// Analysis Dock----------------------------------------------------------------------
	/*
	DockWidgetEx* analysisDock = new DockWidgetEx(tr("红方"), this);
	analysisDock->setObjectName("AnalysisDock1");
	analysisDock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_F2);
	//m_mainAnalysis = new Chess::AnalysisWidget(this);
	m_AnalysisWidget[0]->setObjectName("Analysis1");
	setupAnalysisWidget(analysisDock, m_AnalysisWidget[0]);
	addDockWidget(Qt::LeftDockWidgetArea, analysisDock);

	// Analysis Dock 2
	DockWidgetEx* analysisDock2 = new DockWidgetEx(tr("黑方"), this);
	analysisDock2->setObjectName("AnalysisDock2");
	analysisDock2->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_F3);
	//m_secondaryAnalysis = new Chess::AnalysisWidget(this);
	m_AnalysisWidget[1]->setObjectName("Analysis2");
	setupAnalysisWidget(analysisDock2, m_AnalysisWidget[1]);
	addDockWidget(Qt::LeftDockWidgetArea, analysisDock2);
	*/
	//----------------------------------------------------------------------------------------

	// Players' eval widgets
	auto whiteEvalDock = new QDockWidget(tr("主引擎"), this);
	whiteEvalDock->setObjectName("WhiteEvalDock");
	whiteEvalDock->setWidget(m_AnalysisWidget[Chess::Side::White]);   // 不比赛时是主引擎
	addDockWidget(Qt::RightDockWidgetArea, whiteEvalDock);

	connect(this, SIGNAL(reconfigure()), m_AnalysisWidget[0], SLOT(slotReconfigure()));


	auto blackEvalDock = new QDockWidget(tr("副引擎"), this);
	blackEvalDock->setObjectName("BlackEvalDock");
	blackEvalDock->setWidget(m_AnalysisWidget[Chess::Side::Black]);
	addDockWidget(Qt::RightDockWidgetArea, blackEvalDock);

	connect(this, SIGNAL(reconfigure()), m_AnalysisWidget[1], SLOT(slotReconfigure()));

	/*auto whiteEvalDock = new QDockWidget(tr("红方评分"), this);
	whiteEvalDock->setObjectName("WhiteEvalDock");
	whiteEvalDock->setWidget(m_evalWidgets[Chess::Side::White]);
	addDockWidget(Qt::RightDockWidgetArea, whiteEvalDock);
	auto blackEvalDock = new QDockWidget(tr("黑方评分"), this);
	blackEvalDock->setObjectName("BlackEvalDock");
	blackEvalDock->setWidget(m_evalWidgets[Chess::Side::Black]);
	addDockWidget(Qt::RightDockWidgetArea, blackEvalDock);*/

	m_whiteEvalDock = whiteEvalDock;
	m_blackEvalDock = blackEvalDock;


	// Move list
	QDockWidget* moveListDock = new QDockWidget(tr("棋谱"), this);
	moveListDock->setObjectName("MoveListDock");
	moveListDock->setWidget(m_moveList);
	addDockWidget(Qt::RightDockWidgetArea, moveListDock);
	splitDockWidget(moveListDock, whiteEvalDock, Qt::Horizontal);
	splitDockWidget(whiteEvalDock, blackEvalDock, Qt::Vertical);

	// Tags
	QDockWidget* tagsDock = new QDockWidget(tr("标签"), this);
	tagsDock->setObjectName("TagsDock");
	QTreeView* tagsView = new QTreeView(tagsDock);
	tagsView->setModel(m_tagsModel);
	tagsView->setAlternatingRowColors(true);
	tagsView->setRootIsDecorated(false);
	tagsDock->setWidget(tagsView);

	addDockWidget(Qt::RightDockWidgetArea, tagsDock);

	tabifyDockWidget(moveListDock, tagsDock);
	moveListDock->raise();





	// 标注窗口
	 /* Game view
	DockWidgetEx* gameTextDock = new DockWidgetEx(tr("标注"), this);
	gameTextDock->setObjectName("GameTextDock");


	m_gameWindow = new Chess::GameWindow(gameTextDock);
	connect(this, SIGNAL(reconfigure()), m_gameWindow, SLOT(slotReconfigure()));

	//m_gameToolBar = new QToolBar(tr("Game Time"), m_gameWindow);
	//m_gameToolBar->setObjectName("GameToolBar");
	//m_gameToolBar->setMovable(false);
	//m_gameWindow->addToolBar(Qt::BottomToolBarArea, m_gameToolBar);
	//for (int i = 0; i < 2; ++i)
	//{
	//	QLCDNumber* annotatedTime = new QLCDNumber(m_gameToolBar);
	//	annotatedTime->setObjectName(QString("Clock") + QString::number(i));
	//	m_gameToolBar->addWidget(annotatedTime);
	//	annotatedTime->setDigitCount(7);
	//	annotatedTime->setSegmentStyle(QLCDNumber::Flat);
	//	annotatedTime->display("1:00:00");
	//	if (i == 0)
	//	{
	//		ChartWidget* chartWidget = new ChartWidget(m_gameToolBar);
	//		chartWidget->setObjectName("ChartWidget");
	//		chartWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//		connect(chartWidget, SIGNAL(halfMoveRequested(int)), this, SLOT(slotGameMoveToPly(int)));
	//		m_gameToolBar->addWidget(chartWidget);
	//	}
	//}
	//m_menuView->addAction(m_gameToolBar->toggleViewAction());
	//m_gameToolBar->setVisible(AppSettings->getValue("/MainWindow/GameToolBar").toBool());
	m_gameView = m_gameWindow->browser();
	//m_gameView->toolBar = m_gameToolBar;
	connect(m_gameView, SIGNAL(anchorClicked(const QUrl&)), SLOT(slotGameViewLink(const QUrl&)));
	connect(m_gameView, SIGNAL(actionRequested(EditAction)), SLOT(slotGameModify(EditAction)));
	connect(m_gameView, SIGNAL(queryActiveGame(const Game**)), this, SLOT(slotGetActiveGame(const Game**)));
	connect(m_gameView, SIGNAL(signalMergeGame(GameId, QString)), this, SLOT(slotMergeActiveGame(GameId, QString)));
	connect(this, SIGNAL(signalGameLoaded(const Board&)), gameTextDock, SLOT(raise()));
	connect(this, SIGNAL(displayTime(const QString&, Color, const QString&)), m_gameView, SLOT(slotDisplayTime(const QString&, Color, const QString&)));

	gameTextDock->setWidget(m_gameWindow);
	connect(this, SIGNAL(reconfigure()), m_gameView, SLOT(slotReconfigure()));
	addDockWidget(Qt::RightDockWidgetArea, gameTextDock);
	m_gameTitle = new QLabel;
	connect(m_gameTitle, SIGNAL(linkActivated(QString)), this, SLOT(slotGameViewLink(QString)));
	gameTextDock->setTitleBarWidget(m_gameTitle);

	//m_menuView->addAction(gameTextDock->toggleViewAction());
	gameTextDock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_E);
	*/

	// Add toggle view actions to the View menu
	m_viewMenu->addAction(moveListDock->toggleViewAction());
	m_viewMenu->addAction(tagsDock->toggleViewAction());
	m_viewMenu->addAction(engineDebugDock->toggleViewAction());
	m_viewMenu->addAction(evalHistoryDock->toggleViewAction());
	m_viewMenu->addAction(whiteEvalDock->toggleViewAction());
	m_viewMenu->addAction(blackEvalDock->toggleViewAction());

	m_viewMenu->addAction(mainToolbar->toggleViewAction());
}

void MainWindow::createActions()
{
	m_editBoardAct = new QAction(tr("编辑局面"), this);

	m_newGameAct = new QAction(tr("&新建对局..."), this);
	m_newGameAct->setShortcut(QKeySequence::New);

	m_openPgnAct = new QAction(tr("打开 PGN 对局"), this);
	m_openPgnAct->setShortcut(QKeySequence::Open);        //

	m_closeGameAct = new QAction(tr("&关闭"), this);
#ifdef Q_OS_WIN32
	m_closeGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
#else
	m_closeGameAct->setShortcut(QKeySequence::Close);
#endif

	m_saveGameAct = new QAction(tr("&保存对局"), this);
	m_saveGameAct->setShortcut(QKeySequence::Save);

	m_saveGameAsAct = new QAction(tr("&对局另存为..."), this);
	m_saveGameAsAct->setShortcut(QKeySequence::SaveAs);

	m_copyFenAct = new QAction(tr("&复制 FEN"), this);
	QAction* copyFenSequence = new QAction(m_gameViewer);
	copyFenSequence->setShortcut(QKeySequence::Copy);
	copyFenSequence->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	m_gameViewer->addAction(copyFenSequence);

	m_pasteFenAct = new QAction(tr("&粘贴 FEN"), this);
	m_pasteFenAct->setShortcut(QKeySequence(QKeySequence::Paste));

	m_copyPgnAct = new QAction(tr("&复制 PGN"), this);

	m_flipBoardAct = new QAction(tr("&上下翻转棋盘"), this);
	m_flipBoardAct->setShortcut(Qt::CTRL + Qt::Key_F);

	m_adjudicateDrawAct = new QAction(tr("&判定和棋"), this);
	m_adjudicateWhiteWinAct = new QAction(tr("判定红胜"), this);
	m_adjudicateBlackWinAct = new QAction(tr("判定黑胜"), this);

	m_changeBoardPicAct = new QAction(tr("更换棋盘"), this);

	m_changeBoardBackGroundAct = new QAction(tr("更换背景"), this);
	m_changeBoardChessAct = new QAction(tr("更换棋子"), this);

	m_resignGameAct = new QAction(tr("认输"), this);

	m_quitGameAct = new QAction(tr("&退出"), this);
	m_quitGameAct->setMenuRole(QAction::QuitRole);
#ifdef Q_OS_WIN32
	m_quitGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
#else
	m_quitGameAct->setShortcut(QKeySequence::Quit);
#endif

	m_newTournamentAct = new QAction(tr("&新建联赛..."), this);
	m_stopTournamentAct = new QAction(tr("&停止联赛"), this);
	m_showTournamentResultsAct = new QAction(tr("&联赛结果..."), this);

	m_showSettingsAct = new QAction(tr("&通用设置"), this);
	m_showSettingsAct->setMenuRole(QAction::PreferencesRole);

	m_showSettingsActX = new QAction(tr("&其它设置"), this);
	m_showSettingsActX->setMenuRole(QAction::PreferencesRole);

	m_showGameDatabaseWindowAct = new QAction(tr("&对局数据库"), this);

	m_showGameWallAct = new QAction(tr("&当前对局"), this);

	m_minimizeAct = new QAction(tr("&最小化"), this);
	m_minimizeAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));

	m_showPreviousTabAct = new QAction(tr("&显示前一局"), this);
#ifdef Q_OS_MAC
	m_showPreviousTabAct->setShortcut(QKeySequence(Qt::MetaModifier + Qt::ShiftModifier + Qt::Key_Tab));
#else
	m_showPreviousTabAct->setShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Tab));
#endif

	m_showNextTabAct = new QAction(tr("&显示下一局"), this);
#ifdef Q_OS_MAC
	m_showNextTabAct->setShortcut(QKeySequence(Qt::MetaModifier + Qt::Key_Tab));
#else
	m_showNextTabAct->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Tab));
#endif

	m_aboutAct = new QAction(tr("&关于佳佳界面..."), this);
	m_aboutAct->setMenuRole(QAction::AboutRole);

	//this->m_gameViewer->boardScene()->changeBoardPicture();
	connect(m_changeBoardPicAct, SIGNAL(triggered()), this->m_gameViewer->boardScene(), SLOT(OnchangeBoardPicture()));
	connect(m_changeBoardBackGroundAct, SIGNAL(triggered()), this->m_gameViewer->boardScene(), SLOT(OnChangeBackGround()));
	connect(m_changeBoardChessAct, SIGNAL(triggered()), this->m_gameViewer->boardScene(), SLOT(OnChangeChessPicece()));

	connect(m_editBoardAct, SIGNAL(triggered()), this, SLOT(slotEditBoard()));
	connect(m_newGameAct, SIGNAL(triggered()), this, SLOT(slotNewGame()));

	connect(m_openPgnAct, SIGNAL(triggered()), this, SLOT(slotOpenPgnGame()));

	connect(m_copyFenAct, SIGNAL(triggered()), this, SLOT(copyFen()));
	connect(m_pasteFenAct, SIGNAL(triggered()), this, SLOT(pasteFen()));
	connect(copyFenSequence, SIGNAL(triggered()), this, SLOT(copyFen()));
	connect(m_copyPgnAct, SIGNAL(triggered()), this, SLOT(copyPgn()));
	connect(m_flipBoardAct, SIGNAL(triggered()),
		m_gameViewer->boardScene(), SLOT(flip()));
	connect(m_closeGameAct, &QAction::triggered, this, [=]()
		{
			auto focusWindow = CuteChessApplication::activeWindow();
			if (!focusWindow)
				return;

			auto focusMainWindow = qobject_cast<MainWindow*>(focusWindow);
			if (focusMainWindow)
			{
				focusMainWindow->closeCurrentGame();
				return;
			}

			focusWindow->close();
		});

	auto app = CuteChessApplication::instance();

	connect(m_saveGameAct, SIGNAL(triggered()), this, SLOT(slotSave()));
	connect(m_saveGameAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	connect(m_adjudicateDrawAct, SIGNAL(triggered()), this, SLOT(slotAdjudicateDraw()));
	connect(m_adjudicateWhiteWinAct, SIGNAL(triggered()), this, SLOT(slotAdjudicateWhiteWin()));
	connect(m_adjudicateBlackWinAct, SIGNAL(triggered()), this, SLOT(slotAdjudicateBlackWin()));

	connect(m_resignGameAct, SIGNAL(triggered()), this, SLOT(slotResignGame()));

	connect(m_quitGameAct, &QAction::triggered,
		app, &CuteChessApplication::onQuitAction);

	connect(m_newTournamentAct, SIGNAL(triggered()), this, SLOT(slotNewTournament()));

	connect(m_minimizeAct, &QAction::triggered, this, [=]()
		{
			auto focusWindow = app->focusWindow();
			if (focusWindow != nullptr)
			{
				focusWindow->showMinimized();
			}
		});

	connect(m_showSettingsAct, SIGNAL(triggered()),
		app, SLOT(showSettingsDialog()));

	connect(m_showSettingsActX, SIGNAL(triggered()),
		this, SLOT(showSettingsDialogX()));

	connect(m_showTournamentResultsAct, SIGNAL(triggered()),
		app, SLOT(showTournamentResultsDialog()));

	connect(m_showGameDatabaseWindowAct, SIGNAL(triggered()),
		app, SLOT(showGameDatabaseDialog()));

	connect(m_showGameWallAct, SIGNAL(triggered()),
		app, SLOT(showGameWall()));

	connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

void MainWindow::createMenus()
{
	m_gameMenu = menuBar()->addMenu(tr("&游戏"));
	m_gameMenu->addAction(m_editBoardAct);
	m_gameMenu->addAction(m_newGameAct);
	m_gameMenu->addAction(m_openPgnAct);
	m_gameMenu->addSeparator();
	m_gameMenu->addAction(m_closeGameAct);
	m_gameMenu->addAction(m_saveGameAct);
	m_gameMenu->addAction(m_saveGameAsAct);
	m_gameMenu->addSeparator();
	m_gameMenu->addAction(m_copyFenAct);
	m_gameMenu->addAction(m_copyPgnAct);
	m_gameMenu->addAction(m_pasteFenAct);
	m_gameMenu->addSeparator();
	m_gameMenu->addAction(m_adjudicateDrawAct);
	m_gameMenu->addAction(m_adjudicateWhiteWinAct);
	m_gameMenu->addAction(m_adjudicateBlackWinAct);
	m_gameMenu->addSeparator();
	m_gameMenu->addAction(m_resignGameAct);
	m_gameMenu->addSeparator();
	m_gameMenu->addAction(m_quitGameAct);

	m_tournamentMenu = menuBar()->addMenu(tr("&联赛"));
	m_tournamentMenu->addAction(m_newTournamentAct);
	m_tournamentMenu->addAction(m_stopTournamentAct);
	m_tournamentMenu->addAction(m_showTournamentResultsAct);
	m_tournamentMenu->addSeparator();
	//m_tournamentMenu->addAction(m_showSettingsAct);

	m_stopTournamentAct->setEnabled(false);

	m_toolsMenu = menuBar()->addMenu(tr("&设置"));
	m_toolsMenu->addAction(m_showSettingsAct);
	m_toolsMenu->addAction(m_showSettingsActX);

	m_toolsMenu->addAction(m_showGameDatabaseWindowAct);

	m_viewMenu = menuBar()->addMenu(tr("&视图"));
	m_viewMenu->addAction(m_flipBoardAct);
	m_viewMenu->addSeparator();

	m_windowMenu = menuBar()->addMenu(tr("&窗口"));
	addDefaultWindowMenu();

	connect(m_windowMenu, SIGNAL(aboutToShow()), this,
		SLOT(onWindowMenuAboutToShow()));

	m_helpMenu = menuBar()->addMenu(tr("&帮助"));
	m_helpMenu->addAction(m_aboutAct);
}

void MainWindow::createToolBars()
{
	m_tabBar = new GameTabBar();
	m_tabBar->setDocumentMode(true);
	m_tabBar->setTabsClosable(true);
	m_tabBar->setMovable(false);
	m_tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	connect(m_tabBar, SIGNAL(currentChanged(int)),
		this, SLOT(onTabChanged(int)));
	connect(m_tabBar, SIGNAL(tabCloseRequested(int)),
		this, SLOT(onTabCloseRequested(int)));
	connect(m_showPreviousTabAct, SIGNAL(triggered()),
		m_tabBar, SLOT(showPreviousTab()));
	connect(m_showNextTabAct, SIGNAL(triggered()),
		m_tabBar, SLOT(showNextTab()));

	QToolBar* toolBar = new QToolBar(tr("Game Tabs"));
	toolBar->setObjectName("GameTabs");
	toolBar->setVisible(false);
	toolBar->setFloatable(false);
	toolBar->setMovable(false);
	toolBar->setAllowedAreas(Qt::TopToolBarArea);
	toolBar->addWidget(m_tabBar);
	addToolBar(toolBar);


	//QAction* actLinkChessBoard;   // 连接其它棋盘
	//QAction* actEngineThink;      // 让引擎思考
	//QAction* actEngineStop;       // 让引擎停止
	//QAction* actEngineAnalyze;    // 让引擎分析

	// 主菜单工具条
	this->mainToolbar = new QToolBar(this);
	this->mainToolbar->setObjectName(QStringLiteral("mainToolBar"));
	this->mainToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly); //  ToolButtonTextUnderIcon); ToolButtonIconOnly
	this->mainToolbar->setMovable(true);
	this->mainToolbar->setAllowedAreas(Qt::TopToolBarArea);
	this->mainToolbar->setVisible(true);
	this->addToolBar(Qt::TopToolBarArea, this->mainToolbar);

	this->mainToolbar->setWindowTitle("工具栏");

	// 引擎执红
	this->tbtnEnginePlayRed = new QToolButton(this);
	this->tbtnEnginePlayRed->setCheckable(true);
	this->tbtnEnginePlayRed->setObjectName(QStringLiteral("EnginePlayRed"));
	QIcon iconEnginePlayRed;
	iconEnginePlayRed.addFile(QStringLiteral(":/icon/com_red.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnEnginePlayRed->setIcon(iconEnginePlayRed);
	this->tbtnEnginePlayRed->setToolTip("电脑执红走棋");
	this->mainToolbar->addWidget(this->tbtnEnginePlayRed);
	connect(this->tbtnEnginePlayRed, SIGNAL(toggled(bool)), this, SLOT(onPlayRedToggled(bool)));

	// 引擎执黑
	this->tbtnEnginePlayBlack = new QToolButton(this);
	this->tbtnEnginePlayBlack->setCheckable(true);
	this->tbtnEnginePlayBlack->setObjectName(QStringLiteral("EnginePlayBlack"));
	QIcon iconEnginePlayBlack;
	iconEnginePlayBlack.addFile(QStringLiteral(":/icon/com_blue.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnEnginePlayBlack->setIcon(iconEnginePlayBlack);
	this->tbtnEnginePlayBlack->setToolTip("电脑执黑走棋");
	this->mainToolbar->addWidget(this->tbtnEnginePlayBlack);
	connect(this->tbtnEnginePlayBlack, SIGNAL(toggled(bool)), this, SLOT(onPlayBlackToggled(bool)));
	//tbtnTrainFenTimerDelOne

	QWidget* empty2 = new QWidget();
	empty2->setFixedSize(2, 20);
	empty2->setStyleSheet(QString::fromUtf8("border:1px solid gray"));
	this->mainToolbar->addWidget(empty2);

	// 让引擎立即出步
	this->actEngineStop = new QAction(this);
	this->actEngineStop->setObjectName(QStringLiteral("EngineStop"));
	QIcon iconEngineStop;
	iconEngineStop.addFile(QStringLiteral(":/icon/stop.ico"), QSize(), QIcon::Normal, QIcon::Off);
	this->actEngineStop->setIcon(iconEngineStop);
	this->actEngineStop->setText("立即出步");
	this->actEngineStop->setToolTip("让引擎立即出步");
	//this->mainToolbar->addAction(this->actEngineStop);

	// 连接其它棋盘，红方走棋
	this->tbtnLinkChessBoardRed = new QToolButton(this);
	this->tbtnLinkChessBoardRed->setCheckable(true);
	this->tbtnLinkChessBoardRed->setObjectName(QStringLiteral("LinkChessBoardRed"));
	QIcon iconLinkChessBoardRed;
	iconLinkChessBoardRed.addFile(QStringLiteral(":/icon/RedLink.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnLinkChessBoardRed->setIcon(iconLinkChessBoardRed);
	this->tbtnLinkChessBoardRed->setText("红方连线");
	this->tbtnLinkChessBoardRed->setToolTip("连接其它棋盘, 我方执红");
	this->mainToolbar->addWidget(this->tbtnLinkChessBoardRed);
	connect(this->tbtnLinkChessBoardRed, SIGNAL(toggled(bool)), this, SLOT(onLinkRedToggled(bool)));


	//onLinkRedToggled

	// 连接其它棋盘，黑方走棋
	this->tbtnLinkChessBoardBlack = new QToolButton(this);
	this->tbtnLinkChessBoardBlack->setCheckable(true);
	this->tbtnLinkChessBoardBlack->setObjectName(QStringLiteral("LinkChessBoardBlack"));
	QIcon iconLinkChessBoardBlack;
	iconLinkChessBoardBlack.addFile(QStringLiteral(":/icon/BlackLink.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnLinkChessBoardBlack->setIcon(iconLinkChessBoardBlack);
	this->tbtnLinkChessBoardBlack->setText("红方连线");
	this->tbtnLinkChessBoardBlack->setToolTip("连接其它棋盘, 我方执黑");
	this->mainToolbar->addWidget(this->tbtnLinkChessBoardBlack);
	connect(this->tbtnLinkChessBoardBlack, SIGNAL(toggled(bool)), this, SLOT(onLinkBlackToggled(bool)));

	// 全自动连接其它棋盘
	this->tbtnLinkAuto = new QToolButton(this);
	this->tbtnLinkAuto->setCheckable(true);
	this->tbtnLinkAuto->setObjectName(QStringLiteral("LinkChessBoardAuto"));
	QIcon iconLinkChessBoardAuto;
	iconLinkChessBoardAuto.addFile(QStringLiteral(":/icon/autolink.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnLinkAuto->setIcon(iconLinkChessBoardAuto);
	this->tbtnLinkAuto->setText("全自动连线");
	this->tbtnLinkAuto->setToolTip("全自动连接其它棋盘");
	this->mainToolbar->addWidget(this->tbtnLinkAuto);
	connect(this->tbtnLinkAuto, SIGNAL(toggled(bool)), this, SLOT(onLinkAutomaticToggled(bool)));


	// 盲区专用

	/*

	QWidget* empty = new QWidget();
	empty->setFixedSize(10, 20);
	this->mainToolbar->addWidget(empty);

	//QComboBox* cbtnLinkBoard;            // 连线的棋盘
	this->cbtnLinkBoard = new QComboBox(this);
	this->cbtnLinkBoard->setObjectName(QStringLiteral("cbtnLinkBoard"));
	this->cbtnLinkBoard->setToolTip("改变连线方案");
	QStringList strList;
	//strList << "天天象棋" << "王者象棋" ;
	strList << "王者象棋" << "天天象棋";
	this->cbtnLinkBoard->addItems(strList);
	connect(this->cbtnLinkBoard, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onLinkBoardCombox(const QString&)));
	this->mainToolbar->addWidget(this->cbtnLinkBoard);


	//int sel = this->cbtnLinkBoard->currentIndex();
	//QSettings().setValue("ui/linkboard_curSel", sel);

	int sel = QSettings().value("ui/linkboard_curSel").toInt();
	this->cbtnLinkBoard->setCurrentIndex(sel);


	QWidget* empty2 = new QWidget();
	empty2->setFixedSize(10, 20);
	this->mainToolbar->addWidget(empty2);

	//cbtnLinkEngine
	this->cbtnLinkEngine = new QComboBox(this);
	this->cbtnLinkEngine->setObjectName(QStringLiteral("cbtnLinkEngine"));
	this->cbtnLinkEngine->setToolTip("选择连线的引擎");

	//
	EngineManager* m_engineManager =
		CuteChessApplication::instance()->engineManager();

	for (int i = 0; i < m_engineManager->engineCount(); i++) {
		this->cbtnLinkEngine->addItem(m_engineManager->engineAt(i).name());
	}

	//QSet<QString> qset = m_engineManager->engineNames();
	//for (auto v : qset) {
	//	this->cbtnLinkEngine->addItem(v);
	//}
	this->mainToolbar->addWidget(this->cbtnLinkEngine);

	sel = QSettings().value("ui/linkboard_curEngine").toInt();
	this->cbtnLinkEngine->setCurrentIndex(sel);

	*/


	//QSettings().setValue("ui/linkboard", true);

	//bool b = QSettings().value("ui/linkboard").toBool();

	//int a = 0;

	//http://c.biancheng.net/view/1849.html

	//QAction* actLinkChessBoardRed;      // 连接其它棋盘
	//QAction* actLinkChessBoardBlack;    // 连接其它棋盘

	//// 连接其它棋盘
	//this->actLinkChessBoard = new QAction(this);
	//this->actLinkChessBoard->setObjectName(QStringLiteral("LinkChessBoard"));
	//QIcon iconLinkChessBoard;
	//iconLinkChessBoard.addFile(QStringLiteral(":/icon/Links.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actLinkChessBoard->setIcon(iconLinkChessBoard);
	//this->actLinkChessBoard->setText("连线");
	//this->actLinkChessBoard->setToolTip("连接其它棋盘");

	//// 让引擎思考
	//this->actEngineThink = new QAction(this);
	//this->actEngineThink->setObjectName(QStringLiteral("EngineThink"));
	//QIcon iconEngineThink;
	//iconEngineThink.addFile(QStringLiteral(":/icon/thought-balloon.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actEngineThink->setIcon(iconEngineThink);
	//this->actEngineThink->setText("思考");
	//this->actEngineThink->setToolTip("让引擎思考当前棋局，并自动走棋");

	//this->mainToolbar->addAction(this->actEngineThink);
	//connect(this->actEngineThink, &QAction::triggered, this, &MainWindow::onActEngineThink);

	//*****************************************************************************************************
	//QSettings 
	QSettings s;  //superAdmin
	QString userName = s.value("trainFen/UserName", "").toString();
	if (userName.length() > 3) {
		QWidget* empty3 = new QWidget();
		empty3->setFixedSize(2, 20);
		empty3->setStyleSheet(QString::fromUtf8("border:1px solid gray"));
		this->mainToolbar->addWidget(empty3);

		// 定时删除一个FEN，防止过度训练
		this->tbtnTrainFenTimerDelOne = new QToolButton(this);
		this->tbtnTrainFenTimerDelOne->setCheckable(true);
		this->tbtnTrainFenTimerDelOne->setObjectName(QStringLiteral("tbtnTrainFenTimerDelOne"));
		QIcon iconTrainFenTimerDelOne;
		iconTrainFenTimerDelOne.addFile(QStringLiteral(":/icon/update.ico"), QSize(), QIcon::Normal, QIcon::Off);
		this->tbtnTrainFenTimerDelOne->setIcon(iconTrainFenTimerDelOne);
		this->tbtnTrainFenTimerDelOne->setToolTip("定时删除一个FEN，防止过度训练");
		this->mainToolbar->addWidget(this->tbtnTrainFenTimerDelOne);
		connect(this->tbtnTrainFenTimerDelOne, SIGNAL(toggled(bool)), this, SLOT(onTrainFenTimerDelOne(bool)));
		//tbtnTrainFenTimerDelOne

		// 上传当前棋局的Fen Add
		this->actTrainFenAdd = new QAction(this);
		this->actTrainFenAdd->setObjectName(QStringLiteral("actTrainFenAdd"));
		QIcon iconactTrainFenAdd;
		iconactTrainFenAdd.addFile(QStringLiteral(":/icon/add.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenAdd->setIcon(iconactTrainFenAdd);
		this->actTrainFenAdd->setText("上传Fen");
		this->actTrainFenAdd->setToolTip("将当前棋局的Fen上传至服务器训练");

		this->mainToolbar->addAction(this->actTrainFenAdd);
		connect(this->actTrainFenAdd, &QAction::triggered, this, &MainWindow::onTrainFenAdd);


		// 上传高分和棋的局面
		this->actTrainFenDrawTooHigh = new QAction(this);
		this->actTrainFenDrawTooHigh->setObjectName(QStringLiteral("actTrainFenDrawTooHigh"));
		QIcon iconactTrainFenDrawTooHigh;
		iconactTrainFenDrawTooHigh.addFile(QStringLiteral(":/icon/dove_green.png"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenDrawTooHigh->setIcon(iconactTrainFenDrawTooHigh);
		this->actTrainFenDrawTooHigh->setText("上传高分和棋Fen");
		this->actTrainFenDrawTooHigh->setToolTip("上传引擎显示高分，但其实和棋的局面");

		this->mainToolbar->addAction(this->actTrainFenDrawTooHigh);
		connect(this->actTrainFenDrawTooHigh, &QAction::triggered, this, &MainWindow::onTrainDrawToHigh);

		// 删除当前棋局的Fen Delete
		this->actTrainFenDelete = new QAction(this);
		this->actTrainFenDelete->setObjectName(QStringLiteral("actTrainFenDelete"));
		QIcon iconactTrainFenDelete;
		iconactTrainFenDelete.addFile(QStringLiteral(":/icon/del.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenDelete->setIcon(iconactTrainFenDelete);
		this->actTrainFenDelete->setText("删除上传的Fen");
		this->actTrainFenDelete->setToolTip("将此前上传至服务器的Fen删除");

		this->mainToolbar->addAction(this->actTrainFenDelete);
		connect(this->actTrainFenDelete, &QAction::triggered, this, &MainWindow::onTrainFenDelete);

		// 上传常用的的Fen CommonFen
		this->actTrainFenAddCommonFen = new QAction(this);
		this->actTrainFenAddCommonFen->setObjectName(QStringLiteral("actTrainFenAddCommonFen"));
		QIcon iconactTrainFenAddCommonFen;
		iconactTrainFenAddCommonFen.addFile(QStringLiteral(":/icon/commmon.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenAddCommonFen->setIcon(iconactTrainFenAddCommonFen);
		this->actTrainFenAddCommonFen->setText("上传常用Fen");
		this->actTrainFenAddCommonFen->setToolTip("上传常用Fen至服务器，增加开局多样性");

		this->mainToolbar->addAction(this->actTrainFenAddCommonFen);
		connect(this->actTrainFenAddCommonFen, &QAction::triggered, this, &MainWindow::onTrainFenAddCommon);

		// 清除上传所有的Fen ClearAll
		this->actTrainFenClearAll = new QAction(this);
		this->actTrainFenClearAll->setObjectName(QStringLiteral("actTrainFenClearAll"));
		QIcon iconactTrainFenClearAll;
		iconactTrainFenClearAll.addFile(QStringLiteral(":/icon/remove_all.png"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenClearAll->setIcon(iconactTrainFenClearAll);
		this->actTrainFenClearAll->setText("清除所有的Fen");
		this->actTrainFenClearAll->setToolTip("将服务器上的Fen全部清除");

		this->mainToolbar->addAction(this->actTrainFenClearAll);
		connect(this->actTrainFenClearAll, &QAction::triggered, this, &MainWindow::onTrainFenClearAll);


		// actTrainFenUpdateLast30
		this->actTrainFenUpdateLast50 = new QAction(this);
		this->actTrainFenUpdateLast50->setObjectName(QStringLiteral("actTrainFenUpdateLast30"));
		QIcon iconUpdateLast30;
		iconUpdateLast30.addFile(QStringLiteral(":/icon/up50.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenUpdateLast50->setIcon(iconUpdateLast30);
		this->actTrainFenUpdateLast50->setText("上传最后30 Fen");
		this->actTrainFenUpdateLast50->setToolTip("无条件上传当前棋局的最后30个Fen");

		this->mainToolbar->addAction(this->actTrainFenUpdateLast50);
		connect(this->actTrainFenUpdateLast50, &QAction::triggered, this, &MainWindow::onTrainFenLast50);
	}

	//*****************************************************************************************************


	QWidget* empty4 = new QWidget();
	empty4->setFixedSize(2, 20);
	empty4->setStyleSheet(QString::fromUtf8("border:1px solid gray"));
	this->mainToolbar->addWidget(empty4);


	//// 让引擎思考
	this->actEngineThink = new QAction(this);
	this->actEngineThink->setObjectName(QStringLiteral("EngineThink"));
	QIcon iconEngineThink;
	iconEngineThink.addFile(QStringLiteral(":/icon/thought-balloon.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->actEngineThink->setIcon(iconEngineThink);
	this->actEngineThink->setText("思考");
	this->actEngineThink->setToolTip("让引擎思考当前棋局，并自动走棋");

	this->mainToolbar->addAction(this->actEngineThink);
	connect(this->actEngineThink, &QAction::triggered, this, &MainWindow::onActEngineThink);

	//*****************************************************************************************************




	//// 让引擎分析
	//this->actEngineAnalyze = new QAction(this);
	//this->actEngineAnalyze->setObjectName(QStringLiteral("EngineAnalyze"));
	//QIcon iconEngineAnalyze;
	//iconEngineAnalyze.addFile(QStringLiteral(":/icon/analyze.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actEngineAnalyze->setIcon(iconEngineAnalyze);
	//this->actEngineAnalyze->setText("分析");
	//this->actEngineAnalyze->setToolTip("让引擎分析当前棋局");



	//// 引擎设置
	//this->actEngineSetting = new QAction(this);
	//this->actEngineSetting->setObjectName(QStringLiteral("EngineSetting"));
	//QIcon iconEngineSetting;
	//iconEngineSetting.addFile(QStringLiteral(":/icon/Settings.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actEngineSetting->setIcon(iconEngineSetting);
	//this->actEngineSetting->setText("设置");
	//this->actEngineSetting->setToolTip("设置引擎参数");

	//this->mainToolbar->addAction(this->actLinkChessBoard);
	//this->mainToolbar->addAction(this->actEngineThink);
	//this->mainToolbar->addAction(this->actEngineAnalyze);
	//this->mainToolbar->addAction(this->actEngineStop);   
	//this->mainToolbar->addAction(this->actEngineSetting);   


	//connect(this->actLinkChessBoard, &QAction::triggered, this, &MainWindow::onLXchessboard);


	//connect(m_tabBar, SIGNAL(currentChanged(int)),
	//	this, SLOT(onTabChanged(int)));
}

QString MainWindow::preverb()
{
	static const QString list[] = {
			 "Less is more.",
			 "Where there is s will，there is a way.",
			 "No pains，no gains.",
			 "Time and tide wait for no man.",
			 "Strike while the iron is hot.",
			 "It‘s never too late to mend.",
			 "There is no smoke without fire.",
			 "We never know the worth of water till the well is dry.",
			 "Seeing is believing.",
			 "Well begun is half done.",
			 "Time flies never to be recalled.",
			 "When in Rome, do as Roman do.",
			 "He laughs best who laughs last.",
			 "Haste makes waste.",
			 "No weal without woe.",
			 "Absence sharpens love, presence strengthens it.",
			 "Pain past is pleasure.",
			 "A burden of one's choice is not felt.",
			 "Adversity leads to prosperity.",
			 "A faithful friend is hard to find.",
			 "A friend is easier lost than found.",
			 "A friend without faults will never be found.",
			 "A good book is a good friend.",
			 "A good fame is better than a good face.",
			 "A hedge between keeps friendship green.",
			 "A little body often harbors a great soul.",
			 "All rivers run into sea.",
			 "All that ends well is well.",
			 "All that glitters is not gold.",
			 "A man becomes learned by asking questions.",
			 "A man is known by his friends.",
			 "A merry heart goes all the way.",
			 "A miss is as good as a mile.",
			 "An hour in the morning is worth two in the evening.",
			 "A still tongue makes a wise head.",
			 "A word spoken is past recalling.",
			 "Beauty lies in the love‘s eyes.",
			 "Between friends all is common.",
			 "Cannot see the wood for the trees.",
			 "Care and diligence bring luck."
	};
	constexpr int a = sizeof(list) / sizeof(QString);
	return list[Chess::Random::Get().GetInt(0, a - 1)];
}
