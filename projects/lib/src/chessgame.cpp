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

#include "chessgame.h"
#include <QThread>
#include <QTimer>
#include "board/board.h"
#include "chessplayer.h"
#include "openingbook.h"
#include "timecontrol.h"

#include "../../gui/src/annotation.h"

//const QStringList ChessGame::s_specList = QStringList() << s_emt << s_clk << s_egt << s_csl << s_cal;

namespace {

QString evalString(const MoveEvaluation& eval)
{
	if (eval.isBookEval())
		return "book";
	if (eval.isEmpty())
		return QString();

	QString str = eval.scoreText();
	if (eval.depth() > 0)
		str += "/" + QString::number(eval.depth()) + " ";

	int t = eval.time();
	if (t == 0)
		return str + "0s";

	int precision = 0;
	if (t < 100)
		precision = 3;
	else if (t < 1000)
		precision = 2;
	else if (t < 10000)
		precision = 1;
	str += QString::number(double(t / 1000.0), 'f', precision) + 's';

	return str;
}

} // anonymous namespace

ChessGame::ChessGame(const ChessGame& game) : QObject()
{
	mountRefCount = 0;
	//m_currentNode = 0;
	//m_startPly = 0;
	m_currentBoard = nullptr;
	mountBoard();
	*this = game;
}

ChessGame& ChessGame::operator=(const ChessGame& game)
{
	if (this != &game)
	{
		//clearTags();
		clear();
		//assign non pointer variables
		m_startingBoard = game.m_startingBoard;
		//m_tags = game.m_tags;
		//m_tags.detach();
		//m_variationStartAnnotations = game.m_variationStartAnnotations;
		//m_variationStartAnnotations.detach();
		//m_annotations = game.m_annotations;
		//m_annotations.detach();
		//m_startPly = game.m_startPly;
		//m_moveNodes = game.m_moveNodes;
		//m_moveNodes.detach();
		//m_currentNode = game.m_currentNode;
		if (m_currentBoard && game.m_currentBoard)
		{
			*m_currentBoard = *game.m_currentBoard;
		}
		else if (m_currentBoard)
		{
			//moveToStart();
		}

		this->m_startingFen = game.m_startingFen;
		this->m_fens = game.m_fens;
		this->m_scores = game.m_scores;		
	}
	return *this;
}

ChessGame::ChessGame(Chess::Board* board, PgnGame* pgn, QObject* parent)
	: QObject(parent),
	m_currentBoard(board),
	m_startDelay(0),
	m_finished(false),
	m_gameInProgress(false),
	m_paused(false),
	m_pgnInitialized(false),
	m_bookOwnership(false),
	m_boardShouldBeFlipped(false),
	m_pgn(pgn),
	//isGetSetting(false),
	m_isLinkBoard(false)
	//m_isEngingMatch(false)   // 不是引擎比赛
{
	Q_ASSERT(pgn != nullptr);

	this->m_startingFen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";

	for (int i = 0; i < 2; i++)
	{
		m_player[i] = nullptr;
		m_book[i] = nullptr;
		m_bookDepth[i] = 0;
	}
}

ChessGame::~ChessGame()
{
	if (mountRefCount)
	{
		delete m_currentBoard;
		m_currentBoard = nullptr;
	}
	if (m_bookOwnership)
	{
		if (m_book[0]) {
			bool same = (m_book[0] == m_book[1]);
			delete m_book[0];
			if (!same)
				delete m_book[1];
		}
	}
}

void ChessGame::mountBoard()
{
	++mountRefCount;
	if (mountRefCount == 1)
	{
		m_currentBoard = Chess::BoardFactory::create("");
	}
}

void ChessGame::unmountBoard()
{
	if (mountRefCount > 0)
	{
		--mountRefCount;
		if (mountRefCount == 0)
		{
			delete m_currentBoard;
			m_currentBoard = nullptr;
		}
	}
}

//void ChessGame::clearTags()
//{
//	m_tags.clear();
//}

void ChessGame::clear()
{
	//m_variationStartAnnotations.clear();
	//m_annotations.clear();

	//m_startPly = 0;

	
	m_startingBoard->setFenString(m_startingBoard->defaultFenString());
	//m_startingBoard.setStandardPosition();
	if (m_currentBoard)
	{
		//m_currentNode = 0;
		delete m_currentBoard;
		m_currentBoard = m_startingBoard->copy();
	}
	else
	{
		//m_currentNode = NO_MOVE;
	}

	//m_moveNodes.append(MoveNode());
}

QString ChessGame::errorString() const
{
	return m_error;
}

ChessPlayer* ChessGame::player(Chess::Side side) const
{
	Q_ASSERT(!side.isNull());
	return m_player[side];
}

bool ChessGame::isFinished() const
{
	return m_finished;
}

bool ChessGame::boardShouldBeFlipped() const
{
	return m_boardShouldBeFlipped;
}

void ChessGame::setBoardShouldBeFlipped(bool flip)
{
	m_boardShouldBeFlipped = flip;
}



PgnGame* ChessGame::pgn() const
{
	return m_pgn;
}

Chess::Board* ChessGame::board() const
{
	return m_currentBoard; // m_board;
}

QString ChessGame::startingFen() const
{
	//return m_currentBoard->startingFenString();
	return m_startingFen;
}

const QVector<Chess::Move>& ChessGame::moves() const
{
	return m_moves;
}

const QMap<int,int>& ChessGame::scores() const
{
	return m_scores;
}

Chess::Result ChessGame::result() const
{
	return m_result;
}

ChessPlayer* ChessGame::playerToMove() const
{
	if (m_currentBoard->sideToMove().isNull())
		return nullptr;
	return m_player[m_currentBoard->sideToMove()];
}

ChessPlayer* ChessGame::playerToWait() const
{
	if (m_currentBoard->sideToMove().isNull())
		return nullptr;
	return m_player[m_currentBoard->sideToMove().opposite()];
}

void ChessGame::stop(bool emitMoveChanged)
{
	if (m_finished)
		return;

	m_finished = true;
	emit humanEnabled(false);
	if (!m_gameInProgress)
	{
		m_result = Chess::Result();
		finish();
		return;
	}
	
	QDateTime gameEndTime = QDateTime::currentDateTime();

	initializePgn();
	m_gameInProgress = false;
	const QVector<PgnGame::MoveData>& moves(m_pgn->moves());
	int plies = moves.size();

	m_pgn->setTag("PlyCount", QString::number(plies));

	m_pgn->setGameEndTime(gameEndTime);

	m_pgn->setResult(m_result);
	m_pgn->setResultDescription(m_result.description());

	if (emitMoveChanged && plies > 1)
	{
		const PgnGame::MoveData& md(moves.at(plies - 1));
		emit moveChanged(plies - 1, md.move, md.moveString, md.comment);
	}

	m_player[Chess::Side::White]->endGame(m_result);
	m_player[Chess::Side::Black]->endGame(m_result);

	connect(this, SIGNAL(playersReady()), this, SLOT(finish()), Qt::QueuedConnection);
	syncPlayers();
}

void ChessGame::finish()
{
	disconnect(this, SIGNAL(playersReady()), this, SLOT(finish()));
	for (int i = 0; i < 2; i++)
	{
		if (m_player[i] != nullptr)
			m_player[i]->disconnect(this);
	}

	emit finished(this, m_result);
}

void ChessGame::kill()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_player[i] != nullptr)
			m_player[i]->kill();
	}

	stop();
}

void ChessGame::addPgnMove(const Chess::Move& move, const QString& comment)
{
	PgnGame::MoveData md;
	md.key = m_currentBoard->key();
	md.move = m_currentBoard->genericMove(move);
	md.moveString = m_currentBoard->moveString(move, Chess::Board::StandardChinese);
	md.comment = comment;

	m_pgn->addMove(md);
}

void ChessGame::emitLastMove()
{
	int ply = m_moves.size() - 1;
	if (m_scores.contains(ply))
	{
		int score = m_scores[ply];
		if (score != MoveEvaluation::NULL_SCORE)
			emit scoreChanged(ply, score);
	}

	const auto& md = m_pgn->moves().last();
	emit moveMade(md.move, md.moveString, md.comment);
}

void ChessGame::onMoveMade(const Chess::Move& move)
{
	ChessPlayer* sender = qobject_cast<ChessPlayer*>(QObject::sender());
	Q_ASSERT(sender != nullptr);

	Q_ASSERT(m_gameInProgress);
	Q_ASSERT(m_currentBoard->isLegalMove(move));
	if (sender != playerToMove())
	{
		qWarning("%s tried to make a move on the opponent's turn",
			 qUtf8Printable(sender->name()));
		return;
	}

	m_scores[m_moves.size()] = sender->evaluation().score();
	m_moves.append(move);
	addPgnMove(move, evalString(sender->evaluation()));

	// Get the result before sending the move to the opponent
	m_currentBoard->makeMove(move);
	m_result = m_currentBoard->result();
	if (m_result.isNone())
	{
		if (m_currentBoard->reversibleMoveCount() == 0)
			m_adjudicator.resetDrawMoveCount();

		m_adjudicator.addEval(m_currentBoard, sender->evaluation());
		m_result = m_adjudicator.result();
	}
	m_currentBoard->undoMove();

	ChessPlayer* player = playerToWait();
	if (player->timeControl()->isHourglass()
		&& sender->timeControl()->isHourglass())
		player->addTime(sender->timeControl()->lastMoveTime());
	player->makeMove(move);
	m_currentBoard->makeMove(move);

	// 得到FEN
	this->m_fens.append(m_currentBoard->fenString());

	if (m_result.isNone())
	{
		emitLastMove();
		startTurn();
	}
	else
	{
		stop(false);
		emitLastMove();
	}
}

void ChessGame::startTurn()
{
	if (m_paused)
		return;

	Chess::Side side(m_currentBoard->sideToMove());
	Q_ASSERT(!side.isNull());

	emit humanEnabled(m_player[side]->isHuman());

	Chess::Move move(bookMove(side));
	if (move.isNull())
	{
		m_player[side]->go();
		m_player[side.opposite()]->startPondering();
	}
	else
	{
		m_player[side.opposite()]->clearPonderState();
		m_player[side]->makeBookMove(move);
	}
}

void ChessGame::PlayerMakeBookMove(Chess::Move move)
{
	//if (m_paused)
	//	return;

	Chess::Side side(m_currentBoard->sideToMove());
	Q_ASSERT(!side.isNull());

	emit humanEnabled(m_player[side]->isHuman());

	//Chess::Move move(bookMove(side));
	if (move.isNull())
	{
		//m_player[side]->go();
		//m_player[side.opposite()]->startPondering();
	}
	else
	{
		m_player[side.opposite()]->clearPonderState();
		m_player[side]->makeBookMove(move);
	}
}

void ChessGame::onAdjudication(const Chess::Result& result)
{
	if (m_finished || result.type() != Chess::Result::Adjudication)
		return;

	m_result = result;

	stop();
}

void ChessGame::onResignation(const Chess::Result& result)
{
	if (m_finished || result.type() != Chess::Result::Resignation)
		return;

	m_result = result;

	stop();
}

void ChessGame::onAdjustTimePerMove(int timePerMove)
{
	for (int i = 0; i < 2; i++) {
		if (m_player[i]->isHuman() == false) {
			//m_timeControl[i].setTimePerMove(timePerMove);
			m_player[i]->setTimePerMove(timePerMove);
		}
	}
}

void ChessGame::onResultClaim(const Chess::Result& result)
{
	if (m_finished)
		return;

	ChessPlayer* sender = qobject_cast<ChessPlayer*>(QObject::sender());
	Q_ASSERT(sender != nullptr);

	if (result.type() == Chess::Result::Disconnection)
	{
		// The engine may not be properly started so we have to
		// figure out the player's side this way
		Chess::Side side(Chess::Side::White);
		if (m_player[side] != sender)
			side = Chess::Side::Black;
		m_result = Chess::Result(result.type(), side.opposite());
	}
	else if (!m_gameInProgress && result.winner().isNull())
	{
		qWarning("Unexpected result claim from %s: %s",
			 qUtf8Printable(sender->name()),
			 qUtf8Printable(result.toVerboseString()));
	}
	else if (sender->areClaimsValidated() && result.loser() != sender->side())
	{
		qWarning("%s forfeits by invalid result claim: %s",
			 qUtf8Printable(sender->name()),
			 qUtf8Printable(result.toVerboseString()));
		m_result = Chess::Result(Chess::Result::Adjudication,
					 sender->side().opposite(),
					 "Invalid result claim");
	}
	else
		m_result = result;

	stop();
}

//void ChessGame::bookGetNextPosKeys(QVector<quint64>& keys)
//{
//
//	m_board->GetNextPosKeys(keys);
//	//QVarLengthArray<Chess::Move> moves;
//	////QVector<Move> legalMoves;
//
//	//m_board->generateMoves(moves);
//	//legalMoves.reserve(moves.size());
//
//	//for (int i = moves.size() - 1; i >= 0; i--)
//	//{
//	//	if (vIsLegalMove(moves[i]))
//	//		legalMoves << moves[i];
//	//}
//
//	//return legalMoves;
//	//
//	//
//	//
//	//const auto moves = m_board->legalMoves();
//	//quint64 preKey = m_board->key;
//
//	//for (const auto& move : moves){
//
//	//	makeMove(move);
//	//	bool isLegal = isLegalPosition();
//	//	undoMove();
//
//	//}
//}

Chess::Move ChessGame::bookMove(Chess::Side side)
{
	Q_ASSERT(!side.isNull());

	if (m_book[side] == nullptr
	||  m_moves.size() >= m_bookDepth[side] * 2)
		return Chess::Move();


	//
	//QVector<quint64> keys;
	//bookGetNextPosKeys(keys);


	Chess::GenericMove bookMove = m_book[side]->move(m_currentBoard->key());
	Chess::Move move = m_currentBoard->moveFromGenericMove(bookMove);				// 
	if (move.isNull())
		return Chess::Move();

	if (!m_currentBoard->isLegalMove(move))
	{
		qWarning("Illegal opening book move for %s: %s",
			 qUtf8Printable(side.toString()),
			 qUtf8Printable(m_currentBoard->moveString(move, Chess::Board::LongAlgebraic)));
		return Chess::Move();
	}

	if (m_currentBoard->isRepetition(move))
		return Chess::Move();

	return move;
}

void ChessGame::setError(const QString& message)
{
	m_error = message;
}

void ChessGame::setPlayer(Chess::Side side, ChessPlayer* player)
{
	Q_ASSERT(!side.isNull());
	Q_ASSERT(player != nullptr);
	m_player[side] = player;
}

void ChessGame::setStartingFen(const QString& fen)
{
	Q_ASSERT(!m_gameInProgress);
	//m_startingFen = fen;

	QStringList stFList = fen.split("moves");		// by LGL
	m_startingFen = stFList[0];

	if (stFList.length() == 2) {
		// 如果有move
		//m_board->setFenString(m_startingFen);

		QStringList strList = stFList[1].split(' ');
		QStringList::iterator token = strList.begin();
		++token;

		while (token != strList.end()) {
			QString str = *token;

			Chess::Move m = m_currentBoard->moveFromString(str);
			if (!m.isNull()) {	
				addPgnMove(m, "");            //
				m_currentBoard->makeMove(m);
			}
			else {
				qWarning("Fen Error! %s", qUtf8Printable(fen));
				//return true;
			}
			++token;
		}
	}
	return;
}

void ChessGame::setTimeControl(const TimeControl& timeControl, Chess::Side side)
{
	if (side != Chess::Side::White)
		m_timeControl[Chess::Side::Black] = timeControl;
	if (side != Chess::Side::Black)
		m_timeControl[Chess::Side::White] = timeControl;
}

void ChessGame::setMoves(const QVector<Chess::Move>& moves)
{
	Q_ASSERT(!m_gameInProgress);
	m_scores.clear();
	m_moves = moves;
}

bool ChessGame::setMoves(const PgnGame& pgn)
{
	setStartingFen(pgn.startingFenString());
	if (!resetBoard())
		return false;
	m_scores.clear();
	m_moves.clear();
	m_fens.clear();
	//m_fens.append(m_currentBoard->fenString());  // 这个不用

	for (const PgnGame::MoveData& md : pgn.moves())
	{
		Chess::Move move(m_currentBoard->moveFromGenericMove(md.move));
		if (!m_currentBoard->isLegalMove(move))
			return false;

		m_currentBoard->makeMove(move);
		if (!m_currentBoard->result().isNone())
			return true;

		m_moves.append(move);
		m_fens.append(m_currentBoard->fenString());
	}

	return true;
}

void ChessGame::setOpeningBook(const OpeningBook* book,
			       Chess::Side side,
			       int depth)
{
	Q_ASSERT(!m_gameInProgress);

	if (side.isNull())
	{
		setOpeningBook(book, Chess::Side::White, depth);
		setOpeningBook(book, Chess::Side::Black, depth);
	}
	else
	{
		m_book[side] = book;
		m_bookDepth[side] = depth;
	}
}

void ChessGame::setAdjudicator(const GameAdjudicator& adjudicator)
{
	m_adjudicator = adjudicator;
}

void ChessGame::generateOpening()
{
	if (m_book[Chess::Side::White] == nullptr || m_book[Chess::Side::Black] == nullptr)
		return;
	if (!resetBoard())
		return;

	// First play moves that are already in the opening
	for (const Chess::Move& move : qAsConst(m_moves))
	{
		Q_ASSERT(m_currentBoard->isLegalMove(move));

		m_currentBoard->makeMove(move);
		if (!m_currentBoard->result().isNone())
			return;
	}

	// Then play the opening book moves
	for (;;)
	{
		Chess::Move move = bookMove(m_currentBoard->sideToMove());
		if (move.isNull())
			break;

		m_currentBoard->makeMove(move);
		if (!m_currentBoard->result().isNone())
			break;

		m_moves.append(move);
		m_fens.append(m_currentBoard->fenString());
	}
}

void ChessGame::emitStartFailed()
{
	emit startFailed(this);
}

void ChessGame::setStartDelay(int time)
{
	Q_ASSERT(time >= 0);
	m_startDelay = time;
}

void ChessGame::setBookOwnership(bool enabled)
{
	m_bookOwnership = enabled;
}

void ChessGame::pauseThread()
{
	m_pauseSem.release();
	m_resumeSem.acquire();
}

//void ChessGame::dbIndicateAnnotationsOnBoard(MoveId moveId)
//{
//	QString annotation = squareAnnotation(moveId);
//	m_currentBoard->setSquareAnnotation(annotation);
//
//	annotation = arrowAnnotation(moveId);
//	m_currentBoard->setArrowAnnotation(annotation);
//}
//
//MoveId ChessGame::currentMove() const
//{
//	return m_currentNode;
//}

//bool ChessGame::dbMoveToId(MoveId moveId, QString* algebraicMoveList)
//{
//	moveId = nodeValid(moveId);
//	if (moveId == NO_MOVE)
//	{
//		return false;
//	}
//
//	if (m_currentNode != moveId)
//	{
//		//jump to node, travelling back to start adding the moves to the stack
//		MoveId node = moveId;
//		QStack<Chess::Move> moveStack;
//		while (node)
//		{
//			moveStack.push(m_moveNodes[node].move);
//			node = m_moveNodes[node].previousNode;
//		}
//
//		//reset the board, then make the moves on the stack to create the correct position
//		m_currentNode = moveId;
//		//*m_currentBoard = *m_startingBoard;
//		delete m_currentBoard;
//		m_currentBoard = m_startingBoard->copy();
//		while (!moveStack.isEmpty())
//		{
//			Chess::Move m = moveStack.pop();
//			m_currentBoard->makeMove(m);
//			//m_currentBoard->doMove(m);
//			if (algebraicMoveList)
//			{
//				if(m.isNull())//if (m.isNullMove())  // isNull
//				{
//					// Avoid trouble with a null move - UCI does not specify this and Stockfish makes nonsense
//					algebraicMoveList->clear();
//					algebraicMoveList = nullptr;
//				}
//				else
//				{
//					algebraicMoveList->push_back(m.toAlgebraic());
//					algebraicMoveList->push_back(" ");
//				}
//			}
//		}
//	}
//
//	return true;
//}

//MoveId ChessGame::nodeValid(MoveId moveId) const
//{
//	if (moveId == CURRENT_MOVE)
//	{
//		moveId = m_currentNode;
//	}
//	if ((moveId >= 0) && (moveId < m_moveNodes.size()))
//	{
//		if (m_moveNodes[moveId].Removed())
//		{
//			return NO_MOVE;
//		}
//		return moveId;
//	}
//	return NO_MOVE;
//}

//void ChessGame::moveToStart()
//{
//	m_currentNode = 0;
//	delete m_currentBoard;
//	m_currentBoard = m_startingBoard->copy();
//
//	indicateAnnotationsOnBoard(m_currentNode);
//}

//void ChessGame::indicateAnnotationsOnBoard(MoveId moveId)
//{
//	dbIndicateAnnotationsOnBoard(moveId);
//	emit signalMoveChanged();
//}

//QString ChessGame::squareAnnotation(MoveId moveId) const
//{
//	QString s = specAnnotation(QRegExp(s_csl), moveId);
//	return s;
//}
//
//QString ChessGame::specAnnotations(MoveId moveId, Position position) const
//{
//	QString s = annotation(moveId, position);
//	QString retval;
//	for( auto sr: s_specList)
//	{
//		QRegExp r(sr);
//		int pos = r.indexIn(s);
//		if (pos >= 0)
//		{
//			retval += r.cap(0);
//		}
//	}
//	return retval;
//}

//QString ChessGame::annotation(MoveId moveId, Position position) const
//{
//	MoveId node = nodeValid(moveId);
//
//	if ((position == AfterMove) || (node == 0))
//	{
//		return m_annotations.value(node, QString(""));
//	}
//	else
//	{
//		return m_variationStartAnnotations.value(node, QString(""));
//	}
//}

//bool ChessGame::dbSetAnnotation(QString annotation, MoveId moveId, Position position)
//{
//	MoveId node = nodeValid(moveId);
//	if (node == NO_MOVE)
//	{
//		return false;
//	}
//
//	int moves;
//	int comments;
//	int nags;
//	moveCount(&moves, &comments, &nags);
//
//	annotation.remove('}'); // Just make sure a comment does not destroy the database
//
//	if (position == AfterMove)
//	{
//		if (annotation.isEmpty())
//		{
//			m_annotations.remove(node);
//		}
//		else
//		{
//			m_annotations[node] = annotation;
//		}
//	}
//	else if (canHaveStartAnnotation(node))  	// Pre-move comment
//	{
//		if (node == 0)
//		{
//			setGameComment(annotation);
//		}
//		else
//		{
//			if (annotation.isEmpty() && (node > 1)) // Do not remove empty comment
//			{
//				m_variationStartAnnotations.remove(node);
//			}
//			else
//			{
//				m_variationStartAnnotations[node] = annotation;
//			}
//		}
//	}
//	return true;
//}

//void ChessGame::moveCount(int* moves, int* comments, int* nags) const
//{
//	*moves = *comments = 0;
//	if (nags) *nags = 0;
//
//	MoveId node = 1;
//	while ((node = nodeValid(node)) != NO_MOVE)
//	{
//		*moves += 1;
//		if (nags)
//		{
//			if (m_moveNodes[node].nags.count() != 0)
//			{
//				*nags += 1;
//			}
//		}
//		node = m_moveNodes[node].nextNode;
//	}
//	// Count comments
//	for (int i = 0; i < m_annotations.size(); ++i)
//	{
//		*comments += 1;
//	}
//	for (int i = 0; i < m_variationStartAnnotations.size(); ++i)
//	{
//		*comments += 1;
//	}
//}

//QString ChessGame::arrowAnnotation(MoveId moveId) const
//{
//	QString s = specAnnotation(QRegExp(s_cal), moveId);
//	return s;
//}

//bool ChessGame::canHaveStartAnnotation(MoveId moveId) const
//{
//	MoveId node = nodeValid(moveId);
//	return atLineStart(moveId) || atGameStart(m_moveNodes[node].previousNode);
//}

//QString ChessGame::specAnnotation(const QRegExp& r, MoveId moveId) const
//{
//	MoveId node = nodeValid(moveId);
//	if (node == NO_MOVE)
//	{
//		return QString("");
//	}
//
//	QString annotation = m_annotations[node];
//	if (annotation.isNull())
//	{
//		return QString("");
//	}
//
//	int pos = r.indexIn(annotation);
//	if (pos >= 0)
//	{
//		return r.cap(2);
//	}
//
//	return "";
//}

//bool ChessGame::atLineStart(MoveId moveId) const
//{
//	MoveId node = nodeValid(moveId);
//	if (node == NO_MOVE)
//	{
//		return false;
//	}
//	return (m_moveNodes[node].previousNode == m_moveNodes[node].parentNode)
//		|| m_moveNodes[node].previousNode == 0;
//}

//bool ChessGame::atGameStart(MoveId moveId) const
//{
//	return (nodeValid(moveId) == 0);
//}
//
//void ChessGame::setGameComment(const QString& gameComment)
//{
//	setAnnotation(gameComment, 0);
//}

//bool ChessGame::setAnnotation(QString annotation, MoveId moveId, Position position)
//{
//	ChessGame state = *this;
//	if (dbSetAnnotation(annotation, moveId, position))
//	{
//		dbIndicateAnnotationsOnBoard(currentMove());
//		emit signalGameModified(true, state, tr("Set annotation"));
//		return true;
//	}
//	return false;
//}

//bool ChessGame::isEmpty() const
//{
//	int moves;
//	int comments;
//	moveCount(&moves, &comments);
//	bool gameIsEmpty = ((moves + comments) == 0);
//	return gameIsEmpty;
//}

//bool ChessGame::isMainline(MoveId moveId) const
//{
//	if (moveId == 0)
//	{
//		return true;
//	}
//	MoveId node = nodeValid(moveId);
//	if (node == NO_MOVE)
//	{
//		return false;
//	}
//	else
//	{
//		return m_moveNodes[node].parentNode == NO_MOVE;
//	}
//}
//
//bool ChessGame::atLineEnd(MoveId moveId) const
//{
//	MoveId node = nodeValid(moveId);
//	if (node != NO_MOVE)
//	{
//		if (m_moveNodes[node].nextNode == NO_MOVE)
//		{
//			return true;
//		}
//	}
//
//	return false;
//}

//Chess::NagSet ChessGame::nags(MoveId moveId) const
//{
//	MoveId node = nodeValid(moveId);
//	if (node != NO_MOVE)
//	{
//		return m_moveNodes[node].nags;
//	}
//	return Chess::NagSet();
//}

//bool ChessGame::canMoveVariationUp(MoveId moveId) const
//{
//	if (isMainline())
//	{
//		return false;
//	}
//
//	MoveId variation = variationNumber(moveId);
//	MoveId parentNode = m_moveNodes[moveId].parentNode;
//
//	const QList <MoveId>& v = m_moveNodes[parentNode].variations;
//	int i = v.indexOf(variation);
//	return (i > 0);
//}

//bool ChessGame::canMoveVariationDown(MoveId moveId) const
//{
//	if (isMainline())
//	{
//		return false;
//	}
//
//	MoveId variation = variationNumber(moveId);
//	MoveId parentNode = m_moveNodes[moveId].parentNode;
//
//	const QList <MoveId>& v = m_moveNodes[parentNode].variations;
//	int i = v.indexOf(variation);
//	return (i >= 0 && (i + 1) < v.count());
//}

//MoveId ChessGame::variationNumber(MoveId moveId) const
//{
//	if (isMainline())
//	{
//		return 0;
//	}
//	MoveId node = nodeValid(moveId);
//	if (node != NO_MOVE)
//	{
//		MoveId parentNode = m_moveNodes[node].parentNode;
//
//		while (m_moveNodes[node].previousNode != parentNode)
//		{
//			node = m_moveNodes[node].previousNode;
//		}
//	}
//	return node;
//}

//bool ChessGame::setSquareAnnotation(QString squareAnnotation, MoveId moveId)
//{
//	squareAnnotation = squareAnnotation.trimmed();
//
//	MoveId node = nodeValid(moveId);
//	if (node == NO_MOVE)
//	{
//		return false;
//	}
//
//	QString s = annotation(moveId);
//	s.remove(QRegExp(s_csl));
//
//	if (!squareAnnotation.isEmpty())
//	{
//		s.append(QString("[%csl %1]").arg(squareAnnotation));
//	}
//	dbSetAnnotation(s, moveId);
//	indicateAnnotationsOnBoard(moveId);
//	return true;
//}

//QString ChessGame::specAnnotations(MoveId moveId, Position position) const
//{
//	QString s = annotation(moveId, position);
//	QString retval;
//	foreach(QString sr, s_specList)
//	{
//		QRegExp r(sr);
//		int pos = r.indexIn(s);
//		if (pos >= 0)
//		{
//			retval += r.cap(0);
//		}
//	}
//	return retval;
//}

void ChessGame::lockThread()
{
	if (QThread::currentThread() == thread())
		return;

	QMetaObject::invokeMethod(this, "pauseThread", Qt::QueuedConnection);
	m_pauseSem.acquire();
}

void ChessGame::unlockThread()
{
	if (QThread::currentThread() == thread())
		return;

	m_resumeSem.release();
}

bool ChessGame::resetBoard()
{
	QString fen(m_startingFen);
	if (fen.isEmpty())
	{
		fen = m_currentBoard->defaultFenString();
		//if (m_board->isRandomVariant())
		//	m_startingFen = fen;
	}

	if (!m_currentBoard->setFenString(fen))
	{
		qWarning("无效的 FEN 字符串: %s", qUtf8Printable(fen));
		m_currentBoard->reset();
		//if (m_board->isRandomVariant())
		//	m_startingFen = m_board->fenString();
		//else
			m_startingFen.clear();
		return false;
	}
	else if (!m_startingFen.isEmpty())
		m_startingFen = m_currentBoard->fenString();

	//this->isGetSetting = false; 

	return true;
}

void ChessGame::onPlayerReady()
{
	ChessPlayer* sender = qobject_cast<ChessPlayer*>(QObject::sender());
	Q_ASSERT(sender != nullptr);

	disconnect(sender, SIGNAL(ready()),
		   this, SLOT(onPlayerReady()));
	disconnect(sender, SIGNAL(disconnected()),
		   this, SLOT(onPlayerReady()));

	for (int i = 0; i < 2; i++)
	{
		if (!m_player[i]->isReady()
		&&  m_player[i]->state() != ChessPlayer::Disconnected)
			return;
	}

	emit playersReady();
}

void ChessGame::syncPlayers()
{
	bool ready = true;

	for (int i = 0; i < 2; i++)
	{
		ChessPlayer* player = m_player[i];
		Q_ASSERT(player != nullptr);

		if (!player->isReady()
		&&  player->state() != ChessPlayer::Disconnected)
		{
			ready = false;
			connect(player, SIGNAL(ready()),
				this, SLOT(onPlayerReady()));
			connect(player, SIGNAL(disconnected()),
				this, SLOT(onPlayerReady()));
		}
	}
	if (ready)
		emit playersReady();
}

void ChessGame::start()
{
	if (m_startDelay > 0)
	{
		QTimer::singleShot(m_startDelay, this, SLOT(start()));
		m_startDelay = 0;
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		connect(m_player[i], SIGNAL(resultClaim(Chess::Result)),
			this, SLOT(onResultClaim(Chess::Result)));
	}

	// Start the game in the correct thread
	connect(this, SIGNAL(playersReady()), this, SLOT(startGame()));
	QMetaObject::invokeMethod(this, "syncPlayers", Qt::QueuedConnection);


	m_result = Chess::Result();
	emit humanEnabled(false);
	resetBoard();
	initializePgn();
	emit initialized(this);
	emit fenChanged(m_currentBoard->startingFenString());
}

void ChessGame::pause()
{
	m_paused = true;
}

void ChessGame::resume()
{
	if (!m_paused)
		return;
	m_paused = false;

	QMetaObject::invokeMethod(this, "startTurn", Qt::QueuedConnection);  
	// 如果type是Qt :: QueuedConnection 则会发送一个QEvent，并在应用程序进入主事件循环后立即调用该成员。
}

void ChessGame::initializePgn()
{
	if (m_pgnInitialized)
		return;
	m_pgnInitialized = true;

	m_pgn->setVariant(m_currentBoard->variant());
	m_pgn->setStartingFenString(m_currentBoard->startingSide(), m_startingFen);
	m_pgn->setDate(QDate::currentDate());
	m_pgn->setPlayerName(Chess::Side::White, m_player[Chess::Side::White]->name());
	m_pgn->setPlayerName(Chess::Side::Black, m_player[Chess::Side::Black]->name());
	m_pgn->setResult(m_result);

	if (m_timeControl[Chess::Side::White] == m_timeControl[Chess::Side::Black])
		m_pgn->setTag("TimeControl", m_timeControl[0].toString());
	else
	{
		m_pgn->setTag("WhiteTimeControl", m_timeControl[Chess::Side::White].toString());
		m_pgn->setTag("BlackTimeControl", m_timeControl[Chess::Side::Black].toString());
	}
}

void ChessGame::startGame()
{
	disconnect(this, SIGNAL(playersReady()), this, SLOT(startGame()));
	if (m_finished)
		return;

	m_gameInProgress = true;
	for (int i = 0; i < 2; i++)
	{
		ChessPlayer* player = m_player[i];
		Q_ASSERT(player != nullptr);
		Q_ASSERT(player->isReady());

		if (player->state() == ChessPlayer::Disconnected)
		{
			setError(tr("Could not initialize player %1: %2")
			         .arg(player->name()).arg(player->errorString()));
			m_result = Chess::Result(Chess::Result::ResultError);
			stop();
			emitStartFailed();
			return;
		}
		if (!player->supportsVariant(m_currentBoard->variant()))		// 游戏变种处理
		{
			qWarning("%s doesn't support variant %s",
				 qUtf8Printable(player->name()),
				 qUtf8Printable(m_currentBoard->variant()));
			m_result = Chess::Result(Chess::Result::ResultError);
			stop();
			return;
		}
	}

	m_pgn->setPlayerName(Chess::Side::White, m_player[Chess::Side::White]->name());
	m_pgn->setPlayerName(Chess::Side::Black, m_player[Chess::Side::Black]->name());

	emit started(this);
	QDateTime gameStartTime = QDateTime::currentDateTime();
	m_pgn->setGameStartTime(gameStartTime);

	for (int i = 0; i < 2; i++)
	{
		Chess::Side side = Chess::Side::Type(i);

		Q_ASSERT(m_timeControl[side].isValid());
		m_player[side]->setTimeControl(m_timeControl[side]);
		m_player[side]->newGame(side, m_player[side.opposite()], m_currentBoard);
	}

	// Play the forced opening moves first
	for (int i = 0; i < m_moves.size(); i++)
	{
		Chess::Move move(m_moves.at(i));
		Q_ASSERT(m_currentBoard->isLegalMove(move));
		
		addPgnMove(move, "book");

		playerToMove()->makeBookMove(move);
		playerToWait()->makeMove(move);
		m_currentBoard->makeMove(move);
		
		emitLastMove();

		if (!m_currentBoard->result().isNone())
		{
			qWarning("Every move was played from the book");
			m_result = m_currentBoard->result();
			stop();
			return;
		}
	}
	
	for (int i = 0; i < 2; i++)
	{
		connect(m_player[i], SIGNAL(moveMade(Chess::Move)),
			this, SLOT(onMoveMade(Chess::Move)));
		if (m_player[i]->isHuman())
			connect(m_player[i], SIGNAL(wokeUp()),
				this, SLOT(resume()));
	}
	
	startTurn();
}
