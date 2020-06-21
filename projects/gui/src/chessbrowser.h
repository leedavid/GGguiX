#pragma once

#include <QtGui>
#include <QTextBrowser>
#include "editaction.h"
#include "chessengine.h"

class DatabaseInfo;
class QToolBar;

namespace Chess {
    /** @ingroup GUI
        The ChessBrowser class is a slightly modified QTextBrowser
        that handles internal pseudo-links. */
    class ChessBrowser : public QTextBrowser
    {
        Q_OBJECT
    public:
        /** Constructs new instance with parent @p parent. */
        ChessBrowser(QWidget* p);
        QToolBar* toolBar;
        QMap<QAction*, EditAction> m_actions;

        QStringList getAnchors(QList<MoveId> list);
    public slots:
        /** Store current configuration. */
        void saveConfig();
        /** Restore current configuration. */
        void slotReconfigure();
        /** Scroll to show given mode. */
        void showMove(int id);

        /** Invoke action */
        void slotAction(QAction* action);
        /** Show menu */
        void slotContextMenu(const QPoint& pos);
        /** Show the time in the String for the player with @p color */
        void slotDisplayTime(const QString& text, Side color, const QString& otherText);
        /** Show the material */
        void slotDisplayMaterial(const QList<double>& material);
        void slotDisplayPly(int ply);

    signals:
        void actionRequested(const EditAction& action);
        void queryActiveGame(const ChessGame** game);
        void signalMergeGame(GameId gameIndex, QString source);

    protected:
        virtual void selectAnchor(const QString& href);
        virtual void setSource(const QUrl& url);
        void setupMenu();
        QAction* createAction(const QString& name, EditAction::Type type);
        QAction* createNagAction(const Nag& nag);

        void configureFont();

    protected: // Drag+Drop
        void dragEnterEvent(QDragEnterEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dragLeaveEvent(QDragLeaveEvent* event);
        void dropEvent(QDropEvent* event);
        void mergeGame(GameId gameIndex);

        QStringList getAnchors(const QStringList& hrefs);
    private:

        QAction* m_copyHtml;
        QAction* m_copyText;
        QAction* m_uncomment;
        QAction* m_remove;
        QAction* m_startComment;
        QAction* m_gameComment;
        QAction* m_gameComment2;
        QAction* m_addComment;
        QAction* m_removeVariation;
        QAction* m_promoteVariation;
        QAction* m_VariationUp;
        QAction* m_VariationDown;
        QAction* m_removePrevious;
        QAction* m_removeNext;
        QAction* m_addNullMove;
        QAction* m_addNullMove2;
        QAction* m_removeNags;
        QAction* m_enumerateVariations1;
        QAction* m_enumerateVariations2;
        QMenu* m_gameMenu;
        QMenu* m_browserMenu;
        QMenu* m_mainMenu;
        int m_currentMove;
    };

}