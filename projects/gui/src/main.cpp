/*
    This file is part of GGzero Chess.
    Copyright (C) 2008-2018 GGzero Chess authors

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

#include "cutechessapp.h"
#include <QStyleFactory>
#include <QLoggingCategory>
#include <QTextStream>
#include <QStringList>
#include <QMetaType>
#include <QTranslator>

#include <board/genericmove.h>
#include <board/move.h>
#include <board/side.h>
#include <board/result.h>
#include <moveevaluation.h>

#include "capture.h"
#include "logstream.h"
#include "settings.h"

// Necessary includes and defines for memory leak detection:
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER


#if defined(_MSC_VER)

// Code to display the memory leak report
// We use a custom report hook to filter out Qt's own memory leaks
// Credit to Andreas Schmidts - http://www.schmidt-web-berlin.de/winfig/blog/?p=154

_CRT_REPORT_HOOK prevHook;

int customReportHook(int /* reportType */, char* message, int* /* returnValue */) {
	// This function is called several times for each memory leak.
	// Each time a part of the error message is supplied.
	// This holds number of subsequent detail messages after
	// a leak was reported
	const int numFollowupDebugMsgParts = 2;
	static bool ignoreMessage = false;
	static int debugMsgPartsCount = 0;

	// check if the memory leak reporting starts
	if ((strncmp(message, "Detected memory leaks!\n", 10) == 0)
		|| ignoreMessage)
	{
		// check if the memory leak reporting ends
		if (strncmp(message, "Object dump complete.\n", 10) == 0)
		{
			_CrtSetReportHook(prevHook);
			ignoreMessage = false;
		}
		else
			ignoreMessage = true;

		// something from our own code?
		if (strstr(message, ".cpp") == NULL)
		{
			if (debugMsgPartsCount++ < numFollowupDebugMsgParts)
				// give it back to _CrtDbgReport() to be printed to the console
				return FALSE;
			else
				return TRUE;  // ignore it
		}
		else
		{
			debugMsgPartsCount = 0;
			// give it back to _CrtDbgReport() to be printed to the console
			return FALSE;
		}
	}
	else
		// give it back to _CrtDbgReport() to be printed to the console
		return FALSE;
}

#endif

int main(int argc, char* argv[])
{
	
#if defined(_MSC_VER)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	prevHook = _CrtSetReportHook(customReportHook);
	// _CrtSetBreakAlloc(157); // Use this line to break at the nth memory allocation
#endif

	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);     // HiDPI support
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // Windows Surface Book // 
	
	// Register types for signal / slot connections
	qRegisterMetaType<Chess::GenericMove>("Chess::GenericMove");
	qRegisterMetaType<Chess::Move>("Chess::Move");
	qRegisterMetaType<Chess::Side>("Chess::Side");
	qRegisterMetaType<Chess::Result>("Chess::Result");
	qRegisterMetaType<MoveEvaluation>("MoveEvaluation");

	qRegisterMetaType<Chess::stCaptureMsg>("Chess::stCaptureMsg");

	QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

	CuteChessApplication app(argc, argv);

	
	// Setting =================================================================================
	QString portableIni = Chess::Settings::portableIniPath();
	if (QFile::exists(portableIni))
	{
		Chess::AppSettings = new Chess::Settings(portableIni);
	}
	if (!Chess::AppSettings)
	{
		Chess::AppSettings = new Chess::Settings;
	}

	if (Chess::AppSettings->getValue("/MainWindow/DarkTheme").toBool())
	{
		// set style
		app.setStyle(QStyleFactory::create("Fusion"));
		// modify palette to dark
		QPalette darkPalette;
		darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::WindowText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
		darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
		darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
		darkPalette.setColor(QPalette::ToolTipBase, QColor(20, 20, 20));
		darkPalette.setColor(QPalette::ToolTipText, Qt::yellow);
		darkPalette.setColor(QPalette::Text, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
		darkPalette.setColor(QPalette::Disabled, QPalette::Light, QColor(0, 0, 0, 0));
		darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
		darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
		darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ButtonText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
		darkPalette.setColor(QPalette::BrightText, Qt::red);
		darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
		darkPalette.setColor(QPalette::HighlightedText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

		app.setPalette(darkPalette);
	}
	else
	{
		app.setStyle("plastique");
		QPalette lightPalette = qApp->palette();
		lightPalette.setColor(QPalette::BrightText, Qt::red);
		app.setPalette(lightPalette);
	}

	// Setting =================================================================================	


	// ”Ô—‘…Ë÷√
	QTranslator translator;
	translator.load(QLocale(), "GGzeroGui", "_", "translations", ".qm");
	app.installTranslator(&translator);

	QStringList arguments = app.arguments();
	arguments.takeFirst(); // application name

	// Use trivial command-line parsing for now
	QTextStream out(stdout);
	while (!arguments.isEmpty())
	{
		if (arguments.first() == QLatin1String("-v") ||
			arguments.first() == QLatin1String("--version"))
		{
			out << "GGzeroGui Chess " << CUTECHESS_VERSION << endl;
			out << "Using Qt version " << qVersion() << endl << endl;
			out << "Copyright (C) 2008-2018 LeeDavid " << endl;
			out << "This is free software; see the source for copying ";
			out << "conditions.  There is NO" << endl << "warranty; not even for ";
			out << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.";
			out << endl << endl;

			return 0;
		}
		else
		{
			out << "Unknown argument: " << arguments.first() << endl;
		}
		arguments.takeFirst();
	}
	app.newDefaultGame();

	startFileLog();
	int result = app.exec();
	stopFileLog();


#if defined(_MSC_VER)
	// Once the app has finished running and has been deleted,
	// we run this command to view the memory leaks:
	_CrtDumpMemoryLeaks();
#endif

	return result;

}
