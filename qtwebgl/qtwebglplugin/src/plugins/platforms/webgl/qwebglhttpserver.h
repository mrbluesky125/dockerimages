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

#ifndef QWEBGLHTTPSERVER_H
#define QWEBGLHTTPSERVER_H

#include <QtPlatformHeaders/qwebglhttpserverfunctions.h>

#include <QtCore/qobject.h>

#include <QtCore/qurl.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>

#include <QtNetwork/qhostaddress.h>


QT_BEGIN_NAMESPACE

class QUrl;
class QTcpSocket;
class QByteArray;
class QTcpServer;
class QWebGLWebSocketServer;

class QWebGLHttpServer : public QObject
{
    Q_OBJECT

public:
    using CustomRequestFunction = QWebGLHttpServerFunctions::CustomRequestFunction;

    explicit QWebGLHttpServer(QWebGLWebSocketServer *m_webSocketServer, QObject *parent = nullptr);

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    bool isListening() const;
    quint16 serverPort() const;

    QList<QUrl> urls() const;

    CustomRequestFunction customRequestFunction() const;
    void setCustomRequestFunction(CustomRequestFunction function);

    QIODevice *customRequestDevice(const QString &name);
    void setCustomRequestDevice(const QString &name, QIODevice *device);

private slots:
    void clientConnected();
    void clientDisconnected();
    void readData();
    void answerClient(QTcpSocket *socket, const QUrl &urls);

private:
    struct HttpRequest {
        quint16 port = 0;

        bool readMethod(QTcpSocket *socket);
        bool readUrl(QTcpSocket *socket);
        bool readStatus(QTcpSocket *socket);
        bool readHeader(QTcpSocket *socket);

        enum class State {
            ReadingMethod,
            ReadingUrl,
            ReadingStatus,
            ReadingHeader,
            ReadingBody,
            AllDone
        } state = State::ReadingMethod;
        QByteArray fragment;

        enum class Method {
            Unknown,
            Head,
            Get,
            Put,
            Post,
            Delete,
        } method = Method::Unknown;
        quint32 byteSize = 0;
        QUrl url;
        QPair<quint8, quint8> version;
        QMap<QByteArray, QByteArray> headers;
    };

    QMap<QTcpSocket *, HttpRequest> m_clients;
    QMap<QString, QPointer<QIODevice>> m_customRequestDevices;
    QTcpServer *m_server;
    QPointer<QWebGLWebSocketServer> m_webSocketServer;
    CustomRequestFunction m_customRequestFunction = nullptr;
};

QT_END_NAMESPACE

#endif // QWEBGLHTTPSERVER_H
