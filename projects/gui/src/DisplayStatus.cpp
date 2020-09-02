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


//void MainWindow::slotDisplayStatus(int which, QString msg)
void MainWindow::slotDisplayStatus(int which, QString msg)
{
	//int which = 1;
	switch (which) {
	case 1:
		m_status1->setText(msg);
		break;
	case 2:
		m_status2->setText(msg);
		break;
	case 3:
		m_status3->setText(msg);
		break;
	default:
		m_status1->setText(msg);
		break;
	}
}



void MainWindow::createStatus()
{
	// 状态栏--------------------------------------------------------------------------------------------	
	m_status1 = new QLabel();
	statusBar()->addPermanentWidget(m_status1, 1);
	m_status1->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	m_status2 = new QLabel();
	statusBar()->addPermanentWidget(m_status2, 1);
	m_status2->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	m_status3 = new QLabel();
	statusBar()->addPermanentWidget(m_status3, 1);
	m_status3->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	m_status1->setText(preverb());
	m_status2->setText(preverb());

	//m_sliderText->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_status3->setText(QString("www.ggzero.cn ver: ") + CUTECHESS_VERSION);

	//statusBar()->addPermanentWidget(cbtnLinkBoard);
	//statusBar()->addPermanentWidget(cbtnLinkEngine);

	//QWidget* empty = new QWidget();
	//empty->setFixedSize(10, 20);
	//this->mainToolbar->addWidget(empty);

	//QComboBox* cbtnLinkBoard;            // 连线的棋盘
	this->cbtnLinkBoard = new QComboBox(this);
	this->cbtnLinkBoard->setObjectName(QStringLiteral("cbtnLinkBoard"));
	this->cbtnLinkBoard->setToolTip("改变连线方案");
	QStringList strList;
	//strList << "天天象棋" << "王者象棋" ;
	// 得到所有的连线目录
	{
		QString filePath = QCoreApplication::applicationDirPath() + "/image/linkboard/";
		QDir dir(filePath);
		if (dir.exists()) {

			dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot); // | QDir::Files | QDir::NoDotAndDotDot);
			//文件夹优先
			dir.setSorting(QDir::DirsFirst);
			//QFileInfoList list = dir.entryInfoList();
			QStringList files = dir.entryList();
			for (QString file : files) {
				strList << file;
			}
		}
	}
	//strList << "王者象棋" << "天天象棋";
	this->cbtnLinkBoard->addItems(strList);
	connect(this->cbtnLinkBoard, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onLinkBoardCombox(const QString&)));
	//this->mainToolbar->addWidget(this->cbtnLinkBoard);

	statusBar()->addPermanentWidget(this->cbtnLinkBoard);

	int sel = QSettings().value("ui/linkboard_curSel").toInt();
	this->cbtnLinkBoard->setCurrentIndex(sel);


	//QWidget* empty2 = new QWidget();
	//empty2->setFixedSize(10, 20);
	//this->mainToolbar->addWidget(empty2);

	/*
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
	statusBar()->addPermanentWidget(this->cbtnLinkEngine);

	sel = QSettings().value("ui/linkboard_curEngine").toInt();
	this->cbtnLinkEngine->setCurrentIndex(sel);
	*/

	m_sliderSpeed = new Chess::TranslatingSlider(this);
	m_sliderSpeed->setToolTip("即时调整引擎运算时间");
	m_sliderSpeed->setMultiplier(1000);
	m_sliderSpeed->setMultiplier2(10000);
	m_sliderSpeed->setMultiplier3(60000);
	m_sliderSpeed->setOrientation(Qt::Horizontal);
	m_sliderSpeed->setMinimum(0);  // O = Infinite
	m_sliderSpeed->setStart2(30);  // Step 10s after 30s
	m_sliderSpeed->setStart3(57);  // Step 60s after 5min
	m_sliderSpeed->setMaximum(97); // 45 Minutes
	//m_sliderSpeed->setTranslatedValue(QSettings().value("/Board/AutoPlayerInterval").toInt());
	m_sliderSpeed->setTranslatedValue(0);
	m_sliderSpeed->setTickInterval(1);
	m_sliderSpeed->setTickPosition(QSlider::NoTicks);
	m_sliderSpeed->setSingleStep(1);
	m_sliderSpeed->setPageStep(1);
	m_sliderSpeed->setMinimumWidth(120); // 87 + some pixel for overlapping slider
	m_sliderSpeed->setMaximumWidth(400); // Arbitrary limit - not really needed

	//connect(m_sliderSpeed, SIGNAL(translatedValueChanged(int)), SLOT(slotMoveIntervalChanged(int)));
	statusBar()->addPermanentWidget(m_sliderSpeed);
	m_sliderText = new QLabel(this);
	m_sliderText->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	slotSetSliderText(0);
	m_sliderText->setFixedWidth(m_sliderText->sizeHint().width());
	statusBar()->addPermanentWidget(m_sliderText);
	connect(m_sliderSpeed, SIGNAL(translatedValueChanged(int)), this, SLOT(slotSetSliderText()));

	slotSetSliderText();

	statusBar()->setFixedHeight(statusBar()->height());
	statusBar()->setSizeGripEnabled(true);

	//connect(m_sliderSpeed, SIGNAL(translatedValueChanged(int)), m_game, SLOT(onAdjustTimePerMove(int)));
}