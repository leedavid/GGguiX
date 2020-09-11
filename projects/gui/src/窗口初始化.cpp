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
	QDockWidget* engineDebugDock = new QDockWidget(tr("�������"), this);
	engineDebugDock->setObjectName("EngineDebugDock");
	m_engineDebugLog = new PlainTextLog(engineDebugDock);
	engineDebugDock->setWidget(m_engineDebugLog);
	engineDebugDock->close();
	addDockWidget(Qt::BottomDockWidgetArea, engineDebugDock);

	// Evaluation history
	auto evalHistoryDock = new QDockWidget(tr("��ʷ��������"), this);
	evalHistoryDock->setObjectName("EvalHistoryDock");
	evalHistoryDock->setWidget(m_evalHistory);
	addDockWidget(Qt::BottomDockWidgetArea, evalHistoryDock);


	// Analysis Dock----------------------------------------------------------------------
	/*
	DockWidgetEx* analysisDock = new DockWidgetEx(tr("�췽"), this);
	analysisDock->setObjectName("AnalysisDock1");
	analysisDock->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_F2);
	//m_mainAnalysis = new Chess::AnalysisWidget(this);
	m_AnalysisWidget[0]->setObjectName("Analysis1");
	setupAnalysisWidget(analysisDock, m_AnalysisWidget[0]);
	addDockWidget(Qt::LeftDockWidgetArea, analysisDock);

	// Analysis Dock 2
	DockWidgetEx* analysisDock2 = new DockWidgetEx(tr("�ڷ�"), this);
	analysisDock2->setObjectName("AnalysisDock2");
	analysisDock2->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_F3);
	//m_secondaryAnalysis = new Chess::AnalysisWidget(this);
	m_AnalysisWidget[1]->setObjectName("Analysis2");
	setupAnalysisWidget(analysisDock2, m_AnalysisWidget[1]);
	addDockWidget(Qt::LeftDockWidgetArea, analysisDock2);
	*/
	//----------------------------------------------------------------------------------------

	// Players' eval widgets
	auto whiteEvalDock = new QDockWidget(tr("������"), this);
	whiteEvalDock->setObjectName("WhiteEvalDock");
	whiteEvalDock->setWidget(m_AnalysisWidget[Chess::Side::White]);   // ������ʱ��������
	addDockWidget(Qt::RightDockWidgetArea, whiteEvalDock);

	connect(this, SIGNAL(reconfigure()), m_AnalysisWidget[0], SLOT(slotReconfigure()));


	auto blackEvalDock = new QDockWidget(tr("������"), this);
	blackEvalDock->setObjectName("BlackEvalDock");
	blackEvalDock->setWidget(m_AnalysisWidget[Chess::Side::Black]);
	addDockWidget(Qt::RightDockWidgetArea, blackEvalDock);

	connect(this, SIGNAL(reconfigure()), m_AnalysisWidget[1], SLOT(slotReconfigure()));

	/*auto whiteEvalDock = new QDockWidget(tr("�췽����"), this);
	whiteEvalDock->setObjectName("WhiteEvalDock");
	whiteEvalDock->setWidget(m_evalWidgets[Chess::Side::White]);
	addDockWidget(Qt::RightDockWidgetArea, whiteEvalDock);
	auto blackEvalDock = new QDockWidget(tr("�ڷ�����"), this);
	blackEvalDock->setObjectName("BlackEvalDock");
	blackEvalDock->setWidget(m_evalWidgets[Chess::Side::Black]);
	addDockWidget(Qt::RightDockWidgetArea, blackEvalDock);*/

	m_whiteEvalDock = whiteEvalDock;
	m_blackEvalDock = blackEvalDock;


	// Move list
	QDockWidget* moveListDock = new QDockWidget(tr("����"), this);
	moveListDock->setObjectName("MoveListDock");
	moveListDock->setWidget(m_moveList);
	addDockWidget(Qt::RightDockWidgetArea, moveListDock);
	splitDockWidget(moveListDock, whiteEvalDock, Qt::Horizontal);
	splitDockWidget(whiteEvalDock, blackEvalDock, Qt::Vertical);

	// Tags
	QDockWidget* tagsDock = new QDockWidget(tr("��ǩ"), this);
	tagsDock->setObjectName("TagsDock");
	QTreeView* tagsView = new QTreeView(tagsDock);
	tagsView->setModel(m_tagsModel);
	tagsView->setAlternatingRowColors(true);
	tagsView->setRootIsDecorated(false);
	tagsDock->setWidget(tagsView);

	addDockWidget(Qt::RightDockWidgetArea, tagsDock);

	tabifyDockWidget(moveListDock, tagsDock);
	moveListDock->raise();





	// ��ע����
	 /* Game view
	DockWidgetEx* gameTextDock = new DockWidgetEx(tr("��ע"), this);
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
	m_editBoardAct = new QAction(tr("�༭����"), this);

	m_newGameAct = new QAction(tr("&�½��Ծ�..."), this);
	m_newGameAct->setShortcut(QKeySequence::New);

	m_openPgnAct = new QAction(tr("�� PGN �Ծ�"), this);
	m_openPgnAct->setShortcut(QKeySequence::Open);        //

	m_closeGameAct = new QAction(tr("&�ر�"), this);
#ifdef Q_OS_WIN32
	m_closeGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
#else
	m_closeGameAct->setShortcut(QKeySequence::Close);
#endif

	m_saveGameAct = new QAction(tr("&����Ծ�"), this);
	m_saveGameAct->setShortcut(QKeySequence::Save);

	m_saveGameAsAct = new QAction(tr("&�Ծ����Ϊ..."), this);
	m_saveGameAsAct->setShortcut(QKeySequence::SaveAs);

	m_copyFenAct = new QAction(tr("&���� FEN"), this);
	QAction* copyFenSequence = new QAction(m_gameViewer);
	copyFenSequence->setShortcut(QKeySequence::Copy);
	copyFenSequence->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	m_gameViewer->addAction(copyFenSequence);

	m_pasteFenAct = new QAction(tr("&ճ�� FEN"), this);
	m_pasteFenAct->setShortcut(QKeySequence(QKeySequence::Paste));

	m_copyPgnAct = new QAction(tr("&���� PGN"), this);

	m_flipBoardAct = new QAction(tr("&���·�ת����"), this);
	m_flipBoardAct->setShortcut(Qt::CTRL + Qt::Key_F);

	m_adjudicateDrawAct = new QAction(tr("&�ж�����"), this);
	m_adjudicateWhiteWinAct = new QAction(tr("�ж���ʤ"), this);
	m_adjudicateBlackWinAct = new QAction(tr("�ж���ʤ"), this);

	m_changeBoardPicAct = new QAction(tr("��������"), this);

	m_changeBoardBackGroundAct = new QAction(tr("��������"), this);
	m_changeBoardChessAct = new QAction(tr("��������"), this);

	m_resignGameAct = new QAction(tr("����"), this);

	m_quitGameAct = new QAction(tr("&�˳�"), this);
	m_quitGameAct->setMenuRole(QAction::QuitRole);
#ifdef Q_OS_WIN32
	m_quitGameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
#else
	m_quitGameAct->setShortcut(QKeySequence::Quit);
#endif

	m_newTournamentAct = new QAction(tr("&�½�����..."), this);
	m_stopTournamentAct = new QAction(tr("&ֹͣ����"), this);
	m_showTournamentResultsAct = new QAction(tr("&�������..."), this);

	m_showSettingsAct = new QAction(tr("&ͨ������"), this);
	m_showSettingsAct->setMenuRole(QAction::PreferencesRole);

	m_showSettingsActX = new QAction(tr("&��������"), this);
	m_showSettingsActX->setMenuRole(QAction::PreferencesRole);

	m_showGameDatabaseWindowAct = new QAction(tr("&�Ծ����ݿ�"), this);

	m_showGameWallAct = new QAction(tr("&��ǰ�Ծ�"), this);

	m_minimizeAct = new QAction(tr("&��С��"), this);
	m_minimizeAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));

	m_showPreviousTabAct = new QAction(tr("&��ʾǰһ��"), this);
#ifdef Q_OS_MAC
	m_showPreviousTabAct->setShortcut(QKeySequence(Qt::MetaModifier + Qt::ShiftModifier + Qt::Key_Tab));
#else
	m_showPreviousTabAct->setShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Tab));
#endif

	m_showNextTabAct = new QAction(tr("&��ʾ��һ��"), this);
#ifdef Q_OS_MAC
	m_showNextTabAct->setShortcut(QKeySequence(Qt::MetaModifier + Qt::Key_Tab));
#else
	m_showNextTabAct->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Tab));
#endif

	m_aboutAct = new QAction(tr("&���ڼѼѽ���..."), this);
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
	m_gameMenu = menuBar()->addMenu(tr("&��Ϸ"));
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

	m_tournamentMenu = menuBar()->addMenu(tr("&����"));
	m_tournamentMenu->addAction(m_newTournamentAct);
	m_tournamentMenu->addAction(m_stopTournamentAct);
	m_tournamentMenu->addAction(m_showTournamentResultsAct);
	m_tournamentMenu->addSeparator();
	//m_tournamentMenu->addAction(m_showSettingsAct);

	m_stopTournamentAct->setEnabled(false);

	m_toolsMenu = menuBar()->addMenu(tr("&����"));
	m_toolsMenu->addAction(m_showSettingsAct);
	m_toolsMenu->addAction(m_showSettingsActX);

	m_toolsMenu->addAction(m_showGameDatabaseWindowAct);

	m_viewMenu = menuBar()->addMenu(tr("&��ͼ"));
	m_viewMenu->addAction(m_flipBoardAct);
	m_viewMenu->addSeparator();

	m_windowMenu = menuBar()->addMenu(tr("&����"));
	addDefaultWindowMenu();

	connect(m_windowMenu, SIGNAL(aboutToShow()), this,
		SLOT(onWindowMenuAboutToShow()));

	m_helpMenu = menuBar()->addMenu(tr("&����"));
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


	//QAction* actLinkChessBoard;   // ������������
	//QAction* actEngineThink;      // ������˼��
	//QAction* actEngineStop;       // ������ֹͣ
	//QAction* actEngineAnalyze;    // ���������

	// ���˵�������
	this->mainToolbar = new QToolBar(this);
	this->mainToolbar->setObjectName(QStringLiteral("mainToolBar"));
	this->mainToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly); //  ToolButtonTextUnderIcon); ToolButtonIconOnly
	this->mainToolbar->setMovable(true);
	this->mainToolbar->setAllowedAreas(Qt::TopToolBarArea);
	this->mainToolbar->setVisible(true);
	this->addToolBar(Qt::TopToolBarArea, this->mainToolbar);

	this->mainToolbar->setWindowTitle("������");

	// ����ִ��
	this->tbtnEnginePlayRed = new QToolButton(this);
	this->tbtnEnginePlayRed->setCheckable(true);
	this->tbtnEnginePlayRed->setObjectName(QStringLiteral("EnginePlayRed"));
	QIcon iconEnginePlayRed;
	iconEnginePlayRed.addFile(QStringLiteral(":/icon/com_red.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnEnginePlayRed->setIcon(iconEnginePlayRed);
	this->tbtnEnginePlayRed->setToolTip("����ִ������");
	this->mainToolbar->addWidget(this->tbtnEnginePlayRed);
	connect(this->tbtnEnginePlayRed, SIGNAL(toggled(bool)), this, SLOT(onPlayRedToggled(bool)));

	// ����ִ��
	this->tbtnEnginePlayBlack = new QToolButton(this);
	this->tbtnEnginePlayBlack->setCheckable(true);
	this->tbtnEnginePlayBlack->setObjectName(QStringLiteral("EnginePlayBlack"));
	QIcon iconEnginePlayBlack;
	iconEnginePlayBlack.addFile(QStringLiteral(":/icon/com_blue.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnEnginePlayBlack->setIcon(iconEnginePlayBlack);
	this->tbtnEnginePlayBlack->setToolTip("����ִ������");
	this->mainToolbar->addWidget(this->tbtnEnginePlayBlack);
	connect(this->tbtnEnginePlayBlack, SIGNAL(toggled(bool)), this, SLOT(onPlayBlackToggled(bool)));
	//tbtnTrainFenTimerDelOne

	QWidget* empty2 = new QWidget();
	empty2->setFixedSize(2, 20);
	empty2->setStyleSheet(QString::fromUtf8("border:1px solid gray"));
	this->mainToolbar->addWidget(empty2);

	// ��������������
	this->actEngineStop = new QAction(this);
	this->actEngineStop->setObjectName(QStringLiteral("EngineStop"));
	QIcon iconEngineStop;
	iconEngineStop.addFile(QStringLiteral(":/icon/stop.ico"), QSize(), QIcon::Normal, QIcon::Off);
	this->actEngineStop->setIcon(iconEngineStop);
	this->actEngineStop->setText("��������");
	this->actEngineStop->setToolTip("��������������");
	//this->mainToolbar->addAction(this->actEngineStop);

	// �����������̣��췽����
	this->tbtnLinkChessBoardRed = new QToolButton(this);
	this->tbtnLinkChessBoardRed->setCheckable(true);
	this->tbtnLinkChessBoardRed->setObjectName(QStringLiteral("LinkChessBoardRed"));
	QIcon iconLinkChessBoardRed;
	iconLinkChessBoardRed.addFile(QStringLiteral(":/icon/RedLink.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnLinkChessBoardRed->setIcon(iconLinkChessBoardRed);
	this->tbtnLinkChessBoardRed->setText("�췽����");
	this->tbtnLinkChessBoardRed->setToolTip("������������, �ҷ�ִ��");
	this->mainToolbar->addWidget(this->tbtnLinkChessBoardRed);
	connect(this->tbtnLinkChessBoardRed, SIGNAL(toggled(bool)), this, SLOT(onLinkRedToggled(bool)));


	//onLinkRedToggled

	// �����������̣��ڷ�����
	this->tbtnLinkChessBoardBlack = new QToolButton(this);
	this->tbtnLinkChessBoardBlack->setCheckable(true);
	this->tbtnLinkChessBoardBlack->setObjectName(QStringLiteral("LinkChessBoardBlack"));
	QIcon iconLinkChessBoardBlack;
	iconLinkChessBoardBlack.addFile(QStringLiteral(":/icon/BlackLink.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnLinkChessBoardBlack->setIcon(iconLinkChessBoardBlack);
	this->tbtnLinkChessBoardBlack->setText("�췽����");
	this->tbtnLinkChessBoardBlack->setToolTip("������������, �ҷ�ִ��");
	this->mainToolbar->addWidget(this->tbtnLinkChessBoardBlack);
	connect(this->tbtnLinkChessBoardBlack, SIGNAL(toggled(bool)), this, SLOT(onLinkBlackToggled(bool)));

	// ȫ�Զ�������������
	this->tbtnLinkAuto = new QToolButton(this);
	this->tbtnLinkAuto->setCheckable(true);
	this->tbtnLinkAuto->setObjectName(QStringLiteral("LinkChessBoardAuto"));
	QIcon iconLinkChessBoardAuto;
	iconLinkChessBoardAuto.addFile(QStringLiteral(":/icon/autolink.png"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->tbtnLinkAuto->setIcon(iconLinkChessBoardAuto);
	this->tbtnLinkAuto->setText("ȫ�Զ�����");
	this->tbtnLinkAuto->setToolTip("ȫ�Զ�������������");
	this->mainToolbar->addWidget(this->tbtnLinkAuto);
	connect(this->tbtnLinkAuto, SIGNAL(toggled(bool)), this, SLOT(onLinkAutomaticToggled(bool)));


	// ä��ר��

	/*

	QWidget* empty = new QWidget();
	empty->setFixedSize(10, 20);
	this->mainToolbar->addWidget(empty);

	//QComboBox* cbtnLinkBoard;            // ���ߵ�����
	this->cbtnLinkBoard = new QComboBox(this);
	this->cbtnLinkBoard->setObjectName(QStringLiteral("cbtnLinkBoard"));
	this->cbtnLinkBoard->setToolTip("�ı����߷���");
	QStringList strList;
	//strList << "��������" << "��������" ;
	strList << "��������" << "��������";
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
	this->cbtnLinkEngine->setToolTip("ѡ�����ߵ�����");

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

	//QAction* actLinkChessBoardRed;      // ������������
	//QAction* actLinkChessBoardBlack;    // ������������

	//// ������������
	//this->actLinkChessBoard = new QAction(this);
	//this->actLinkChessBoard->setObjectName(QStringLiteral("LinkChessBoard"));
	//QIcon iconLinkChessBoard;
	//iconLinkChessBoard.addFile(QStringLiteral(":/icon/Links.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actLinkChessBoard->setIcon(iconLinkChessBoard);
	//this->actLinkChessBoard->setText("����");
	//this->actLinkChessBoard->setToolTip("������������");

	//// ������˼��
	//this->actEngineThink = new QAction(this);
	//this->actEngineThink->setObjectName(QStringLiteral("EngineThink"));
	//QIcon iconEngineThink;
	//iconEngineThink.addFile(QStringLiteral(":/icon/thought-balloon.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actEngineThink->setIcon(iconEngineThink);
	//this->actEngineThink->setText("˼��");
	//this->actEngineThink->setToolTip("������˼����ǰ��֣����Զ�����");

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

		// ��ʱɾ��һ��FEN����ֹ����ѵ��
		this->tbtnTrainFenTimerDelOne = new QToolButton(this);
		this->tbtnTrainFenTimerDelOne->setCheckable(true);
		this->tbtnTrainFenTimerDelOne->setObjectName(QStringLiteral("tbtnTrainFenTimerDelOne"));
		QIcon iconTrainFenTimerDelOne;
		iconTrainFenTimerDelOne.addFile(QStringLiteral(":/icon/update.ico"), QSize(), QIcon::Normal, QIcon::Off);
		this->tbtnTrainFenTimerDelOne->setIcon(iconTrainFenTimerDelOne);
		this->tbtnTrainFenTimerDelOne->setToolTip("��ʱɾ��һ��FEN����ֹ����ѵ��");
		this->mainToolbar->addWidget(this->tbtnTrainFenTimerDelOne);
		connect(this->tbtnTrainFenTimerDelOne, SIGNAL(toggled(bool)), this, SLOT(onTrainFenTimerDelOne(bool)));
		//tbtnTrainFenTimerDelOne

		// �ϴ���ǰ��ֵ�Fen Add
		this->actTrainFenAdd = new QAction(this);
		this->actTrainFenAdd->setObjectName(QStringLiteral("actTrainFenAdd"));
		QIcon iconactTrainFenAdd;
		iconactTrainFenAdd.addFile(QStringLiteral(":/icon/add.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenAdd->setIcon(iconactTrainFenAdd);
		this->actTrainFenAdd->setText("�ϴ�Fen");
		this->actTrainFenAdd->setToolTip("����ǰ��ֵ�Fen�ϴ���������ѵ��");

		this->mainToolbar->addAction(this->actTrainFenAdd);
		connect(this->actTrainFenAdd, &QAction::triggered, this, &MainWindow::onTrainFenAdd);


		// �ϴ��߷ֺ���ľ���
		this->actTrainFenDrawTooHigh = new QAction(this);
		this->actTrainFenDrawTooHigh->setObjectName(QStringLiteral("actTrainFenDrawTooHigh"));
		QIcon iconactTrainFenDrawTooHigh;
		iconactTrainFenDrawTooHigh.addFile(QStringLiteral(":/icon/dove_green.png"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenDrawTooHigh->setIcon(iconactTrainFenDrawTooHigh);
		this->actTrainFenDrawTooHigh->setText("�ϴ��߷ֺ���Fen");
		this->actTrainFenDrawTooHigh->setToolTip("�ϴ�������ʾ�߷֣�����ʵ����ľ���");

		this->mainToolbar->addAction(this->actTrainFenDrawTooHigh);
		connect(this->actTrainFenDrawTooHigh, &QAction::triggered, this, &MainWindow::onTrainDrawToHigh);

		// ɾ����ǰ��ֵ�Fen Delete
		this->actTrainFenDelete = new QAction(this);
		this->actTrainFenDelete->setObjectName(QStringLiteral("actTrainFenDelete"));
		QIcon iconactTrainFenDelete;
		iconactTrainFenDelete.addFile(QStringLiteral(":/icon/del.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenDelete->setIcon(iconactTrainFenDelete);
		this->actTrainFenDelete->setText("ɾ���ϴ���Fen");
		this->actTrainFenDelete->setToolTip("����ǰ�ϴ�����������Fenɾ��");

		this->mainToolbar->addAction(this->actTrainFenDelete);
		connect(this->actTrainFenDelete, &QAction::triggered, this, &MainWindow::onTrainFenDelete);

		// �ϴ����õĵ�Fen CommonFen
		this->actTrainFenAddCommonFen = new QAction(this);
		this->actTrainFenAddCommonFen->setObjectName(QStringLiteral("actTrainFenAddCommonFen"));
		QIcon iconactTrainFenAddCommonFen;
		iconactTrainFenAddCommonFen.addFile(QStringLiteral(":/icon/commmon.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenAddCommonFen->setIcon(iconactTrainFenAddCommonFen);
		this->actTrainFenAddCommonFen->setText("�ϴ�����Fen");
		this->actTrainFenAddCommonFen->setToolTip("�ϴ�����Fen�������������ӿ��ֶ�����");

		this->mainToolbar->addAction(this->actTrainFenAddCommonFen);
		connect(this->actTrainFenAddCommonFen, &QAction::triggered, this, &MainWindow::onTrainFenAddCommon);

		// ����ϴ����е�Fen ClearAll
		this->actTrainFenClearAll = new QAction(this);
		this->actTrainFenClearAll->setObjectName(QStringLiteral("actTrainFenClearAll"));
		QIcon iconactTrainFenClearAll;
		iconactTrainFenClearAll.addFile(QStringLiteral(":/icon/remove_all.png"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenClearAll->setIcon(iconactTrainFenClearAll);
		this->actTrainFenClearAll->setText("������е�Fen");
		this->actTrainFenClearAll->setToolTip("���������ϵ�Fenȫ�����");

		this->mainToolbar->addAction(this->actTrainFenClearAll);
		connect(this->actTrainFenClearAll, &QAction::triggered, this, &MainWindow::onTrainFenClearAll);


		// actTrainFenUpdateLast30
		this->actTrainFenUpdateLast50 = new QAction(this);
		this->actTrainFenUpdateLast50->setObjectName(QStringLiteral("actTrainFenUpdateLast30"));
		QIcon iconUpdateLast30;
		iconUpdateLast30.addFile(QStringLiteral(":/icon/up50.ico"),
			QSize(), QIcon::Normal, QIcon::Off);
		this->actTrainFenUpdateLast50->setIcon(iconUpdateLast30);
		this->actTrainFenUpdateLast50->setText("�ϴ����30 Fen");
		this->actTrainFenUpdateLast50->setToolTip("�������ϴ���ǰ��ֵ����30��Fen");

		this->mainToolbar->addAction(this->actTrainFenUpdateLast50);
		connect(this->actTrainFenUpdateLast50, &QAction::triggered, this, &MainWindow::onTrainFenLast50);
	}

	//*****************************************************************************************************


	QWidget* empty4 = new QWidget();
	empty4->setFixedSize(2, 20);
	empty4->setStyleSheet(QString::fromUtf8("border:1px solid gray"));
	this->mainToolbar->addWidget(empty4);


	//// ������˼��
	this->actEngineThink = new QAction(this);
	this->actEngineThink->setObjectName(QStringLiteral("EngineThink"));
	QIcon iconEngineThink;
	iconEngineThink.addFile(QStringLiteral(":/icon/thought-balloon.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->actEngineThink->setIcon(iconEngineThink);
	this->actEngineThink->setText("˼��");
	this->actEngineThink->setToolTip("������˼����ǰ��֣����Զ�����");

	this->mainToolbar->addAction(this->actEngineThink);
	connect(this->actEngineThink, &QAction::triggered, this, &MainWindow::onActEngineThink);

	//*****************************************************************************************************




	//// ���������
	//this->actEngineAnalyze = new QAction(this);
	//this->actEngineAnalyze->setObjectName(QStringLiteral("EngineAnalyze"));
	//QIcon iconEngineAnalyze;
	//iconEngineAnalyze.addFile(QStringLiteral(":/icon/analyze.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actEngineAnalyze->setIcon(iconEngineAnalyze);
	//this->actEngineAnalyze->setText("����");
	//this->actEngineAnalyze->setToolTip("�����������ǰ���");



	//// ��������
	//this->actEngineSetting = new QAction(this);
	//this->actEngineSetting->setObjectName(QStringLiteral("EngineSetting"));
	//QIcon iconEngineSetting;
	//iconEngineSetting.addFile(QStringLiteral(":/icon/Settings.ico"),
	//	QSize(), QIcon::Normal, QIcon::Off);
	//this->actEngineSetting->setIcon(iconEngineSetting);
	//this->actEngineSetting->setText("����");
	//this->actEngineSetting->setToolTip("�����������");

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
			 "Where there is s will��there is a way.",
			 "No pains��no gains.",
			 "Time and tide wait for no man.",
			 "Strike while the iron is hot.",
			 "It��s never too late to mend.",
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
			 "Beauty lies in the love��s eyes.",
			 "Between friends all is common.",
			 "Cannot see the wood for the trees.",
			 "Care and diligence bring luck."
	};
	constexpr int a = sizeof(list) / sizeof(QString);
	return list[Chess::Random::Get().GetInt(0, a - 1)];
}
