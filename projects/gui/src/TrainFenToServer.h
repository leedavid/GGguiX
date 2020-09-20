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
		ADD_Draw_Too_High,    // �߷ֺ���
		DEL_FEN,
		COMMON_FEN,
		REMOVE_FEN,
		TIMER_DEL_FEN_ON,     // ��ʱɾ��һ��FEN
		TIMER_DEL_FEN_OFF,
		LAST_50               // ���50������
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

	public slots:
		void handleTimeout();  //��ʱ������


	private:
		void run() Q_DECL_OVERRIDE;
		int getWebInfoByQuery(QUrl url, QString& res);

		bool FenAddLostGame();
		bool FenAddDrawTooHigh();   // �߷ֺ���
		void InfoStatic();         // ����ͳ����Ϣ
		bool FenDelete();
		bool FenCommon();
		bool FenRemoveAll();
		bool FenLast50();

		void TimerFenDelOneON();
		void TimerFenDelOneOFF();

		bool GetTfenTsideFromFEN(QString fen, QString& tfen, QString& tside);
		bool addOneFenToWEB(QString fen, int score = 0, bool first = false);
		bool delOneFenFromWEB(QString fen);
		bool delFirstOneFenFrom();

		bool AddOneFenToSetting(QString fen);
		bool GetAndDelOneFenFromSetting(QString& fen);  // ȡһ����ͬʱɾ������
		bool GetSetting();
		int getDelDelayMs();

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
		int _drawHighScore;
		int _drawHignScoreNum;
		bool _autoUpload; 
		bool _saveBlind;
		//ChessGame* _game;
		QString _startFen;
		QVector<QString> _fens;           // by LGL ���е�fen
		QMap<int, int> _scores;

		CTrainFenMethod _method;

		QTimer* _timer;
		QThread* _timerThread;
		static int ServerFENnum;      // 

		static int DelFenNum;  
		static int AddFenNum;
		static int DelFenDelayMs;

		const static int MaxFEN = 490;


	private:

		MainWindow* pMain;

	};
};