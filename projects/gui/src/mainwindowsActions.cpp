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

void MainWindow::onActEngineThink()
{
	//CuteChessApplication::instance()->gameManager()->MyStartMatch(Chess::Side::White, 0);

	Chess::Side s = m_game->board()->sideToMove();

	int engineIndex = 0;

	if (m_my_ChessPlayerHumen == nullptr) {
		QString error;
		PlayerBuilder* b = new HumanBuilder("¶Ô·½", true);
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

	// 
	int a = 0;

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