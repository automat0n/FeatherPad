/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2014 <tsujan2000@gmail.com>
 *
 * FeatherPad is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FeatherPad is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui>
#include <QApplication>
#include <QToolTip>
#include "tabbar.h"

namespace FeatherPad {

TabBar::TabBar (QWidget *parent)
    : QTabBar (parent)
{
    setMouseTracking (true);
}
/*************************/
void TabBar::mousePressEvent (QMouseEvent *event)
{
    QTabBar::mousePressEvent (event);
    if (event->button() == Qt::LeftButton)
        dragStartPosition_ = event->pos();
    dragStarted_ = false;
}
/*************************/
void TabBar::mouseMoveEvent (QMouseEvent *event)
{
    if (!dragStartPosition_.isNull()
        && (event->pos() - dragStartPosition_).manhattanLength() < QApplication::startDragDistance())
    {
      dragStarted_ = true;
    }

    if ((event->buttons() & Qt::LeftButton)
        && dragStarted_
        && !window()->geometry().contains (event->globalPos()))
    {
        int index = currentIndex();
        if (index == -1) return;

        QDrag *drag = new QDrag (this);
        QMimeData *mimeData = new QMimeData;
        QByteArray array;
        array.append (QString ("%1+%2").arg (window()->winId()).arg (index));
        mimeData->setData ("application/featherpad-tab", array);
        drag->setMimeData (mimeData);
        Qt::DropAction dragged = drag->exec (Qt::MoveAction | Qt::CopyAction);
        if (dragged == Qt::IgnoreAction) // a tab is dropped outside all windows
        {
            if (count() > 1)
                emit tabDetached();
            else
                finishMouseMoveEvent();
            event->accept ();
        }
        else if (dragged == Qt::MoveAction) // a tab is dropped into another window
            event->accept();
        delete drag;
        drag = NULL;
    }
    else
    {
        QTabBar::mouseMoveEvent(event);
        int index = tabAt (event->pos());
        if (index > -1)
            QToolTip::showText (event->globalPos(), tabToolTip (index));
    }
}
/*************************/
// Don't show tooltip with setTabToolTip().
bool TabBar::event (QEvent *event)
{
#ifndef QT_NO_TOOLTIP
    if (event->type() == QEvent::ToolTip)
        return QWidget::event (event);
    else
       return QTabBar::event (event);
#endif
    return QTabBar::event (event);
}
/*************************/
void TabBar::finishMouseMoveEvent()
{
    QMouseEvent finishingEvent (QEvent::MouseMove, QPoint(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    mouseMoveEvent (&finishingEvent);
}
/*************************/
void TabBar::releaseMouse()
{
    QMouseEvent releasingEvent (QEvent::MouseButtonRelease, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    mouseReleaseEvent (&releasingEvent);
}

}
