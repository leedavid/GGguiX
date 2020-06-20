#pragma once

#include <QObject>

namespace Chess {

    class MessageDialog : public QObject
    {
        Q_OBJECT
    public:
        enum { Yes, No, Cancel };
        /** Standard information dialog. */
        static void error(const QString& text, const QString& title = tr("Error"));
        /** Standard warning dialog. */
        static void warning(const QString& text, const QString& title = tr("Warning"));
        /** Standard information dialog. */
        static void information(const QString& text, const QString& title = tr("Information"));
        /** Standard Yes-No dialog. Escape is disabled, 'No' is default. */
        static bool yesNo(const QString& text, const QString& title = tr("Question"),
            const QString& yes = tr("&Yes"), const QString& no = tr("&No"));
        /** Standard Ok-Cancel dialog. Escape cancels, 'Ok' is default. */
        static bool okCancel(const QString& text, const QString& title = tr("Question"),
            const QString& ok = tr("&OK"), const QString& cancel = tr("&Cancel"));
        static int yesNoCancel(const QString& text, const QString& title = tr("Question"),
            const QString& yes = tr("&Yes"), const QString& no = tr("&No"),
            const QString& cancel = tr("&Cancel"));
    };
}
