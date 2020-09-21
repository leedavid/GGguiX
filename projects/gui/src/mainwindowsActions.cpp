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

#include "BoardEditor.h"
#include "random.h"

#include <pgnstream.h>
#include <pgngameentry.h>

#ifdef USE_SOUND
#include <QSound>
#endif
#ifdef USE_SPEECH
#include <QTextToSpeech>
#endif

#ifdef QT_DEBUG
#include <modeltest.h>
#endif

void mouseInput(int x, int y) {
	
	
	
	
	x = x * 65536 / 2560;
	y = y * 65536 / 1440;
	
	
	
	INPUT* buffer = new INPUT[3];

	
	buffer->type = INPUT_MOUSE;
	buffer->mi.dx = x;
	buffer->mi.dy = y;
	buffer->mi.mouseData = 0;
	buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);
	buffer->mi.time = 0;
	buffer->mi.dwExtraInfo = 0;

	(buffer + 1)->type = INPUT_MOUSE;
	(buffer + 1)->mi.dx = x;
	(buffer + 1)->mi.dy = y;
	(buffer + 1)->mi.mouseData = 0;
	(buffer + 1)->mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	(buffer + 1)->mi.time = 0;
	(buffer + 1)->mi.dwExtraInfo = 0;

	(buffer + 2)->type = INPUT_MOUSE;
	(buffer + 2)->mi.dx = x;
	(buffer + 2)->mi.dy = y;
	(buffer + 2)->mi.mouseData = 0;
	(buffer + 2)->mi.dwFlags = MOUSEEVENTF_LEFTUP;
	(buffer + 2)->mi.time = 0;
	(buffer + 2)->mi.dwExtraInfo = 0;

	SendInput(3, buffer, sizeof(INPUT));


	delete buffer;
}

void post(int x, int y) {

	int off_x = 0;
	int off_y = 0;

	HWND hwnd = (HWND)0x000221374;

	LONG temp = MAKELONG(x + off_x, y + off_y);

	::SendMessage(hwnd, WM_MOUSEMOVE, 0, temp);

	::SendMessage(hwnd, WM_LBUTTONDOWN, 0, temp);
	//wait(waitMS);
	::SendMessage(hwnd, WM_LBUTTONUP, 0, temp);
}

void mouseEvent(int x, int y) {

	int X = x * 65536 / 2560 + 1;
	int Y = y * 65536 / 1440 + 1;


	mouse_event(MOUSEEVENTF_LEFTDOWN + MOUSEEVENTF_LEFTUP, X, Y, 0, 0);

	//mouse_event(MOUSEEVENTF_LEFTDOWN + MOUSEEVENTF_LEFTUP + MOUSEEVENTF_ABSOLUTE, X, Y, 0, 0);
}

void MainWindow::onActEngineThink()
{
	
	//LPPOINT p;
	//GetCursorPos(p);
	HWND m_parentHwnd = (HWND)0x00010209C;

	//m_parentHwnd = (HWND)0x000060212C;

	//PostMessage(m_parentHwnd, WM_LBUTTONDOWN, 0x1, MAKELPARAM(319, 450));
	//PostMessage(m_parentHwnd, WM_LBUTTONUP, 0, MAKELPARAM(319, 450));

	//m_parentHwnd = (HWND)0x00000491E50;

	//PostMessage(m_parentHwnd, WM_LBUTTONDOWN, 0x1, MAKELPARAM(319, 450));
	//PostMessage(m_parentHwnd, WM_LBUTTONUP, 0, MAKELPARAM(319, 450));

	//m_parentHwnd = (HWND)0x000010209C;

	//PostMessage(m_parentHwnd, WM_LBUTTONDOWN, 0x1, MAKELPARAM(319, 450));
	//PostMessage(m_parentHwnd, WM_LBUTTONUP, 0, MAKELPARAM(319, 450));

	//SetForegroundWindow(m_parentHwnd);
	//SetTopWindow(m_parentHwnd);

	/*
	typedef    void    (WINAPI* PROCSWITCHTOTHISWINDOW)    (HWND, BOOL);
	PROCSWITCHTOTHISWINDOW    SwitchToThisWindow;
	HMODULE    hUser32 = GetModuleHandle(L"user32");
	SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)
		GetProcAddress(hUser32, "SwitchToThisWindow");
	SwitchToThisWindow(m_parentHwnd, TRUE);
	*/


	//mouseEvent(319, 450);
	//Sleep(500);
	//mouseEvent(319, 410);
	//Sleep(500);
	//return; =

	int X;
	int Y;
	X = 319;
	Y = 450;
	SetCursorPos(X, Y);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, X * 65536 / 2560+5, Y * 65536 / 1440+5, 0, 0);
	Sleep(50);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, X * 65536 / 2560, Y * 65536 / 1440, 0, 0);
	mouseEvent(X, Y);
	//Sleep(50);
	//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//Sleep(50);
	//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	//keybd_event(VK_LBUTTON, 0, 0, 0);
	//keybd_event(VK_LBUTTON, 0, 2, 0);

	wait(300);

	X = 319;
	Y = 410;
	SetCursorPos(X, Y);
	Sleep(50);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, X * 65536 / 2560, Y * 65536 / 1440, 0, 0);
	mouseEvent(X, Y);
	//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//Sleep(50);
	//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	wait(300);


	//SetCursorPos(319, 450);
	//wait(300);
	//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//wait(30);
	//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	////post(319, 450);
	//wait(300);

	//SetCursorPos(319, 410);
	//wait(300);
	//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//wait(30);
	//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	//
	//post(319, 450);


	//SetCursorPos(p->x, p->y);
	return;



	//SetForegroundWindow(m_parentHwnd);

	SetCursorPos(319, 450);
	mouseInput(319, 450);
	wait(20);
	SetCursorPos(319, 410);
	mouseInput(319, 410);
	wait(20);


	//SetCursorPos(p->x, p->y);
	return;
	
	
	//CuteChessApplication::instance()->gameManager()->MyStartMatch(Chess::Side::White, 0);

	

	SetForegroundWindow(m_parentHwnd);

	wait(10);

	//LPPOINT p;
	//GetCursorPos(p);

	SetCursorPos(319, 450);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//wait(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	//this->simWinLeftClick(ffx, ffy);

	wait(10);
	SetCursorPos(319, 410);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//wait(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	//wait(10);

	//SetCursorPos(p->x,p->y);

	return;
	



	//
	//return;
	
	
	
	Chess::Side s = m_game->board()->sideToMove();

	int engineIndex = 0;

	if (m_my_ChessPlayerHumen == nullptr) {
		QString error;
		PlayerBuilder* b = new HumanBuilder("对方", true);
		m_my_ChessPlayerHumen = b->create(this, SIGNAL(debugMessage(QString)),
			this, &error);
	}

	if (m_my_ChessPlayer[engineIndex] == nullptr) {
		QString error;
		EngineManager* engineManager = CuteChessApplication::instance()->engineManager();
		auto config = engineManager->engineAt(engineIndex);
		config.setPondering(true);
		PlayerBuilder* b = new EngineBuilder(config);

		m_my_ChessPlayer[engineIndex] = b->create(this, SIGNAL(debugMessage(QString)),
			this, &error);
	}

	// 当前的替换一下


	// 
	//int a = 0;

}

void MainWindow::moveChanged()
{
	//auto g = m_game;
	//MoveId m = g->currentMove();

	//m_game
	/*
	const Game& g = game();
	MoveId m = g.currentMove();

	// Set board first
	m_boardView->setBoard(g.board(), m_currentFrom, m_currentTo, game().atLineEnd());

	QString annotation = game().textAnnotation();
	BoardViewEx* frame = BoardViewFrame(m_boardView);
	if (frame)
	{
		frame->setComment(annotation);
	}

	m_currentFrom = InvalidSquare;
	m_currentTo = InvalidSquare;

	emit displayTime(g.timeAnnotation(m, Game::BeforeMove), g.board().toMove(), g.timeAnnotation(m, Game::AfterMove));

	// Highlight current move
	m_gameView->showMove(m);
	if (g.isMainline())
	{
		m_gameView->slotDisplayPly(g.ply());
	}

	displayVariations();

	slotSearchTree();

	QString line = getUCIHistory();
	emit boardChange(g.board(), line);

	// Clear  entries
	m_nagText.clear();

	emit signalMoveHasNextMove(!gameMode() && !game().atLineEnd());
	emit signalMoveHasPreviousMove(!gameMode() && !game().atGameStart());
	emit signalMoveHasVariation(!gameMode() && game().variationCount() > 0);
	emit signalMoveHasParent(!gameMode() && !game().isMainline());
	emit signalVariationHasSibling(!gameMode() && game().variationHasSiblings(m));
	emit signalGameIsEmpty(false);
	emit signalGameAtLineStart(!gameMode() && game().atLineStart());

	*/

	//QString line = getUCIHistory();
	//emit boardChange(*g->board(), line);
}

int MainWindow::getSelEngineIndex(bool isMain) const
{
	if (isMain) {
		return m_AnalysisWidget[0]->getCurEngineIndex();
	}
	else {
		return m_AnalysisWidget[1]->getCurEngineIndex();
	}
}


void MainWindow::slotGameChanged(bool bModified)
{
	//UpdateMaterial();
	//UpdateGameText();
	//UpdateGameTitle();
	(void)bModified;
	moveChanged();
}

void MainWindow::slotMoveChanged()
{
	//if (m_training->isChecked() || m_training2->isChecked())
	//{
	//	UpdateGameText();
	//}
	moveChanged();
}

void MainWindow::slotDatabaseChanged()
{
	/*m_undoGroup.setActiveStack(databaseInfo()->undoStack());
	database()->index()->calculateCache();
	setWindowTitle(tr("%1 - ChessX").arg(databaseName()));
	m_gameList->setFilter(databaseInfo()->filter());
	updateLastGameList();
	slotFilterChanged();*/
	slotGameChanged(true);
	//emit databaseChanged(databaseInfo());
	//emit databaseModified();
}