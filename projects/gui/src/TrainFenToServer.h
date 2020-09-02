#pragma once

#include "linkboard.h"

#include <Windows.h>
#include <QString>
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


namespace Chess {

	enum class CTrainFenMethod {
		ADD_FEN = 0,
		DEL_FEN,
		COMMON_FEN,
		REMOVE_FEN
	};

	class CTrainFen : public QThread
	{
		Q_OBJECT


	public:
		explicit CTrainFen(QObject* parent);

		~CTrainFen();

		void on_start2(CTrainFenMethod method, ChessGame* game = nullptr);

		//void RunAddFen();


	signals:
		void SendSignal(int which, QString msg);
		//void SendSignal(QString msg);


	private:
		void run() Q_DECL_OVERRIDE;
		int getWebInfoByQuery(QUrl url, QString& res);

		bool FenAdd();
		bool FenDelete();
		bool FenCommon();
		bool FenRemoveAll();

		bool GetTfenTsideFromFEN(QString fen, QString& tfen, QString& tside);
		bool addOneFenToWEB(QString fen, int score = 0, bool first = false);
		bool delOneFenFromWEB(QString fen);

		bool AddOneFenToSetting(QString fen);
		bool GetAndDelOneFenFromSetting(QString& fen);  // 取一条，同时删除那条
		bool GetSetting();

	private:
		QString _userName;
		QString _passWord;
		QString _WebAddress;
		int _trainID;
		int _maxScore;
		int _minScore;
		int _maxSteps;
		int _minSteps;
		int _stepsGap;
		bool _autoUpload; 
		//ChessGame* _game;
		QString _startFen;
		QVector<QString> _fens;           // by LGL 所有的fen
		QMap<int, int> _scores;

		CTrainFenMethod _method;

	private:

		MainWindow* pMain;

	};
};