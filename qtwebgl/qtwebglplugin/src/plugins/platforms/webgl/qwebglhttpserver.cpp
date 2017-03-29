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

#include "qwebglhttpserver.h"

#include "qwebglwebsocketserver.h"

#include <QtCore/qfile.h>
#include <QtCore/qtimer.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qbytearray.h>

#include <QtGui/qicon.h>
#include <QtGui/qclipboard.h>

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qnetworkinterface.h>

#include <cctype>
#include <cstdlib>

QWebGLHttpServer::QWebGLHttpServer(QWebGLWebSocketServer *webSocketServer, QObject *parent) :
    QObject(parent),
    m_server(new QTcpServer(this)),
    m_webSocketServer(webSocketServer)
{
    connect(m_server, &QTcpServer::newConnection, this, &QWebGLHttpServer::clientConnected);
}

bool QWebGLHttpServer::listen(const QHostAddress &address, quint16 port)
{
    return m_server->listen(address, port);
}

bool QWebGLHttpServer::isListening() const
{
    return m_server->isListening();
}

quint16 QWebGLHttpServer::serverPort() const
{
    return m_server->serverPort();
}

QList<QUrl> QWebGLHttpServer::urls() const
{
    const auto serverAddress = m_server->serverAddress();
    if (serverAddress == QHostAddress::LocalHost)
        return { QUrl(QString("http://127.0.0.1:%1").arg(serverPort())) };
    if (serverAddress == QHostAddress::LocalHostIPv6)
        return { QUrl(QString("http://::1:%1").arg(serverPort())) };
    if (serverAddress == QHostAddress::AnyIPv4) {
        QSet<QUrl> urls;
        const auto allAddresses = QNetworkInterface::allAddresses();
        for (const auto &address : allAddresses) {
            if (address.isNull())
                continue;
            urls.insert(QUrl(QString("http://%1:%2")
                             .arg(QHostAddress(address.toIPv4Address()).toString())
                             .arg(serverPort())));
        }
        return urls.toList();
    }
    if (serverAddress == QHostAddress::AnyIPv6) {
        QSet<QUrl> urls;
        const auto allAddresses = QNetworkInterface::allAddresses();
        for (const auto &address : allAddresses) {
            if (address.isNull())
                continue;
            urls.insert(QUrl(QString("http://%1:%2")
                             .arg(QHostAddress(address.toIPv6Address()).toString())
                             .arg(serverPort())));
        }
        return urls.toList();
    }
    if (serverAddress == QHostAddress::Any) {
        QSet<QUrl> urls;
        const auto allAddresses = QNetworkInterface::allAddresses();
        for (const auto &address : allAddresses) {
            if (address.isNull())
                continue;
            urls.insert(QUrl(QString("http://%1:%2")
                             .arg(QHostAddress(address.toIPv4Address()).toString())
                             .arg(serverPort())));
            urls.insert(QUrl(QString("http://%1:%2")
                             .arg(QHostAddress(address.toIPv6Address()).toString())
                             .arg(serverPort())));
        }
        return urls.toList();
    }
    return { QUrl() };
}

QWebGLHttpServer::CustomRequestFunction QWebGLHttpServer::customRequestFunction() const
{
    return m_customRequestFunction;
}

void QWebGLHttpServer::setCustomRequestFunction(QWebGLHttpServer::CustomRequestFunction function)
{
    m_customRequestFunction = function;
}

QIODevice *QWebGLHttpServer::customRequestDevice(const QString &name)
{
    return m_customRequestDevices.value(name, nullptr).data();
}

void QWebGLHttpServer::setCustomRequestDevice(const QString &name, QIODevice *device)
{
    if (m_customRequestDevices.value(name))
        m_customRequestDevices[name]->deleteLater();
    m_customRequestDevices.insert(name, device);
}

void QWebGLHttpServer::clientConnected()
{
    auto socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, this, &QWebGLHttpServer::clientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &QWebGLHttpServer::readData);
}

void QWebGLHttpServer::clientDisconnected()
{
    auto socket = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(socket);
    m_clients.remove(socket);
    socket->deleteLater();
}

void QWebGLHttpServer::readData()
{
    auto socket = qobject_cast<QTcpSocket *>(sender());
    if (!m_clients.contains(socket))
        m_clients[socket].port = m_server->serverPort();

    auto request = &m_clients[socket];
    bool error = false;

    request->byteSize += socket->bytesAvailable();
    if (Q_UNLIKELY(request->byteSize > 2048)) {
        socket->write(QByteArrayLiteral("HTTP 413 – Request entity too large\r\n"));
        socket->disconnectFromHost();
        m_clients.remove(socket);
        return;
    }

    if (Q_LIKELY(request->state == HttpRequest::State::ReadingMethod))
        if (Q_UNLIKELY(error = !request->readMethod(socket)))
            qWarning("QWebGLHttpServer::readData: Invalid Method");

    if (Q_LIKELY(!error && request->state == HttpRequest::State::ReadingUrl))
        if (Q_UNLIKELY(error = !request->readUrl(socket)))
            qWarning("QWebGLHttpServer::readData: Invalid URL");

    if (Q_LIKELY(!error && request->state == HttpRequest::State::ReadingStatus))
        if (Q_UNLIKELY(error = !request->readStatus(socket)))
            qWarning("QWebGLHttpServer::readData: Invalid Status");

    if (Q_LIKELY(!error && request->state == HttpRequest::State::ReadingHeader))
        if (Q_UNLIKELY(error = !request->readHeader(socket)))
            qWarning("QWebGLHttpServer::readData: Invalid Header");

    if (error) {
        socket->disconnectFromHost();
        m_clients.remove(socket);
    } else if (!request->url.isEmpty()) {
        Q_ASSERT(request->state != HttpRequest::State::ReadingUrl);
        answerClient(socket, request->url);
        m_clients.remove(socket);
    }
}

void QWebGLHttpServer::answerClient(QTcpSocket *socket, const QUrl &url)
{
    bool disconnect = true;
    const auto path = url.path();

    qDebug("QWebGLHttpServer::answerClient: %s requested: %s",
           qPrintable(socket->localAddress().toString()), qPrintable(path));

    QByteArray answer = QByteArrayLiteral("HTTP/1.1 404 Not Found\r\n"
                                          "Content-Type: text/html\r\n"
                                          "Content-Length: 136\r\n\r\n"
                                          "<html>"
                                          "<head><title>404 Not Found</title></head>"
                                          "<body bgcolor=\"white\">"
                                          "<center><h1>404 Not Found</h1></center>"
                                          "</body>"
                                          "</html>");
    const auto addData = [&answer](const QByteArray &contentType, const QByteArray &data)
    {
        answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n");
        QByteArray ret;
        const auto dataSize = QString::number(data.size()).toUtf8();
        answer += QByteArrayLiteral("Content-Type: ") + contentType + QByteArrayLiteral("\r\n") +
                  QByteArrayLiteral("Content-Length: ") + dataSize + QByteArrayLiteral("\r\n\r\n") +
                  data;
    };

    if (path == "/") {
        QFile file(QStringLiteral(":/webgl/index.html"));
        Q_ASSERT(file.exists());
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_ASSERT(file.isOpen());
        auto data = file.readAll();
        addData(QByteArrayLiteral("text/html; charset=\"utf-8\""), data);
    } else if (path == "/clipboard") {
        auto data = qGuiApp->clipboard()->text().toUtf8();
        addData(QByteArrayLiteral("text/html; charset=\"utf-8\""), data);
    } else if (path == "/webqt.js") {
        QFile file(QStringLiteral(":/webgl/webqt.js"));
        Q_ASSERT(file.exists());
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_ASSERT(file.isOpen());
        const auto host = url.host().toUtf8();
        const auto port = QString::number(m_webSocketServer->port()).toUtf8();
        QByteArray data = "var host = \"" + host + "\";\r\nvar port = " + port + ";\r\n";
        data += file.readAll();
        addData(QByteArrayLiteral("application/javascript"), data);
    } else if (path == "/favicon.ico") {
        QFile file(QStringLiteral(":/webgl/favicon.ico"));
        Q_ASSERT(file.exists());
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_ASSERT(file.isOpen());
        auto data = file.readAll();
        addData(QByteArrayLiteral("image/x-icon"), data);
    } else if (path == "/favicon.png") {
        QBuffer buffer;
        qGuiApp->windowIcon().pixmap(16, 16).save(&buffer, "png");
        addData(QByteArrayLiteral("image/x-icon"), buffer.data());
    } else if (auto device = m_customRequestDevices.value(path)) {
        answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n"
                                   "Content-Type: text/plain; charset=\"utf-8\"\r\n"
                                   "Connection: Keep.Alive\r\n\r\n") +
                device->readAll();
        auto timer = new QTimer(device);
        timer->setSingleShot(false);
        connect(timer, &QTimer::timeout, [device, socket]()
        {
            if (device->bytesAvailable())
                socket->write(device->readAll());
        });
        timer->start(1000);
        disconnect = false;
    } else if(m_customRequestFunction) {
        QByteArray data;
        m_customRequestFunction(url, &data);
        if (!data.isEmpty())
            answer.swap(data);
    }
    socket->write(answer);
    if (disconnect)
        socket->disconnectFromHost();
}

bool QWebGLHttpServer::HttpRequest::readMethod(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isupper(c) && fragment.size() < 6)
            fragment += c;
        else
            finished = true;
    }
    if (finished) {
        if (fragment == "HEAD")
            method = Method::Head;
        else if (fragment == "GET")
            method = Method::Get;
        else if (fragment == "PUT")
            method = Method::Put;
        else if (fragment == "POST")
            method = Method::Post;
        else if (fragment == "DELETE")
            method = Method::Delete;
        else
            qWarning("QWebGLHttpServer::HttpRequest::readMethod: Invalid operation %s",
                     fragment.data());

        state = State::ReadingUrl;
        fragment.clear();

        return method != Method::Unknown;
    }
    return true;
}

bool QWebGLHttpServer::HttpRequest::readUrl(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isspace(c))
            finished = true;
        else
            fragment += c;
    }
    if (finished) {
        if (!fragment.startsWith("/")) {
            qWarning("QWebGLHttpServer::HttpRequest::readUrl: Invalid URL path %s",
                     fragment.constData());
            return false;
        }
        url.setUrl(QStringLiteral("http://localhost:") + QString::number(port) +
                   QString::fromUtf8(fragment));
        state = State::ReadingStatus;
        if (!url.isValid()) {
            qWarning("QWebGLHttpServer::HttpRequest::readUrl: Invalid URL %s",
                     fragment.constData());
            return false;
        }
        fragment.clear();
        return true;
    }
    return true;
}

bool QWebGLHttpServer::HttpRequest::readStatus(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            finished = true;
            fragment.resize(fragment.size() - 2);
        }
    }
    if (finished) {
        if (!std::isdigit(fragment.at(fragment.size() - 3)) ||
                !std::isdigit(fragment.at(fragment.size() - 1))) {
            qWarning("QWebGLHttpServer::HttpRequest::::readStatus: Invalid version");
            return false;
        }
        version = qMakePair(fragment.at(fragment.size() - 3) - '0',
                            fragment.at(fragment.size() - 1) - '0');
        state = State::ReadingHeader;
        fragment.clear();
    }
    return true;
}

bool QWebGLHttpServer::HttpRequest::readHeader(QTcpSocket *socket)
{
    while (socket->bytesAvailable()) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            if (fragment == "\r\n") {
                state = State::ReadingBody;
                fragment.clear();
                return true;
            } else {
                fragment.chop(2);
                const int index = fragment.indexOf(':');
                if (index == -1)
                    return false;

                const QByteArray key = fragment.mid(0, index).trimmed();
                const QByteArray value = fragment.mid(index + 1).trimmed();
                headers.insert(key, value);
                if (QStringLiteral("host").compare(key, Qt::CaseInsensitive) == 0) {
                    auto parts = value.split(':');
                    if (parts.size() == 1) {
                        url.setHost(parts.first());
                        url.setPort(80);
                    } else {
                        url.setHost(parts.first());
                        url.setPort(std::strtoul(parts.at(1).constData(), nullptr, 10));
                    }
                }
                fragment.clear();
            }
        }
    }
    return false;
}
