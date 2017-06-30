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

#ifndef QWEBGLWEBSOCKETSERVER_H
#define QWEBGLWEBSOCKETSERVER_H

#include <QtCore/qobject.h>

#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qvector.h>
#include <QtCore/qvariant.h>
#include <QtCore/qwaitcondition.h>

#include <QtGui/qtouchdevice.h>

QT_BEGIN_NAMESPACE

class QWebSocket;
class QByteArray;
class QWebSocketServer;
class QWebGLIntegration;

class QWebGLWebSocketServer : public QObject
{
    Q_OBJECT

public:
    enum class TextMessageType
    {
        Connect,
        GlCommand,
        CreateCanvas,
        DestroyCanvas,
        ClipboardUpdated,
        OpenUrl,
        ChangeTitle
    };

    QWebGLWebSocketServer(QObject *parent = nullptr);
    ~QWebGLWebSocketServer();

    quint16 port() const;

    QMutex *mutex() { return &m_mutex; }
    QWaitCondition *waitCondition() { return &m_condition; }

    QVariant queryValue(int id);

public slots:
    void create();
    void sendTextMessage(QWebSocket *socket,
                         QWebGLWebSocketServer::TextMessageType type,
                         const QVariantMap &values);

signals:
    void clientConnected(QWebSocket *socket,
                         const int width,
                         const int height,
                         const double physicalWidth,
                         const double physicalHeight);
    void clientDisconnected(QWebSocket *socket);
    void canvasResized(QWebSocket *socket,
                       const int width,
                       const int height,
                       const double physicalWidth,
                       const double physicalHeight);

protected:
    bool event(QEvent *event) override;

private slots:
    void onNewConnection();
    void onDisconnect();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void send(quint32 threadId, const QByteArray &data);

private:
    static QWebGLIntegration *platformItengration();

    QMutex m_mutex;
    QWaitCondition m_condition;
    QWebSocketServer *m_server;
    QVector<int> m_pendingResponses;
    QHash<int, QVariant> m_receivedResponses;
    QTouchDevice m_touchDevice;
};

QT_END_NAMESPACE

#endif // QWEBGLWEBSOCKETSERVER_H
