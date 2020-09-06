#pragma once

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QMap>
#include <QSemaphore>
#include "pgngame.h"
#include "board/result.h"
#include "board/move.h"
#include "timecontrol.h"
#include "gameadjudicator.h"
//#include "../../gui/src/nag.h"
//#include "../../gui/src/gameid.h"
#include "board/boardfactory.h"

namespace Chess { class Board; }
class ChessPlayer;
class OpeningBook;
class MoveEvaluation;

//typedef short MoveId;

//#define ROOT_NODE 0
//#define NO_MOVE -1
//#define CURRENT_MOVE -2
//#define CURRENT_VARIATION -3

//class SaveRestoreMove;
//class SaveRestoreMoveCompact;

//typedef QHash<QString, QString> TagMap;
//typedef QHashIterator<QString, QString> TagMapIterator;



class LIB_EXPORT ChessGame : public QObject
{
	Q_OBJECT

	public:

		//static const QStringList s_specList;

		//enum MoveStringFlags
		//{
		//	MoveOnly = 0,      /**< Only the algebraic notation should be included */
		//	WhiteNumbers = 1,  /**< White moves should be preceded by a move number */
		//	BlackNumbers = 2,  /**< Black moves should be preceded by a move number */
		//	Nags = 4,          /**< Nags/symbolic annotation should be included */
		//	FullDetail = 7,    /**< Specifies all move numbers and nags should be included */
		//	TranslatePiece = 8 /**< Translate using the user-defined translation */
		//};

		//enum Position
		//{
		//	BeforeMove,
		//	AfterMove
		//};
		//enum NextPreviousMove
		//{
		//	NextMove,
		//	PreviousMove
		//};

		ChessGame(const ChessGame& game);
		ChessGame& operator=(const ChessGame& game);

		ChessGame(Chess::Board* board, PgnGame* pgn, QObject* parent = nullptr);
		virtual ~ChessGame();

		void mountBoard();
		void unmountBoard();

		//void clearTags();
		void clear();
		
		QString errorString() const;
		ChessPlayer* player(Chess::Side side) const;
		ChessPlayer* playerToMove() const;
		ChessPlayer* playerToWait() const;
		bool isFinished() const;
		bool boardShouldBeFlipped() const;
		void setBoardShouldBeFlipped(bool flip);

		void PlayerMakeBookMove(Chess::Move m);

		PgnGame* pgn() const;
		Chess::Board* board() const;
		QString startingFen() const;

		const QVector<Chess::Move>& moves() const;
		const QMap<int,int>& scores() const;
		const QVector<QString>& fens() const { return m_fens; };

		//QVector<QString> m_fens;           // by LGL 所有的fen

		Chess::Result result() const;

		void setError(const QString& message);
		void setPlayer(Chess::Side side, ChessPlayer* player);
		void setStartingFen(const QString& fen);
		void setTimeControl(const TimeControl& timeControl,
				    Chess::Side side = Chess::Side());
		void setMoves(const QVector<Chess::Move>& moves);
		bool setMoves(const PgnGame& pgn);
		void setOpeningBook(const OpeningBook* book,
				    Chess::Side side = Chess::Side(),
				    int depth = 1000);
		void setAdjudicator(const GameAdjudicator& adjudicator);
		void setStartDelay(int time);
		void setBookOwnership(bool enabled);

		void generateOpening();

		void lockThread();
		void unlockThread();
		void emitLastMovePub() {
			this->emitLastMove();
		};

		//bool isGetSetting;    // 是否从设置文件中得到了设置数据

		//void setIsLinkBoard(bool isLink) {	m_isLinkBoard = isLink;	};        // 是否连线下棋不要翻转	
		//bool getIsLinkBoard() { return m_isLinkBoard; };
		
		//void setIsEngingMatch(bool isMatch) { m_isEngingMatch = isMatch; };   // 是否引擎比赛
		//bool getIsEngingMatch() { return m_isEngingMatch; };
		

	

	public slots:
		void start();
		void pause();
		void resume();
		void stop(bool emitMoveChanged = true);
		void kill();
		void emitStartFailed();
		void onMoveMade(const Chess::Move& move);
		void onAdjudication(const Chess::Result& result);
		void onResignation(const Chess::Result& result);

		void onAdjustTimePerMove(int timePerMove);

	signals:
		void humanEnabled(bool);
		void fenChanged(const QString& fenString);
		void moveMade(const Chess::GenericMove& move,
			      const QString& sanString,
			      const QString& comment);

		void moveMadeFen(const QString& fen);  // 走子后发送一个FEN

		void moveChanged(int ply,
				 const Chess::GenericMove& move,
				 const QString& sanString,
				 const QString& comment);
		void scoreChanged(int ply, int score);
		void initialized(ChessGame* game = nullptr);
		void started(ChessGame* game = nullptr);
		void finished(ChessGame* game = nullptr,
			      Chess::Result result = Chess::Result());
		void startFailed(ChessGame* game = nullptr);
		void playersReady();

		void signalMoveChanged();
		void signalGameModified(bool, ChessGame, QString);

	private slots:
		void startGame();
		void startTurn();
		void finish();
		void onResultClaim(const Chess::Result& result);
		void onPlayerReady();
		void syncPlayers();
		void pauseThread();

	protected:
		//void dbIndicateAnnotationsOnBoard(MoveId moveId);


    public :/** @return current move id. */
		//MoveId currentMove() const;
		/** Moves to the position corresponding to the given move id */
		//bool dbMoveToId(MoveId moveId, QString* algebraicMoveList = nullptr);
		// ***** Moving through game *****
        /** Moves to the beginning of the game */
		//void moveToStart();
		/** Show annotations on the board for the Nose @p moveId. */
		//void indicateAnnotationsOnBoard(MoveId moveId)/*;*/
		/** @return squareAnnotation at move at node @p moveId. */
		//QString squareAnnotation(MoveId moveId = CURRENT_MOVE) const;

		/** Get a string with all special annotations including square brackets etc. */
		//QString specAnnotations(MoveId moveId = CURRENT_MOVE, Position position = AfterMove) const;

		/** @return comment at move at node @p moveId including visual hints for diagrams. */
		//QString annotation(MoveId moveId = CURRENT_MOVE, Position position = AfterMove) const;

		// **** node modification methods ****
        /** Sets the comment associated with move at node @p moveId */
		//bool dbSetAnnotation(QString annotation, MoveId moveId = CURRENT_MOVE, Position position = AfterMove);
		/** Sets the squareAnnotation associated with move at node @p moveId */
		//bool setSquareAnnotation(QString squareAnnotation, MoveId moveId = CURRENT_MOVE);
		/** Counts the number of moves, comments and nags, in mainline, to the end of the game */
		//void moveCount(int* moves, int* comments, int* nags = nullptr) const;
		/** @return arrowAnnotation at move at node @p moveId. */
		//QString arrowAnnotation(MoveId moveId = CURRENT_MOVE) const;
		/** @return @p true if a move can have annotation before the move. */
		//bool canHaveStartAnnotation(MoveId moveId = CURRENT_MOVE) const;
		
		/** @return annotation at move at node @p moveId. */
		//QString specAnnotation(const QRegExp& r, MoveId moveId = CURRENT_MOVE) const;

		/** @return whether the game is currently at the start position */
		//bool atLineStart(MoveId moveId = CURRENT_MOVE) const;
		//bool atGameStart(MoveId moveId = CURRENT_MOVE) const;
		/** set comment associated with game */
		//void setGameComment(const QString& gameComment);
		/** Sets the comment associated with move at node @p moveId */
		//bool setAnnotation(QString annotation, MoveId moveId = CURRENT_MOVE, Position position = AfterMove);
		/** Determine if game contains something reasonable */
		//bool isEmpty() const;
		/** @return whether the current position is in the mainline */
		//bool isMainline(MoveId moveId = CURRENT_MOVE) const;
		/** @return nags for move at node @p moveId */
		//bool atLineEnd(MoveId moveId = CURRENT_MOVE) const;
		//Chess::NagSet nags(MoveId moveId = CURRENT_MOVE) const;
		/** Decide if moveVariationUp() can be executed */
		//bool canMoveVariationUp(MoveId moveId) const;
		/** Decide if moveVariationDown() can be executed */
		//bool canMoveVariationDown(MoveId moveId) const;
		/** @return number of current variation */
		//MoveId variationNumber(MoveId moveId = CURRENT_MOVE) const;

	private:

//#pragma pack(push,2)
//		struct MoveNode
//		{
//			MoveId previousNode;
//			MoveId nextNode;
//			MoveId parentNode;
//			short m_ply;
//			Chess::Move move;
//			Chess::NagSet nags;
//			QList<MoveId> variations;
//			void remove()
//			{
//				parentNode = previousNode = nextNode = NO_MOVE;
//				variations.clear();
//				m_ply |= 0x8000;
//			}
//			MoveNode()
//			{
//				parentNode = nextNode = previousNode = NO_MOVE;
//				variations.clear();
//				m_ply = 0;
//			}
//			void SetPly(short ply) { Q_ASSERT(m_ply < 0x7FFF); m_ply = ply; }
//			short Ply() const { return m_ply & 0x7FFF; }
//			bool Removed() const { return (m_ply & 0x8000); }
//			inline bool operator==(const struct MoveNode& c) const
//			{
//				return (move == c.move &&
//					variations == c.variations &&
//					nags == c.nags &&
//					m_ply == c.m_ply);
//			}
//		};
//#pragma pack(pop)

		//void bookGetNextPosKeys(QVector<quint64>& keys);   // get next move position key list

		Chess::Move bookMove(Chess::Side side);
		bool resetBoard();
		void initializePgn();
		void addPgnMove(const Chess::Move& move, const QString& comment);
		void emitLastMove();
		
		//Chess::Board* m_board;
		ChessPlayer* m_player[2];
		TimeControl m_timeControl[2];
		const OpeningBook* m_book[2];
		int m_bookDepth[2];
		int m_startDelay;
		bool m_finished;
		bool m_gameInProgress;
		bool m_paused;
		bool m_pgnInitialized;
		bool m_bookOwnership;
		bool m_boardShouldBeFlipped;
		QString m_error;
		QString m_startingFen;
		Chess::Result m_result;
		QVector<Chess::Move> m_moves;
		QVector<QString> m_fens;           // by LGL 所有的fen
		QMap<int,int> m_scores;
		PgnGame* m_pgn;
		QSemaphore m_pauseSem;
		QSemaphore m_resumeSem;
		GameAdjudicator m_adjudicator;

		bool m_isLinkBoard = false;       // 是否要翻转棋盘, 连线下棋不要翻转	
		//bool m_isEngingMatch;     // 是不是引擎比赛, 

		// X chess =================================
		/** Reference Counter for this object */
		int mountRefCount;
		/** List of nodes */
		//QList <MoveNode> m_moveNodes;
		/** Keeps the current node in the game */
		//MoveId m_currentNode;
		/** Keeps the start ply of the game, 0 for standard starting position */
		//short m_startPly;
		/** Keeps the start position of the game */
		Chess::Board* m_startingBoard = Chess::BoardFactory::create("");
		/** Keeps the current position of the game */
		Chess::Board* m_currentBoard = Chess::BoardFactory::create("");

		//typedef QMap<MoveId, QString> AnnotationMap;
		/** Start annotations for each variation */
		//AnnotationMap m_variationStartAnnotations;
		/** Annotations for move nodes */
		//AnnotationMap m_annotations;

		/** Map keeping pgn tags of the game */
		//TagMap m_tags;

		// **** memory  management methods ****
	    /** Remove all removed nodes */
		//void compact();

		/** Checks if a moveId is valid, returns the moveId if it is, 0 if not */
		//MoveId nodeValid(MoveId moveId = CURRENT_MOVE) const;
		/** Change parent of each move of a variation. */
		//void reparentVariation(MoveId variation, MoveId parent);
		//void removeTimeCommentsFromMap(AnnotationMap& map);

		//friend class SaveRestoreMove;
		//friend class SaveRestoreMoveCompact;

};

//class SaveRestoreMove
//{
//public:
//	explicit SaveRestoreMove(ChessGame& game)
//	{
//		m_saveGame = &game;
//		m_saveMoveValue = game.currentMove();
//	}
//	~SaveRestoreMove()
//	{
//		m_saveGame->dbMoveToId(m_saveMoveValue);
//	}
//private:
//	ChessGame* m_saveGame;
//	MoveId m_saveMoveValue;
//};

//class SaveRestoreMoveCompact
//{
//public:
//	explicit SaveRestoreMoveCompact(ChessGame& game)
//	{
//		m_saveGame = &game;
//		m_saveMoveValue = game.currentMove();
//	}
//	~SaveRestoreMoveCompact()
//	{
//		m_saveGame->dbMoveToId(m_saveMoveValue);
//		m_saveGame->compact();
//	}
//private:
//	ChessGame* m_saveGame;
//	MoveId m_saveMoveValue;
//};


