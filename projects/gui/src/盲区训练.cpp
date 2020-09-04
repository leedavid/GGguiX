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

Chess::CTrainFen* MainWindow::getCTtrainFen()
{
	if (this->m_ct == nullptr) {
		this->m_ct = new Chess::CTrainFen(this);

		// 这个得加到类的初始化中，有可能是 slot 是 private
		//connect(m_ct, SIGNAL(Chess::CTrainFen::SendSignal(int, QString)),
		//	this, SLOT(slotDisplayStatus(int, QString)));

		//bool ok = connect(m_ct, SIGNAL(Chess::CTrainFen::SendSignal(QString)),
		//	this, SLOT(slotDisplayStatus(QString)));

		//if (ok == false) {
		//	int a = 0;
		//}
	}
	return this->m_ct;
}

void MainWindow::TrainFenAddLostGame(ChessGame* game)
{
	//Chess::CTrainFen ct(this);
	if (this->getCTtrainFen()->isRunning()) {
		this->slotDisplayStatus(3, "trainFen 正在处理中, 请稍候...");
	}
	else {
		this->getCTtrainFen()->on_start2(Chess::CTrainFenMethod::ADD_FEN, game);
	}
}

void MainWindow::TrainFenAddDrawTooHigh(ChessGame* game)
{
	if (this->getCTtrainFen()->isRunning()) {
		this->slotDisplayStatus(3, "trainFen 正在处理中, 请稍候...");
	}
	else {
		this->getCTtrainFen()->on_start2(Chess::CTrainFenMethod::ADD_Draw_Too_High, game);
	}
}


void MainWindow::onTrainFenAdd()
{
	QMessageBox::StandardButton result;
	result = QMessageBox::warning(this, QApplication::applicationName(),
		tr("本操作将当前棋局上传到服务器上训练.\n你确认吗?"),
		QMessageBox::Ok | QMessageBox::Cancel);

	if (result == QMessageBox::Ok) {
		TrainFenAddLostGame(this->GetCurrentChessGame());
	}
}

void MainWindow::onTrainFenDelete()
{
	
	QMessageBox::StandardButton result;
	result = QMessageBox::warning(this, QApplication::applicationName(),
		tr("本操作将删除您以前上传到服务器的局面.\n你确认吗?"),
		QMessageBox::Ok | QMessageBox::Cancel);

	if (result == QMessageBox::Ok) {

		if (this->getCTtrainFen()->isRunning()) {
			this->slotDisplayStatus(3, "trainFen 正在处理中, 请稍候...");
		}
		else {
			this->getCTtrainFen()->on_start2(Chess::CTrainFenMethod::DEL_FEN);
		}
	}
}

void MainWindow::onTrainFenAddCommon()
{
	QMessageBox::StandardButton result;
	result = QMessageBox::warning(this, QApplication::applicationName(),
		tr("本操作将常用开局上传到服务器上训练.\n你确认吗?"),
		QMessageBox::Ok | QMessageBox::Cancel);

	if (result == QMessageBox::Ok) {


		if (this->getCTtrainFen()->isRunning()) {
			this->slotDisplayStatus(3, "trainFen 正在处理中, 请稍候...");
		}
		else {
			this->getCTtrainFen()->on_start2(Chess::CTrainFenMethod::COMMON_FEN);
		}
	}
}

void MainWindow::onTrainDrawToHigh()
{
	QMessageBox::StandardButton result;
	result = QMessageBox::warning(this, QApplication::applicationName(),
		tr("请确认当前棋局是高分，但应该是和棋.\n你确认吗?"),
		QMessageBox::Ok | QMessageBox::Cancel);

	if (result == QMessageBox::Ok) {  // ADD_Draw_Too_High
		TrainFenAddDrawTooHigh(this->GetCurrentChessGame());
	}
}

void MainWindow::onTrainFenClearAll()
{
	QMessageBox::StandardButton result;
	result = QMessageBox::warning(this, QApplication::applicationName(),
		tr("本操作将清除服务器上所有的局面.\n你确认吗?"),
		QMessageBox::Ok | QMessageBox::Cancel);

	if (result == QMessageBox::Ok) {
		if (this->getCTtrainFen()->isRunning()) {
			this->slotDisplayStatus(3, "trainFen 正在处理中, 请稍候...");
		}
		else {
			this->getCTtrainFen()->on_start2(Chess::CTrainFenMethod::REMOVE_FEN);
		}
	}
}
