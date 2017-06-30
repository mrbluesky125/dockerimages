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

#ifndef QWEBGLINTEGRATION_H
#define QWEBGLINTEGRATION_H

#include <QtPlatformHeaders/qwebglhttpserverfunctions.h>

#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <QtCore/QVariant>
#include <QtCore/qcoreevent.h>
#include <qpa/qplatformscreen.h>

QT_BEGIN_NAMESPACE

class QThread;
class QWebSocket;
class QWebGLWindow;
class QWebGLScreen;
class QPlatformSurface;
class QPlatformContext;
class QWebGLHttpServer;
class QWebGLWebSocketServer;
class QWebGLPlatformServices;
class QWebGLPlatformClipboard;

class QWebGLIntegration : public QPlatformIntegration, public QPlatformNativeInterface
{
public:
    class ClientConnectedEvent : public QEvent
    {
    public:
        enum { Type = QEvent::User + 100 };

        ClientConnectedEvent(int width, int height, double physicalWidth, double physicalHeight);

    private:
        int m_width;
        int m_height;
        double m_physicalWidth;
        double m_physicalHeight;
    };

    class ClientDisconnectedEvent : public QEvent
    {
    public:
        enum { Type = QEvent::User + 101 };

        ClientDisconnectedEvent();
    };

    QWebGLIntegration(quint16 port);

    void initialize() override;
    void destroy() override;

    QAbstractEventDispatcher *createEventDispatcher() const override;
    QPlatformFontDatabase *fontDatabase() const override;
    QPlatformServices *services() const override;
    QPlatformInputContext *inputContext() const override { return m_inputContext; }
    QPlatformTheme *createPlatformTheme(const QString &name) const override;
    QPlatformClipboard *clipboard() const override;

    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
    QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const
        override;

    bool hasCapability(QPlatformIntegration::Capability cap) const override;

    QPlatformNativeInterface *nativeInterface() const override;

    QFunctionPointer platformFunction(const QByteArray &function) const override;

    QWebGLHttpServer *httpServer();
    QWebGLWebSocketServer *webSocketServer();

    void openUrl(const QUrl &url);
    void broadcastClipboard();

private slots:
    void onClientConnected(QWebSocket *socket,
                           const int width,
                           const int height,
                           const double physicalWidth,
                           const double physicalHeight);
    void onClientDisconnected(QWebSocket *socket);

private:
    friend class QWebGLWindow;
    friend class QWebGLWebSocketServer;

    struct ClientData
    {
        QWebSocket *socket;
        QWebGLScreen *platformScreen;
        QVector<QWebGLWindow *> platformWindows;
    };

    ClientData *findClientData(const QWebSocket *socket);
    ClientData *findClientData(const QPlatformSurface *surface);

    static QWebGLHttpServerFunctions::CustomRequestFunction httpServerCustomRequestFunction();
    static void httpServerSetCustomRequestFunction(
            QWebGLHttpServerFunctions::CustomRequestFunction function);

    static QIODevice *httpServerCustomRequestDevice(const QString &name);
    static void httpServerSetCustomRequestDevice(const QString &name, QIODevice *device);

    static QList<QUrl> urls();

    QPlatformInputContext *m_inputContext;
    quint16 m_httpPort;
    QScopedPointer<QPlatformFontDatabase> m_fontDatabase;
    QScopedPointer<QWebGLPlatformServices> m_services;
    QScopedPointer<QWebGLPlatformClipboard> m_clipboard;
    QWebGLHttpServer *m_httpServer = nullptr;
    QWebGLWebSocketServer *m_webSocketServer = nullptr;
    QWebGLScreen *m_screen = nullptr;
    QThread *m_webSocketServerThread;
    mutable QList<ClientData> m_clients;
};

QT_END_NAMESPACE

#endif // QWEBGLINTEGRATION_H
