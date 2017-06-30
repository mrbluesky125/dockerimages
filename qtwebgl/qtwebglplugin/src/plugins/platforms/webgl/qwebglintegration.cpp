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

#include "qwebglwindow.h"
#include "qwebglcontext.h"
#include "qwebglcontext.h"
#include "qwebglhttpserver.h"
#include "qwebglwebsocketserver.h"
#include "qwebgloffscreenwindow.h"
#include "qwebglplatformservices.h"
#include "qwebglplatformclipboard.h"

#include <QtCore/qstring.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qloggingcategory.h>

#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOffscreenSurface>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatforminputcontextfactory_p.h>

#include <private/qgenericunixthemes_p.h>


#include <QtGui/qclipboard.h>

#include <QtWebSockets/qwebsocket.h>

#include <private/qopenglcompositorbackingstore_p.h>
#ifdef Q_OS_WIN
#include <private/qwindowsfontdatabase_p.h>
#include <private/qwindowsguieventdispatcher_p.h>
#else
//#include <private/qgenericunixservices_p.h>
#include <private/qgenericunixfontdatabase_p.h>
#include <private/qgenericunixeventdispatcher_p.h>
#endif // Q_OS_WIN

Q_DECLARE_METATYPE(QWebSocket *)

QT_BEGIN_NAMESPACE

QWebGLIntegration::QWebGLIntegration(quint16 port) :
      m_inputContext(0),
      m_httpPort(port),
#if defined(Q_OS_WIN)
      m_fontDatabase(new QWindowsFontDatabase),
#else
      m_fontDatabase(new QGenericUnixFontDatabase),
#endif
      m_services(new QWebGLPlatformServices),
      m_clipboard(new QWebGLPlatformClipboard)
{
    qRegisterMetaType<QWebGLWebSocketServer::TextMessageType>(
                "QWebGLWebSocketServer::TextMessageType");
}

void QWebGLIntegration::initialize()
{
    m_inputContext = QPlatformInputContextFactory::create();
    m_screen = new QWebGLScreen;
    screenAdded(m_screen, true);

    m_webSocketServer = new QWebGLWebSocketServer;
    m_httpServer = new QWebGLHttpServer(m_webSocketServer, this);
    bool ok = m_httpServer->listen(QHostAddress::Any, m_httpPort);
    if (!ok)
        qFatal("QWebGLIntegration::initialize: Failed to initialize");
    m_webSocketServerThread = new QThread(this);
    m_webSocketServer->moveToThread(m_webSocketServerThread);
    connect(m_webSocketServerThread, &QThread::finished, m_webSocketServer, &QObject::deleteLater);
    QMetaObject::invokeMethod(m_webSocketServer, "create", Qt::QueuedConnection);
    QMutexLocker lock(m_webSocketServer->mutex());
    m_webSocketServerThread->start();
    m_webSocketServer->waitCondition()->wait(m_webSocketServer->mutex());

    connect(m_webSocketServer, &QWebGLWebSocketServer::canvasResized, this,
            [=](QWebSocket *socket,
                const int width,
                const int height,
                const int physicalWidth,
                const int physicalHeight)
    {
        auto clientData = findClientData(socket);
        clientData->platformScreen->setGeometry(width, height, physicalWidth, physicalHeight);
    }, Qt::QueuedConnection);
    connect(m_webSocketServer, &QWebGLWebSocketServer::clientConnected, this,
            &QWebGLIntegration::onClientConnected);
    connect(m_webSocketServer, &QWebGLWebSocketServer::clientDisconnected, this,
            &QWebGLIntegration::onClientDisconnected);

    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this,
            &QWebGLIntegration::broadcastClipboard);
}

void QWebGLIntegration::destroy()
{
    foreach (QWindow *w, qGuiApp->topLevelWindows())
        w->destroy();

    destroyScreen(m_screen);

    m_screen = nullptr;

    m_webSocketServerThread->quit();
    m_webSocketServerThread->wait();
    delete m_webSocketServerThread;
}

QAbstractEventDispatcher *QWebGLIntegration::createEventDispatcher() const
{
#ifdef Q_OS_WIN
    return new QWindowsGuiEventDispatcher;
#else
    return createUnixEventDispatcher();
#endif // Q_OS_WIN
}

QPlatformServices *QWebGLIntegration::services() const
{
    return m_services.data();
}

QPlatformFontDatabase *QWebGLIntegration::fontDatabase() const
{
    return m_fontDatabase.data();
}

QPlatformTheme *QWebGLIntegration::createPlatformTheme(const QString &name) const
{
#ifdef Q_OS_WIN
    return QPlatformIntegration::createPlatformTheme(name);
#else
    return QGenericUnixTheme::createUnixTheme(name);
#endif // Q_OS_WIN
}

QPlatformClipboard *QWebGLIntegration::clipboard() const
{
    return (QPlatformClipboard *)m_clipboard.data();
}

QPlatformBackingStore *QWebGLIntegration::createPlatformBackingStore(QWindow *window) const
{
    QOpenGLCompositorBackingStore *bs = new QOpenGLCompositorBackingStore(window);
    if (!window->handle())
        window->create();
    static_cast<QWebGLWindow *>(window->handle())->setBackingStore(bs);
    return bs;
}

QPlatformWindow *QWebGLIntegration::createPlatformWindow(QWindow *window) const
{
    QWindowSystemInterface::flushWindowSystemEvents();

    auto client = &m_clients.last();
    window->setScreen(client->platformScreen->screen());
    client->platformWindows.append(new QWebGLWindow(window));
    auto platformWindow = client->platformWindows.last();
    auto socket = client->socket;
    auto server = m_webSocketServer;
    platformWindow->create();
    platformWindow->requestActivateWindow();
    const auto winId = platformWindow->winId();

    const QVariantMap values {
        { "x", platformWindow->geometry().x() },
        { "y", platformWindow->geometry().y() },
        { "width", platformWindow->geometry().width() },
        { "height", platformWindow->geometry().height() },
        { "winId", winId },
        { "title", window->title() }
    };
    QMetaObject::invokeMethod(server, "sendTextMessage",
                              Q_ARG(QWebSocket *, socket),
                              Q_ARG(QWebGLWebSocketServer::TextMessageType,
                                    QWebGLWebSocketServer::TextMessageType::CreateCanvas),
                              Q_ARG(QVariantMap, values));

    QObject::connect(window, &QWindow::windowTitleChanged,
                     [server, socket, winId](const QString &title)
    {
        const QVariantMap values{{ "title", title }, { "winId", winId }};
        QMetaObject::invokeMethod(server, "sendTextMessage",
                                  Q_ARG(QWebSocket *, socket),
                                  Q_ARG(QWebGLWebSocketServer::TextMessageType,
                                        QWebGLWebSocketServer::TextMessageType::ChangeTitle),
                                  Q_ARG(QVariantMap, values));
    });

    return platformWindow;
}

QPlatformOpenGLContext *QWebGLIntegration::createPlatformOpenGLContext(QOpenGLContext *context)
    const
{
    QPlatformOpenGLContext *share = context->shareHandle();
    QVariant nativeHandle = context->nativeHandle();

    QSurfaceFormat adjustedFormat = context->format();
    QWebGLContext *ctx = new QWebGLContext(adjustedFormat, share);
    context->setNativeHandle(nativeHandle);
    return ctx;
}

QPlatformOffscreenSurface *QWebGLIntegration::createPlatformOffscreenSurface(
        QOffscreenSurface *surface) const
{
    QSurfaceFormat fmt = surface->requestedFormat();
    {
        return new QWebGLOffscreenWindow(fmt, surface);
    }
    // Never return null. Multiple QWindows are not supported by this plugin.
}

bool QWebGLIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    // We assume that devices will have more and not less capabilities
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    case WindowManagement: return false;
    case RasterGLSurface: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformNativeInterface *QWebGLIntegration::nativeInterface() const
{
    return const_cast<QWebGLIntegration *>(this);
}

QFunctionPointer QWebGLIntegration::platformFunction(const QByteArray &function) const
{
    if (function == QByteArrayLiteral("customRequestFunction"))
        return QFunctionPointer(QWebGLIntegration::httpServerCustomRequestFunction);
    else if (function == QByteArrayLiteral("setCustomRequestFunction"))
        return QFunctionPointer(QWebGLIntegration::httpServerSetCustomRequestFunction);
    else if (function == QByteArrayLiteral("customRequestDevice"))
        return QFunctionPointer(QWebGLIntegration::httpServerCustomRequestDevice);
    else if (function == QByteArrayLiteral("setCustomRequestDevice"))
        return QFunctionPointer(QWebGLIntegration::httpServerSetCustomRequestDevice);
    else if (function == QByteArrayLiteral("urls"))
        return QFunctionPointer(QWebGLIntegration::urls);
    qCritical("QWebGLIntegration::platformFunction: Platform function not found");
    return nullptr;
}

QWebGLHttpServer *QWebGLIntegration::httpServer()
{
    return m_httpServer;
}

QWebGLWebSocketServer *QWebGLIntegration::webSocketServer()
{
    return m_webSocketServer;
}

void QWebGLIntegration::openUrl(const QUrl &url)
{
    for (ClientData &clientData : m_clients) {
        const QVariantMap values {
            { "url", url }
        };
        QMetaObject::invokeMethod(m_webSocketServer,
                                  "sendTextMessage",
                                  Q_ARG(QWebSocket *, clientData.socket),
                                  Q_ARG(QWebGLWebSocketServer::TextMessageType,
                                        QWebGLWebSocketServer::TextMessageType::OpenUrl),
                                  Q_ARG(QVariantMap, values));
    }
}

void QWebGLIntegration::broadcastClipboard()
{
    QString subtype;
    const auto data = QGuiApplication::clipboard()->text(subtype);
    QVariantMap values {
        { "subtype", subtype },
        { "data", data }
    };
    std::for_each(m_clients.begin(), m_clients.end(), [=](ClientData &clientData)
    {
        QMetaObject::invokeMethod(m_webSocketServer, "sendTextMessage",
                                  Q_ARG(QWebSocket *, clientData.socket),
                                  Q_ARG(QWebGLWebSocketServer::TextMessageType,
                                        QWebGLWebSocketServer::TextMessageType::ClipboardUpdated),
                                  Q_ARG(QVariantMap, values));
    });
}

void QWebGLIntegration::onClientConnected(QWebSocket *socket, const int width, const int height,
                                          const double physicalWidth, const double physicalHeight)
{
    ClientData client;
    client.socket = socket;
    client.platformScreen = new QWebGLScreen(QSize(width, height),
                                             QSizeF(physicalWidth, physicalHeight));
    m_clients.append(client);
    screenAdded(client.platformScreen, true);
    auto event = new QWebGLIntegration::ClientConnectedEvent(width, height,
                                                             physicalWidth, physicalHeight);
    QCoreApplication::postEvent(qApp, event);
}

void QWebGLIntegration::onClientDisconnected(QWebSocket *socket)
{
    const auto predicate = [=](const ClientData &item)
    {
        return socket == item.socket;
    };

    auto it = std::find_if(m_clients.begin(), m_clients.end(), predicate);
    m_clients.erase(it);
}

QWebGLIntegration::ClientData *QWebGLIntegration::findClientData(const QWebSocket *socket)
{
    auto it = std::find_if(m_clients.begin(), m_clients.end(),
                           [=](const QWebGLIntegration::ClientData &data)
    {
        return data.socket == socket;
    });
    if (it != m_clients.end())
        return &*it;
    return nullptr;
}

QWebGLIntegration::ClientData *QWebGLIntegration::findClientData(const QPlatformSurface *surface)
{
    auto it = std::find_if(m_clients.begin(), m_clients.end(),
                           [=](const QWebGLIntegration::ClientData &data)
    {
        if (!data.platformWindows.isEmpty() && data.platformWindows.last()->surface())
            return surface == data.platformWindows.last()->surface()->surfaceHandle();
        return false;
    });
    if (it != m_clients.end())
        return &*it;
    return nullptr;
}

QWebGLHttpServerFunctions::CustomRequestFunction
QWebGLIntegration::httpServerCustomRequestFunction()
{
    auto iface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    return iface->httpServer()->customRequestFunction();
}

void QWebGLIntegration::httpServerSetCustomRequestFunction(
        QWebGLHttpServerFunctions::CustomRequestFunction function)
{
    auto iface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    return iface->httpServer()->setCustomRequestFunction(function);
}

QIODevice *QWebGLIntegration::httpServerCustomRequestDevice(const QString &name)
{
    auto iface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    return iface->httpServer()->customRequestDevice(name);
}

void QWebGLIntegration::httpServerSetCustomRequestDevice(const QString &name, QIODevice *device)
{
    auto iface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    auto server = iface->httpServer();
    return server->setCustomRequestDevice(name, device);
}

QList<QUrl> QWebGLIntegration::urls()
{
    auto iface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    return iface->httpServer()->urls();
}

QWebGLIntegration::ClientConnectedEvent::ClientConnectedEvent(int width,
                                                              int height,
                                                              double physicalWidth,
                                                              double physicalHeight) :
    QEvent(static_cast<QEvent::Type>(Type)),
    m_width(width),
    m_height(height),
    m_physicalWidth(physicalWidth),
    m_physicalHeight(physicalHeight)
{}

QWebGLIntegration::ClientDisconnectedEvent::ClientDisconnectedEvent() :
    QEvent(static_cast<QEvent::Type>(Type))
{}

QT_END_NAMESPACE
