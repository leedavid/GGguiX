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

#include "TrainFenToServer.h"
#include "ChessDB.h"

using namespace Chess;

// ������Ϣ
void MainWindow::slotProcessCapMsg(Chess::stCaptureMsg msg)
{
	// �õ���ǰ����Ϸ�����ǣ�Ӧ�õõ���ǰ��chessgame

	switch (msg.mType) {
	case Chess::eCapMsgType::eText:
		QMessageBox::warning(this, msg.title, msg.text);
		break;
	case Chess::eCapMsgType::eMove:
		// �ж��߲��ǲ��ǺϷ�
		if (isMoveValid(msg.m) == false) {  // ��ֹ�ظ������߲�
			return;
		}
		m_gameViewer->viewLinkMove(msg.m);

		// 


		break;
	case Chess::eCapMsgType::eSetFen:
	{
		QString fen = msg.text;
		if (this->tbtnLinkAuto->isChecked()) {

			//if (m_pcap == nullptr)
			//	m_pcap = new Chess::Capture(this);

			//while (m_pcap->m_isRuning) {
			//	m_pcap->on_stop();
			//	this->wait(1);
			//	m_game->stop();
			//}


			if (fen.contains("w -", Qt::CaseSensitive)) {
				//m_onLinkRedToggled = true;
				this->tbtnLinkChessBoardRed->setChecked(true);
				this->tbtnLinkChessBoardBlack->setChecked(false);
			}
			else {
				this->tbtnLinkChessBoardRed->setChecked(false);
				this->tbtnLinkChessBoardBlack->setChecked(true);
			}

			//m_pcap->m_bMainGetFenAlready = true;
			//m_pcap->on_start();

		}

		if (this->tbtnLinkChessBoardRed->isChecked() || this->tbtnLinkChessBoardBlack->isChecked()) {  // ��������
			bool ok = true;

			// �õ���ǰ������
			QSettings s;

			s.beginGroup("games");
			//const QString variant1 = s.value("variant").toString();
			const QString variant = "standard"; // s.value("variant").toString();    // ��Ϸ����

			//auto board = Chess::BoardFactory::create(variant);
			//board->SetAutoLinkStat(true);

			//QString fen = "2bakab2/9/9/8R/pnpNP4/3r5/c3c4/1C2B2C1/4A4/2BAK4 w - - 0 1";

			//QString fen(m_gameViewer->board()->fenString());
			//if (!fen.isEmpty() && !board->setFenString(fen))
			//{
				//auto palette = ui->m_fenEdit->palette();
				//palette.setColor(QPalette::Text, Qt::red);
				//ui->m_fenEdit->setPalette(palette);
				//m_isValid = false;
				//emit statusChanged(false);
			//	fen = "";
			//}
			//board->setFenString(fen);

			auto pgn = new PgnGame();

			//pgn->m_autoLink = true;           // ������ʱΪ0
			pgn->SetLinkCatName(this->cbtnLinkBoard->currentText());

			pgn->setSite(QSettings().value("pgn/site").toString());

			auto board = pgn->createBoard();

			auto game = new ChessGame(board, pgn);

			//game->setIsLinkBoard(true);	

			game->setStartingFen(fen);
			//this->m_gameViewer->viewPreviousMove2(game->board());  //

			// ʱ�����
			TimeControl m_timeControl;
			m_timeControl.readSettings(&s);
			game->setTimeControl(m_timeControl);

			// �ö�����
			s.beginGroup("draw_adjudication");
			GameAdjudicator m_adjudicator;
			m_adjudicator.setDrawThreshold(s.value("move_number").toInt(),
				s.value("move_count").toInt(),
				s.value("score").toInt());
			s.endGroup();
			game->setAdjudicator(m_adjudicator);

			//auto suite = ui->m_gameSettings->openingSuite();          // ���ֳ�ʼ�����趨
			//if (suite)
			//{
			//	int depth = ui->m_gameSettings->openingSuiteDepth();
			//	ok = game->setMoves(suite->nextGame(depth));
			//	delete suite;
			//}


			//connect(ui->m_polyglotFileEdit, &QLineEdit::textChanged,
			//	[=](const QString& file)
			//	{
			//		QSettings().setValue("games/opening_book/file", file);
			//	});
			//connect(ui->m_polyglotDepthSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			//	[=](int depth)
			//	{
			//		QSettings().setValue("games/opening_book/depth", depth);
			//	});

		  // QString 

			int depth;
			auto book = this->GetOpeningBook(depth);

			//auto book = ui->m_gameSettings->openingBook();           // ���ֿ�
			//if (book)
			//{
			//	int depth = ui->m_gameSettings->bookDepth();
			//	game->setBookOwnership(true);

			//	for (int i = 0; i < 2; i++)
			//	{
			//		auto side = Chess::Side::Type(i);
			//		if (playerType(side) == CPU)
			//			game->setOpeningBook(book, side, depth);
			//	}
			//}

			if (book) {
				if (this->tbtnLinkChessBoardRed->isChecked()) {
					game->setOpeningBook(book, Chess::Side::White, depth);
				}
				if (this->tbtnLinkChessBoardBlack->isChecked()) {
					game->setOpeningBook(book, Chess::Side::Black, depth);
				}
			}

			s.endGroup();   // 

			if (!ok)
			{
				delete game;
				QMessageBox::critical(this, tr("Could not initialize game"),
					tr("The game could not be initialized "
						"due to an invalid opening."));
				return;
			}

			//auto game = this->m_game;

			//game->isGetSetting = true;    // ��������ú���


			//bool isWhiteCPU = (side == Chess::Side::White);
			//bool isBlackCPU = (side == Chess::Side::Black);

			PlayerBuilder* builders[2] = {
				mainCreatePlayerBuilder(Chess::Side::White, this->tbtnLinkChessBoardRed->isChecked()),
				mainCreatePlayerBuilder(Chess::Side::Black, this->tbtnLinkChessBoardBlack->isChecked())
			};

			//EvalWidget* m_evalWidgets[2];
			/*  ������������ˣ��̶������¶���
			if (this->tbtnLinkChessBoardRed->isChecked()) {
				m_whiteEvalDock->show();
				m_blackEvalDock->hide();
			}
			if (this->tbtnLinkChessBoardBlack->isChecked()) {
				m_blackEvalDock->show();
				m_whiteEvalDock->hide();
			}
			*/


			// 
			////this->m_w

			//Chess::Side sss = game->board()->sideToMove();

			//if (builders[game->board()->sideToMove()]->isHuman())
			//	game->pause();


			m_myClosePreTab = true; // ����ǰһ������

			// Start the game in a new tab
			connect(game, SIGNAL(initialized(ChessGame*)),
				this, SLOT(addGame(ChessGame*)));
			connect(game, SIGNAL(startFailed(ChessGame*)),
				this, SLOT(onGameStartFailed(ChessGame*)));
			CuteChessApplication::instance()->gameManager()->newGame(game,             // ������������ӵ�Tab����
				builders[Chess::Side::White], builders[Chess::Side::Black]);

			// ���������߲���capture
			// void moveMade(const Chess::GenericMove& move,
			//nst QString& sanString,
			//	const QString& comment);

			//connect(game, SIGNAL(moveMade(Chess::GenericMove, QString, QString)),
			//	m_scene, SLOT(makeMove(Chess::GenericMove)));

			//bool b = 
			connect(game, SIGNAL(moveMade(Chess::GenericMove, QString, QString)),
				m_pcap, SLOT(ProcessBoardMove(const Chess::GenericMove&)));

			// ���ʹ���ƿ⣬��ҲҪ����

			// ����Ǻ췽�ߣ��Ͳ���Ҫ��ת����
			//connect(m_flipBoardAct, SIGNAL(triggered()),
			//	m_gameViewer->boardScene(), SLOT(flip()));

			//m_gameViewer->boardScene()->flip();
			//void moveMadeFen(const QString fen);  // ���Ӻ���һ��FEN
			//Chess::ChessDB* pdb = this->GetChessDB();
			//if (pdb != nullptr) {
			//	//bool a  = 
			//	connect(game, SIGNAL(moveMadeFen(const QString&)),
			//		pdb, SLOT(FindChessDBmoveByFEN(const QString&)));
			//	//int c = 0;
			//}


		}
	}
	break;
	case Chess::eCapMsgType::eDraw:   // ��վ�ж���Ϣ

		break;
	default:
		break;
	}
}



void MainWindow::onLXchessboardStart() {

	
	QString catName = this->cbtnLinkBoard->currentText();

	if (m_pcap == nullptr)
		m_pcap = new Chess::Capture(this, catName);

	m_pcap->on_start();
}

void MainWindow::onLXchessboardStop() {

	m_pcap->on_stop();
	//delete pcap;

	m_game->stop();
}

PlayerBuilder* MainWindow::mainCreatePlayerBuilder(Chess::Side side, bool isCPU) const
{
	(void)side;

	if (isCPU) //(playerType(side) == CPU)
	{

		//auto config =  m_engineConfig[side];

		EngineManager* engineManager = CuteChessApplication::instance()->engineManager();

		//int sel = this->cbtnLinkEngine->currentIndex();   // ����ѡ��'
		int sel = this->getSelEngineIndex(true);
		auto config = engineManager->engineAt(sel);

		QSettings s;
		s.beginGroup("games");

		bool ponder = s.value("pondering").toBool();
		config.setPondering(ponder);
		// �Ƿ�Ҫ��̨˼��
		// ui->m_gameSettings->applyEngineConfiguration(&config);

		s.endGroup();

		return new EngineBuilder(config);

	}
	bool ignoreFlag = QSettings().value("games/human_can_play_after_timeout",
		true).toBool();
	return new HumanBuilder(CuteChessApplication::userName(), ignoreFlag);
}

// �췽����˼����ť, ��ֹ����ģʽ
void MainWindow::onPlayRedToggled(bool checked) {

	//this->mainToolbar->setVisible(true);
	tbtnEnginePlayBlack->setDisabled(checked);

	// ��ֹ������ť
	tbtnLinkChessBoardRed->setDisabled(checked);
	tbtnLinkChessBoardBlack->setDisabled(checked);
	tbtnLinkAuto->setDisabled(checked);

	this->onPlayWhich();  //  , Chess::Side::White);	

}

// �ڷ�����˼����ť
void MainWindow::onPlayBlackToggled(bool checked) {




	// ��ֹ������ť
	tbtnEnginePlayRed->setDisabled(checked);


	tbtnLinkChessBoardRed->setDisabled(checked);
	tbtnLinkChessBoardBlack->setDisabled(checked);
	tbtnLinkAuto->setDisabled(checked);

	this->onPlayWhich(); // , Chess::Side::Black);
}

void MainWindow::onPlayWhich() //, Chess::Side side)
{

	//CuteChessApplication::instance()->gameManager()->MyStartMatch(Chess::Side::White, 0);
	//return;

	//(void)checked;
	if (this->tbtnEnginePlayRed->isChecked() || this->tbtnEnginePlayBlack->isChecked()) {


		//EngineConfiguration config = CuteChessApplication::instance()->engineManager()->engineAt(0);
		//m_tabs.at(m_tabBar->currentIndex()).m_game->setPlayer(Chess::Side::White, (new EngineBuilder(config))->create(nullptr, nullptr, this, nullptr));

		//auto cur_game = m_tabs.at(m_tabBar->currentIndex()).m_game;
		//this->m_game

		//QString stFen = m_game->startingFen();
		//QVector<Chess::Move> moves = m_game->moves();

		//auto game = this->m_game;

		//if (this->m_game->isGetSetting == false) {
		//if(1){

			//int preIndex = m_tabBar->currentIndex();

			//EngineManager* engineManager = CuteChessApplication::instance()->engineManager();			
		bool ok = true;

		// �õ���ǰ������
		QSettings s;

		s.beginGroup("games");
		//const QString variant1 = s.value("variant").toString();
		const QString variant = "standard"; // s.value("variant").toString();    // ��Ϸ����

		auto board = Chess::BoardFactory::create(variant);

		//QString fen = "2bakab2/9/9/8R/pnpNP4/3r5/c3c4/1C2B2C1/4A4/2BAK4 w - - 0 1";

		QString fen(m_gameViewer->board()->fenString());
		//if (!fen.isEmpty() && !board->setFenString(fen))
		//{
			//auto palette = ui->m_fenEdit->palette();
			//palette.setColor(QPalette::Text, Qt::red);
			//ui->m_fenEdit->setPalette(palette);
			//m_isValid = false;
			//emit statusChanged(false);
		//	fen = "";
		//}

		auto pgn = new PgnGame();
		pgn->setSite(QSettings().value("pgn/site").toString());
		auto game = new ChessGame(board, pgn);

		game->setStartingFen(fen);

		// ʱ�����
		TimeControl m_timeControl;
		m_timeControl.readSettings(&s);
		game->setTimeControl(m_timeControl);

		// �ö�����
		s.beginGroup("draw_adjudication");
		GameAdjudicator m_adjudicator;
		m_adjudicator.setDrawThreshold(s.value("move_number").toInt(),
			s.value("move_count").toInt(),
			s.value("score").toInt());
		s.endGroup();
		game->setAdjudicator(m_adjudicator);

		//auto suite = ui->m_gameSettings->openingSuite();          // ���ֳ�ʼ�����趨
		//if (suite)
		//{
		//	int depth = ui->m_gameSettings->openingSuiteDepth();
		//	ok = game->setMoves(suite->nextGame(depth));
		//	delete suite;
		//}



		//auto book = ui->m_gameSettings->openingBook();           // ���ֿ�
		//if (book)
		//{
		//	int depth = ui->m_gameSettings->bookDepth();
		//	game->setBookOwnership(true);

		//	for (int i = 0; i < 2; i++)
		//	{
		//		auto side = Chess::Side::Type(i);
		//		if (playerType(side) == CPU)
		//			game->setOpeningBook(book, side, depth);
		//	}
		//}

		s.endGroup();   // 

		if (!ok)
		{
			delete game;
			QMessageBox::critical(this, tr("Could not initialize game"),
				tr("The game could not be initialized "
					"due to an invalid opening."));
			return;
		}

		//auto game = this->m_game;

		//game->isGetSetting = true;    // ��������ú���


		//bool isWhiteCPU = (side == Chess::Side::White);
		//bool isBlackCPU = (side == Chess::Side::Black);

		int depth;
		auto book = this->GetOpeningBook(depth);

		//auto book = ui->m_gameSettings->openingBook();           // ���ֿ�
		//if (book)
		//{
		//	int depth = ui->m_gameSettings->bookDepth();
		//	game->setBookOwnership(true);

		//	for (int i = 0; i < 2; i++)
		//	{
		//		auto side = Chess::Side::Type(i);
		//		if (playerType(side) == CPU)
		//			game->setOpeningBook(book, side, depth);
		//	}
		//}

		if (book) {
			if (this->tbtnEnginePlayRed->isChecked()) {
				game->setOpeningBook(book, Chess::Side::White, depth);
			}
			if (this->tbtnEnginePlayBlack->isChecked()) {
				game->setOpeningBook(book, Chess::Side::Black, depth);
			}
		}


		//EvalWidget* m_evalWidgets[2];
		/*
		if (this->tbtnEnginePlayRed->isChecked()) {
			m_whiteEvalDock->show();
			m_blackEvalDock->hide();
		}
		if (this->tbtnEnginePlayBlack->isChecked()) {
			m_blackEvalDock->show();
			m_whiteEvalDock->hide();
		}
		*/


		PlayerBuilder* builders[2] = {
			mainCreatePlayerBuilder(Chess::Side::White, this->tbtnEnginePlayRed->isChecked()),
			mainCreatePlayerBuilder(Chess::Side::Black, this->tbtnEnginePlayBlack->isChecked())
		};


		if (builders[game->board()->sideToMove()]->isHuman())
			game->pause();


		m_myClosePreTab = true; // ����ǰһ������

		// Start the game in a new tab
		connect(game, SIGNAL(initialized(ChessGame*)),
			this, SLOT(addGame(ChessGame*)));
		connect(game, SIGNAL(startFailed(ChessGame*)),
			this, SLOT(onGameStartFailed(ChessGame*)));
		CuteChessApplication::instance()->gameManager()->newGame(game,             // ������������ӵ�Tab���У�������������ʵ��
			builders[Chess::Side::White], builders[Chess::Side::Black]);

		//if (m_tabs.size() >= 2) {
		//	closeTab(preIndex);
		//}

	//}
	}
	else { // ֹͣ����
		// destroyGame(m_game);
		// this->m_game->isGetSetting = false;
		//resignGame();
		//closeTab(m_tabBar->currentIndex());
		//int ci = m_tabBar->currentIndex();
		//int count = m_tabBar->count();

		//if (m_tabs.size() >= 2){
		//	closeTab(m_tabBar->currentIndex());
		//}

		//int currentIndex() const;
		//int count() const;
		m_game->stop();
	}
}

void MainWindow::onLinkRedToggled(bool checked)
{
	if (tbtnLinkAuto->isChecked()) return;

	tbtnEnginePlayRed->setDisabled(checked);
	tbtnEnginePlayBlack->setDisabled(checked);
	tbtnLinkChessBoardBlack->setDisabled(checked);
	tbtnLinkAuto->setDisabled(checked);

	this->onLinkWhich(checked);  //  , Chess::Side::White);
}
void MainWindow::onLinkBlackToggled(bool checked)
{
	if (tbtnLinkAuto->isChecked()) return;

	tbtnEnginePlayRed->setDisabled(checked);
	tbtnEnginePlayBlack->setDisabled(checked);
	tbtnLinkChessBoardRed->setDisabled(checked);
	tbtnLinkAuto->setDisabled(checked);

	this->onLinkWhich(checked);  //  , Chess::Side::White);
}

void MainWindow::onLinkAutomaticToggled(bool checked)
{

	m_now_is_match = false;

	tbtnEnginePlayRed->setDisabled(checked);
	tbtnEnginePlayBlack->setDisabled(checked);
	tbtnLinkChessBoardRed->setDisabled(checked);
	tbtnLinkChessBoardBlack->setDisabled(checked);

	//
	QString catName = this->cbtnLinkBoard->currentText();

	if (checked) {
		if (m_autoClickCap == nullptr)
			m_autoClickCap = new Chess::Capture(this, catName, true);
		m_autoClickCap->on_start();

		if (m_pcap == nullptr)
			m_pcap = new Chess::Capture(this, catName);

		while (m_pcap->isRunning()) {
			m_pcap->on_stop();
			this->wait(1);
			m_game->stop();
		}
		m_pcap->on_start();
	}
	else {

		while (m_pcap->isRunning()) {
			m_autoClickCap->on_stop();
			m_pcap->on_stop();
			m_game->stop();
		}

		this->tbtnLinkChessBoardRed->setChecked(false);
		this->tbtnLinkChessBoardBlack->setChecked(false);

		//m_bAutomaticLinking = false;
	}
}

void MainWindow::onLinkWhich(bool checked)
{
	if (checked) {
		onLXchessboardStart();  // ��ʼ����
	}
	else {
		onLXchessboardStop();
	}
}




void MainWindow::onLinkBoardCombox(const QString& txt)
{
	if (this->m_pcap != nullptr) {
		this->m_pcap->SetCatlogName(txt);
	}
	if (this->m_autoClickCap != nullptr) {
		this->m_autoClickCap->SetCatlogName(txt);
	}
}