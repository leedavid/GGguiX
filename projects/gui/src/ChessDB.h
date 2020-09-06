#pragma once



#include <QPixmap>
#include <QProcess>
#include <Qthread>
#include <QMessagebox>
#include <QMutex>

#include <QNetworkrequest>
#include <QNetworkaccessmanager>
#include <QEventloop>
#include <QNetworkreply>
#include <QUrlquery>

#include <chessgame.h>
#include <board/board.h>
#include <board/boardfactory.h>
#include "uciengine.h"

//#include "linkboard.h"


class MainWindow;

namespace Chess {

	struct stCaptureMsg;




	class ChessDB : public QThread
	{
		Q_OBJECT

	public:
		explicit ChessDB(QObject* parent);
		~ChessDB();


		int setEndGame(bool e) { this->m_Endgame = e; };
		void on_start();
		void on_stop() { this->m_bMustStop = true; };


	public
    slots:
		void FindChessDBmoveByFEN(const QString& fen);

	signals:
		void SendSignalStatus(int which, QString msg);   // 发送状态档提示信息
		//void CapSendSignal(stCaptureMsg msg);

	private:
		void run() Q_DECL_OVERRIDE;
		int getWebInfoByQuery(QUrl url, QString& res);
		int Query(const QString& Fen, QString& Res, CHESSDB_QUERY_TYPE type = CHESSDB_QUERY_TYPE::QUERY_TYPE_BEST);
		void QueryDB();

	private:
		MainWindow* pMain;
		QString m_BanMove;
		QString m_newFen;
		bool m_Endgame;
		bool m_findMove;
		bool m_bInputNewFen;
		CHESSDB_ENDGAME_TYPE m_bEndGameType;
		QString m_WebAddress;
		bool m_bMustStop; 
	};
}

