#pragma once

#include "linkboard.h"

#include <Windows.h>
#include <QString>
#include <QPixmap>
#include <QProcess>
#include <Qthread>
#include <QMessagebox>
#include <QMutex>

#include <chessgame.h>


namespace Chess {	

	class Capture : public QThread
	{
		Q_OBJECT

		enum window_search_mode {
			INCLUDE_MINIMIZED,
			EXCLUDE_MINIMIZED
		};

	public:	


	signals:
		void CapSendSignal(stCaptureMsg msg);

	public slots:
		void ProcessBoardMove(const Chess::GenericMove& move);
	
		
	private:

		void run() Q_DECL_OVERRIDE;


	public:

		explicit Capture(QObject* parent, QString catName, bool isAuto = false);

		~Capture();

		void SetCatlogName(QString catName) { this->m_linkBoard->SetCatlogName(catName); };

		void on_start();
		void on_stop();
		void on_pause();
		
		void SendMessageToMain(const QString title, const QString msg);
		void SendMoveToMain(const Chess::GenericMove m);
		void SendFenToMain(const QString fen);

	private:

		LinkBoard* m_linkBoard;		


	private:		

		MainWindow* pMain;
		
	};

};