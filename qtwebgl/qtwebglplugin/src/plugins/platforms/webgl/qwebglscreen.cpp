/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qtextstream.h>
#include <QtGui/qwindow.h>
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformcursor.h>
#include <private/qopenglcompositor_p.h>

#include "qwebglscreen.h"
#include "qwebglwindow.h"

QT_BEGIN_NAMESPACE

QWebGLScreen::QWebGLScreen()
{}

QWebGLScreen::QWebGLScreen(const QSize size, const QSizeF physicalSize) :
    m_size(size), m_physicalSize(physicalSize)
{}

QWebGLScreen::~QWebGLScreen()
{
    QOpenGLCompositor::destroy();
}

QRect QWebGLScreen::geometry() const
{
    return QRect(QPoint(0, 0), m_size);
}

int QWebGLScreen::depth() const
{
    return 32;
}

QImage::Format QWebGLScreen::format() const
{
    return QImage::Format_ARGB32;
}

QSizeF QWebGLScreen::physicalSize() const
{
    return m_physicalSize;
}

QDpi QWebGLScreen::logicalDpi() const
{
    return QPlatformScreen::logicalDpi();
}

qreal QWebGLScreen::pixelDensity() const
{
    return QPlatformScreen::pixelDensity();
}

Qt::ScreenOrientation QWebGLScreen::nativeOrientation() const
{
    return QPlatformScreen::nativeOrientation();
}

Qt::ScreenOrientation QWebGLScreen::orientation() const
{
    return QPlatformScreen::orientation();
}

qreal QWebGLScreen::refreshRate() const
{
    return 60;
}

QPixmap QWebGLScreen::grabWindow(WId wid, int x, int y, int width, int height) const
{
    QOpenGLCompositor *compositor = QOpenGLCompositor::instance();
    const QList<QOpenGLCompositorWindow *> windows = compositor->windows();
    Q_ASSERT(!windows.isEmpty());

    QImage img;

    if (static_cast<QWebGLWindow *>(windows.first()->sourceWindow()->handle())->isRaster()) {
        // Request the compositor to render everything into an FBO and read it back. This
        // is of course slow, but it's safe and reliable. It will not include the mouse
        // cursor, which is a plus.
        img = compositor->grab();
    } else {
        // Just a single OpenGL window without compositing. Do not support this case for now. Doing
        // glReadPixels is not an option since it would read from the back buffer which may have
        // undefined content when calling right after a swapBuffers (unless preserved swap is
        // available and enabled, but we have no support for that).
        qWarning("grabWindow: Not supported for non-composited OpenGL content. "
                 "Use QQuickWindow::grabWindow() instead.");
        return QPixmap();
    }

    if (!wid) {
        const QSize screenSize = geometry().size();
        if (width < 0)
            width = screenSize.width() - x;
        if (height < 0)
            height = screenSize.height() - y;
        return QPixmap::fromImage(img).copy(x, y, width, height);
    }

    foreach (QOpenGLCompositorWindow *w, windows) {
        const QWindow *window = w->sourceWindow();
        if (window->winId() == wid) {
            const QRect geom = window->geometry();
            if (width < 0)
                width = geom.width() - x;
            if (height < 0)
                height = geom.height() - y;
            QRect rect(geom.topLeft() + QPoint(x, y), QSize(width, height));
            rect &= window->geometry();
            return QPixmap::fromImage(img).copy(rect);
        }
    }

    return QPixmap();
}

void QWebGLScreen::setGeometry(int width, int height, const int physicalWidth,
                               const int physicalHeight)
{
    m_size = QSize(width, height);
    m_physicalSize = QSize(physicalWidth, physicalHeight);
    resizeMaximizedWindows();
}

QT_END_NAMESPACE
