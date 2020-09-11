#include "capture.h"

#include <QThread>
#include <QScreen>
#include <QPixmap>
#include <QRect>
#include <QDir>
#include <QGuiApplication>
#include <QMessagebox>
#include <board/boardfactory.h>
#include <QMutex>

#include "mainwindow.h"

namespace Chess {

	// ��ʼ����̬��Ա
	//QMutex Capture::mutex;
	//bool Capture::m_MayNewGame = false;

	Capture::Capture(QObject* parent, QString catName, bool isAuto)
		:QThread(parent),
		pMain((MainWindow*)(parent))
	{
		//m_linkBoard = new LinkBoard(pMain, this, "��������", isAuto);
		m_linkBoard = new LinkBoard(pMain, this, catName, isAuto);

		connect(this, SIGNAL(CapSendSignal(stCaptureMsg)),
			parent, SLOT(slotProcessCapMsg(stCaptureMsg)));
	}



	//stCaptureMsg m_msg;      // �ǲ��ǿ��ܷ���
	//stCaptureMsg m_msg;      // �ǲ��ǿ��ܷ���
	void Capture::SendMessageToMain(const QString title, const QString msg)
	{
		stCaptureMsg m_msg;      // �ǲ��ǿ��ܷ���
		m_msg.mType = eCapMsgType::eText;
		m_msg.text = title;
		m_msg.title = msg;
		emit CapSendSignal(m_msg);
	}

	void Capture::SendMoveToMain(const Chess::GenericMove m)
	{
		stCaptureMsg m_msg;      // �ǲ��ǿ��ܷ���
		m_msg.mType = eCapMsgType::eMove;
		m_msg.m = m;
		emit CapSendSignal(m_msg);
	}

	void Capture::SendFenToMain(const QString fen)
	{
		stCaptureMsg m_msg;      // �ǲ��ǿ��ܷ���
		m_msg.mType = eCapMsgType::eSetFen;
		m_msg.text = fen;
		emit CapSendSignal(m_msg);
	}

	// �������淢�����岽���������
	void Capture::ProcessBoardMove(const Chess::GenericMove& move) {

		this->m_linkBoard->ProcessBoardMove(move);
	}



	// �߳�����
	void Capture::run() {	

		this->m_linkBoard->run();
	}

	// �����߳�
	void Capture::on_start()
	{
		this->start();	
	}

	void Capture::on_stop()
	{
		//bMustStop = true;
		this->m_linkBoard->setStop(true);
	}

	void Capture::on_pause()
	{
	}

	Capture::~Capture()
	{
		while (isRunning()) {
			on_stop();
			wait(1);
		}
		delete m_linkBoard;
		//delete this->m_board;		
	}
	};  // namespace Chess 
	
