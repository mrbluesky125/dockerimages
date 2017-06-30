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

#include "qwebglintegration.h"

#include "qwebglwebsocketserver.h"

#include <QtCore/qtextstream.h>
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformintegration.h>
#include <private/qguiapplication_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtGui/QOpenGLContext>
#include <private/qopenglcompositorbackingstore_p.h>

#include "qwebglwindow.h"

QT_BEGIN_NAMESPACE

QAtomicInt QWebGLWindow::s_id(1);

QWebGLWindow::QWebGLWindow(QWindow *w)
    : QPlatformWindow(w),
      m_backingStore(0),
      m_raster(false),
      m_flags(0)
{}

QWebGLWindow::~QWebGLWindow()
{
    destroy();
}

void QWebGLWindow::create()
{
    if (m_flags.testFlag(Created))
        return;

    m_id = s_id.fetchAndAddAcquire(1);

    // Save the original surface type before changing to OpenGLSurface.
    m_raster = (window()->surfaceType() == QSurface::RasterSurface);
    if (m_raster) // change to OpenGL, but not for RasterGLSurface
        window()->setSurfaceType(QSurface::OpenGLSurface);

    if (window()->windowState() == Qt::WindowFullScreen) {
        QRect fullscreenRect(QPoint(), screen()->availableGeometry().size());
        QPlatformWindow::setGeometry(fullscreenRect);
        QWindowSystemInterface::handleGeometryChange(window(), fullscreenRect);
        return;
    }

    m_flags = Created;

    if (window()->type() == Qt::Desktop)
        return;

    // Stop if there is already a window backed by a native window and surface. Additional
    // raster windows will not have their own native window, surface and context. Instead,
    // they will be composited onto the root window's surface.
    QWebGLScreen *screen = this->screen();
    QOpenGLCompositor *compositor = QOpenGLCompositor::instance();

    m_flags |= HasNativeWindow;
    setGeometry(window()->geometry()); // will become fullscreen
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(0, 0), geometry().size()));

    resetSurface();

    if (isRaster()) {
        QOpenGLContext *context = new QOpenGLContext(QGuiApplication::instance());
        context->setShareContext(qt_gl_global_share_context());
        context->setFormat(m_format);
        context->setScreen(window()->screen());
        if (Q_UNLIKELY(!context->create()))
            qFatal("QWebGL: Failed to create compositing context");
        compositor->setTarget(context, window(), screen->geometry());
        // If there is a "root" window into which raster and QOpenGLWidget content is
        // composited, all other contexts must share with its context.
        if (!qt_gl_global_share_context()) {
            qt_gl_set_global_share_context(context);
            // What we set up here is in effect equivalent to the application setting
            // AA_ShareOpenGLContexts. Set the attribute to be fully consistent.
            QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
        }
    }
}

void QWebGLWindow::destroy()
{
    if (m_flags.testFlag(HasNativeWindow)) {
        invalidateSurface();
    }

    m_flags = 0;
    QOpenGLCompositor::instance()->removeWindow(this);

    auto integration = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    auto clientData = integration->findClientData(surface()->surfaceHandle());

    const QVariantMap values {
        { "winId", winId() }
    };
    QMetaObject::invokeMethod(integration->m_webSocketServer, "sendTextMessage",
                              Q_ARG(QWebSocket *, clientData->socket),
                              Q_ARG(QWebGLWebSocketServer::TextMessageType,
                                    QWebGLWebSocketServer::TextMessageType::DestroyCanvas),
                              Q_ARG(QVariantMap, values));
    clientData->platformWindows.removeAll(this);
}

void QWebGLWindow::invalidateSurface()
{}

void QWebGLWindow::resetSurface()
{}

void QWebGLWindow::setBackingStore(QOpenGLCompositorBackingStore *backingStore)
{
    m_backingStore = backingStore;
}

void QWebGLWindow::setVisible(bool visible)
{
    QOpenGLCompositor *compositor = QOpenGLCompositor::instance();
    QList<QOpenGLCompositorWindow *> windows = compositor->windows();
    QWindow *wnd = window();

    if (wnd->type() != Qt::Desktop) {
        if (visible) {
            compositor->addWindow(this);
        } else {
            compositor->removeWindow(this);
            windows = compositor->windows();
            if (windows.size())
                windows.last()->sourceWindow()->requestActivate();
        }
    }

    QWindowSystemInterface::handleExposeEvent(wnd, QRect(QPoint(0, 0), wnd->geometry().size()));

    if (visible)
        QWindowSystemInterface::flushWindowSystemEvents();
}

void QWebGLWindow::setGeometry(const QRect &rect)
{
    QWindowSystemInterface::handleGeometryChange(window(), rect);
    QPlatformWindow::setGeometry(rect);
}

QRect QWebGLWindow::geometry() const
{
    return QPlatformWindow::geometry();
}

void QWebGLWindow::requestActivateWindow()
{
    if (window()->type() != Qt::Desktop)
        QOpenGLCompositor::instance()->moveToTop(this);

    QWindow *wnd = window();
    QWindowSystemInterface::handleWindowActivated(wnd);
    QWindowSystemInterface::handleExposeEvent(wnd, QRect(QPoint(0, 0), wnd->geometry().size()));
}

void QWebGLWindow::raise()
{
    QWindow *wnd = window();
    if (wnd->type() != Qt::Desktop) {
        QOpenGLCompositor::instance()->moveToTop(this);
        QWindowSystemInterface::handleExposeEvent(wnd, QRect(QPoint(0, 0), wnd->geometry().size()));
    }
}

void QWebGLWindow::lower()
{
    QOpenGLCompositor *compositor = QOpenGLCompositor::instance();
    QList<QOpenGLCompositorWindow *> windows = compositor->windows();
    if (window()->type() != Qt::Desktop && windows.count() > 1) {
        int idx = windows.indexOf(this);
        if (idx > 0) {
            compositor->changeWindowIndex(this, idx - 1);
            const auto size = windows.last()->sourceWindow()->geometry().size();
            QWindowSystemInterface::handleExposeEvent(windows.last()->sourceWindow(),
                                                      QRect(QPoint(0, 0), size));
        }
    }
}

QSurfaceFormat QWebGLWindow::format() const
{
    return m_format;
}

QWebGLScreen *QWebGLWindow::screen() const
{
    return static_cast<QWebGLScreen *>(QPlatformWindow::screen());
}

bool QWebGLWindow::isRaster() const
{
    return m_raster || window()->surfaceType() == QSurface::RasterGLSurface;
}

QWindow *QWebGLWindow::sourceWindow() const
{
    return window();
}

const QPlatformTextureList *QWebGLWindow::textures() const
{
    if (m_backingStore)
        return m_backingStore->textures();

    return 0;
}

void QWebGLWindow::endCompositing()
{
    if (m_backingStore)
        m_backingStore->notifyComposited();
}

WId QWebGLWindow::winId() const
{
    return m_id;
}

void QWebGLWindow::setOpacity(qreal)
{
    if (!isRaster())
        qWarning("QWebGLWindow: Cannot set opacity for non-raster windows");

    // Nothing to do here. The opacity is stored in the QWindow.
}

QT_END_NAMESPACE
