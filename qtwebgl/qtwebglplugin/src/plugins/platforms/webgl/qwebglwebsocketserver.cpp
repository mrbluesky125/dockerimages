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

#include "qwebglwebsocketserver.h"

#include "qwebglwindow.h"
#include "qwebglintegration.h"
#include "qwebglfunctioncall.h"

#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>

#include <QtCore/qdebug.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>

#include <QtGui/qevent.h>
#include <QtGui/qguiapplication.h>

#include <qpa/qwindowsysteminterface.h>

#include <private/qobject_p.h>

#include <cstring>

QT_BEGIN_NAMESPACE

const QHash<QString, Qt::Key> keyMap {
    { "Alt", Qt::Key_Alt },
    { "ArrowDown", Qt::Key_Down },
    { "ArrowLeft", Qt::Key_Left },
    { "ArrowRight", Qt::Key_Right },
    { "ArrowUp", Qt::Key_Up },
    { "Backspace", Qt::Key_Backspace },
    { "Control", Qt::Key_Control },
    { "Delete", Qt::Key_Delete },
    { "End", Qt::Key_End },
    { "Enter", Qt::Key_Enter },
    { "F1", Qt::Key_F1 },
    { "F2", Qt::Key_F2 },
    { "F3", Qt::Key_F3 },
    { "F4", Qt::Key_F4 },
    { "F5", Qt::Key_F5 },
    { "F6", Qt::Key_F6 },
    { "F7", Qt::Key_F7 },
    { "F8", Qt::Key_F8 },
    { "F9", Qt::Key_F9 },
    { "F10", Qt::Key_F10 },
    { "F11", Qt::Key_F11 },
    { "F12", Qt::Key_F12 },
    { "Escape", Qt::Key_Escape },
    { "Home", Qt::Key_Home },
    { "Insert", Qt::Key_Insert },
    { "Meta", Qt::Key_Meta },
    { "PageDown", Qt::Key_PageDown },
    { "PageUp", Qt::Key_PageUp },
    { "Shift", Qt::Key_Shift },
    { "Space", Qt::Key_Space },
    { "AltGraph", Qt::Key_AltGr },
    { "Tab", Qt::Key_Tab },
    { "Unidentified", Qt::Key_F },
    { "OS", Qt::Key_Super_L }
};

inline QWebGLIntegration *webGLIntegration()
{
#ifdef QT_DEBUG
    auto nativeInterface = dynamic_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    Q_ASSERT(nativeInterface);
#else
    auto nativeInterface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
#endif // QT_DEBUG
    return nativeInterface;
}

QWebGLWebSocketServer::QWebGLWebSocketServer(QObject *parent) :
    QObject(parent)
{
    m_touchDevice.setName("EmulatedTouchDevice");
    m_touchDevice.setType(QTouchDevice::TouchScreen);
    m_touchDevice.setCapabilities(QTouchDevice::Position | QTouchDevice::Pressure |
                                  QTouchDevice::MouseEmulation);
    m_touchDevice.setMaximumTouchPoints(6);
    QWindowSystemInterface::registerTouchDevice(&m_touchDevice);
}

QWebGLWebSocketServer::~QWebGLWebSocketServer()
{
    QWindowSystemInterface::unregisterTouchDevice(&m_touchDevice);
}

quint16 QWebGLWebSocketServer::port() const
{
    return m_server->serverPort();
}

QVariant QWebGLWebSocketServer::queryValue(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_receivedResponses.contains(id))
        return m_receivedResponses.take(id);
    return QVariant();
}

void QWebGLWebSocketServer::create()
{
    m_server = new QWebSocketServer(QLatin1String("qtwebgl"), QWebSocketServer::NonSecureMode);
    bool ok = m_server->listen(QHostAddress::Any);
    if (ok) {
        connect(m_server, &QWebSocketServer::newConnection, this,
                &QWebGLWebSocketServer::onNewConnection);
    }

    QMutexLocker lock(&m_mutex);
    m_condition.wakeAll();
}

void QWebGLWebSocketServer::sendTextMessage(QWebSocket *socket,
                                            QWebGLWebSocketServer::TextMessageType type,
                                            const QVariantMap &values)
{
    QString typeString;
    switch (type) {
    case TextMessageType::Connect:
        typeString = "connect";
        break;
    case TextMessageType::GlCommand: {
        const auto functionName = values["function"].toString().toUtf8();
        const auto parameters = values["parameters"].toList();
        const quint32 parameterCount = parameters.size();
        const quint32 id = values["id"].toUInt();
        QByteArray data;
        {
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream << id << functionName << parameterCount;
            for (const auto &value : qAsConst(parameters)) {
                if (value.isNull()) {
                    stream << (quint8)'n';
                } else switch (value.type()) {
                case QVariant::Int:
                    stream << (quint8)'i' << value.toInt();
                    break;
                case QVariant::UInt:
                    stream << (quint8)'u' << value.toUInt();
                    break;
                case QVariant::Bool:
                    stream << (quint8)'b' << (quint8)value.toBool();
                    break;
                case QVariant::Double:
                    stream << (quint8)'d' << value.toDouble();
                    break;
                case QVariant::String:
                    stream << (quint8)'s' << value.toString().toUtf8();
                    break;
                case QVariant::ByteArray: {
                    const auto byteArray = value.toByteArray();
                    if (data.isNull())
                        stream << (quint8)'n';
                    else
                        stream << (quint8)'x' << byteArray;
                    break;
                }
                default:
                    qCritical("QWebGLWebSocketServer::sendTextMessage: Unsupported type");
                    break;
                }
            }
            stream << (quint32)0xbaadf00d;
        }
        const quint32 totalMessageSize = data.size();
        const quint32 maxMessageSize = 1024;
        for (quint32 i = 0; i <= data.size() / maxMessageSize; ++i) {
            const quint32 offset = i * maxMessageSize;
            const quint32 size = qMin(totalMessageSize - offset, maxMessageSize);
            const auto chunk = QByteArray::fromRawData(data.constData() + offset, size);
            socket->sendBinaryMessage(chunk);
        }
        return;
    }
    case TextMessageType::CreateCanvas:
        typeString = "create_canvas";
        break;
    case TextMessageType::DestroyCanvas:
        typeString = "destroy_canvas";
        break;
    case TextMessageType::ClipboardUpdated:
        typeString = "clipboard_updated";
        break;
    case TextMessageType::OpenUrl:
        typeString = "open_url";
        break;
    case TextMessageType::ChangeTitle:
        typeString = "changle_title";
        break;
    }
    QJsonDocument document;
    auto commandObject = QJsonObject::fromVariantMap(values);
    commandObject["type"] = typeString;
    document.setObject(commandObject);
    auto data = document.toJson(QJsonDocument::Compact);
    socket->sendTextMessage(data);
}

bool QWebGLWebSocketServer::event(QEvent *event)
{
    int type = event->type();
    if (type == QWebGLFunctionCall::WebGLFunctionCall) {
        auto e = static_cast<QWebGLFunctionCall *>(event);
        const QVariantMap values {
           { "function", e->functionName() },
           { "id", e->id() },
           { "parameters", e->parameters() }
        };
        auto clientData = platformItengration()->findClientData(e->surface());
        if (clientData) {
            sendTextMessage(clientData->socket, TextMessageType::GlCommand, values);
            if (e->isBlocking()) {
                m_pendingResponses.append(e->id());
            }
            return true;
        }
        return false;
    }
    return QObject::event(event);
}

void QWebGLWebSocketServer::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();
    if (socket) {
        connect(socket, &QWebSocket::disconnected, this, &QWebGLWebSocketServer::onDisconnect);
        connect(socket, &QWebSocket::textMessageReceived, this,
                &QWebGLWebSocketServer::onTextMessageReceived);
        connect(socket, &QWebSocket::binaryMessageReceived, this,
                &QWebGLWebSocketServer::onBinaryMessageReceived);

        const QVariantMap values{
            { "buildAbi", QSysInfo::buildAbi() },
            { "buildCpuArchitecture", QSysInfo::buildCpuArchitecture() },
            { "currentCpuArchitecture", QSysInfo::currentCpuArchitecture() },
            { "kernelType", QSysInfo::kernelType() },
            { "machineHostName", QSysInfo::machineHostName() },
            { "prettyProductName", QSysInfo::prettyProductName() },
            { "productType", QSysInfo::productType() },
            { "productVersion", QSysInfo::productVersion() }
        };

        sendTextMessage(socket, TextMessageType::Connect, values);
    }
}

void QWebGLWebSocketServer::onDisconnect()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    Q_ASSERT(socket);
    Q_EMIT clientDisconnected(socket);
    socket->deleteLater();
    QEvent *event = new QEvent(QEvent::Type(QEvent::User + 667));
    QCoreApplication::postEvent(qApp, event);
}

void QWebGLWebSocketServer::onTextMessageReceived(const QString &message)
{
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    Q_ASSERT(parseError.error == QJsonParseError::NoError);
    Q_ASSERT(document.isObject());
    auto object = document.object();
    Q_ASSERT(object.contains("type"));
    const auto type = object[QStringLiteral("type")];

    const auto socket = qobject_cast<QWebSocket *>(sender());
    const auto clientData = platformItengration()->findClientData(socket);
    if (type == QStringLiteral("connect")) {
        const auto width = object["width"].toInt();
        const auto height = object["height"].toInt();
        const auto physicalWidth = object["physicalWidth"].toDouble();
        const auto physicalHeight = object["physicalHeight"].toDouble();
        auto socket = qobject_cast<QWebSocket *>(sender());
        Q_ASSERT(socket);
        Q_EMIT clientConnected(socket, width, height, physicalWidth, physicalHeight);
    } else if (!clientData || clientData->platformWindows.isEmpty()) {
        return;
    }

    const auto keyboardModifiers = [object]()
    {
        Qt::KeyboardModifiers modifiers = Qt::NoModifier;
        if (object.value("ctrlKey").toBool())
            modifiers |= Qt::ControlModifier;
        if (object.value("shiftKey").toBool())
            modifiers |= Qt::ShiftModifier;
        if (object.value("altKey").toBool())
            modifiers |= Qt::AltModifier;
        if (object.value("metaKey").toBool())
            modifiers |= Qt::MetaModifier;
        return modifiers;
    };
    const auto findWindow = [&clientData](WId winId) {
        auto &windows = clientData->platformWindows;
        auto it = std::find_if(windows.begin(), windows.end(),
                               [winId](QWebGLWindow *window)
        {
            return window->winId() == winId;
        });
        Q_ASSERT(it != windows.end());
        return *it;
    };

    if (type == QStringLiteral("default_context_parameters")) {
        const auto winId = object.value("name").toInt(-1);
        Q_ASSERT(winId != -1);
        QWebGLWindow *platformWindow = findWindow(winId);
        Q_ASSERT(platformWindow);
        auto data = object.toVariantMap();
        data.remove("name");
        data.remove("type");
        QMap<GLenum, QVariant> result;
        for (auto it = data.cbegin(), end = data.cend(); it != end; ++it)
            if (it.key() != QStringLiteral("name") && it.key() != QStringLiteral("type"))
                result.insert(it.key().toInt(), *it);
        platformWindow->m_defaultData.set_value(result);
    } else if (type == QStringLiteral("gl_response")) {
        QMutexLocker locker(&m_mutex);
        const auto id = object["id"];
        const auto value = object["value"].toVariant();
        Q_ASSERT(m_pendingResponses.contains(id.toInt()));
        m_receivedResponses.insert(id.toInt(), value);
        m_pendingResponses.removeOne(id.toInt());
        m_condition.wakeAll(); // TODO: This will wake all the conditions
    } else if (type == QStringLiteral("mouse")) {
        const auto winId = object.value("name").toInt(-1);
        Q_ASSERT(winId != -1);
        QPointF localPos(object.value("localX").toDouble(),
                         object.value("localY").toDouble());
        QPointF globalPos(object.value("globalX").toDouble(),
                          object.value("globalY").toDouble());
        auto buttons = static_cast<Qt::MouseButtons>(object.value("buttons").toInt());
        auto time = object.value("time").toDouble();
        auto platformWindow = findWindow(winId);
        QWindowSystemInterface::handleMouseEvent(platformWindow->window(),
                                                 static_cast<ulong>(time),
                                                 localPos,
                                                 globalPos,
                                                 Qt::MouseButtons(buttons),
                                                 Qt::NoModifier,
                                                 Qt::MouseEventNotSynthesized);
    } else if (type == QStringLiteral("wheel")) {
        const auto winId = object.value("name").toInt(-1);
        Q_ASSERT(winId != -1);
        auto platformWindow = findWindow(winId);
        auto time = object.value("time").toDouble();
        QPointF localPos(object.value("localX").toDouble(),
                         object.value("localY").toDouble());
        QPointF globalPos(object.value("globalX").toDouble(),
                          object.value("globalY").toDouble());
        const int deltaX = -object.value("deltaX").toInt(0);
        const int deltaY = -object.value("deltaY").toInt(0);
        auto orientation = deltaY != 0 ? Qt::Vertical : Qt::Horizontal;
        QWindowSystemInterface::handleWheelEvent(platformWindow->window(),
                                                 time,
                                                 localPos,
                                                 globalPos,
                                                 orientation == Qt::Vertical ? deltaY
                                                                             : deltaX,
                                                 orientation);
    } else if (type == QStringLiteral("touch")) {
        const auto winId = object.value("name").toInt(-1);
        Q_ASSERT(winId != -1);
        auto window = findWindow(winId)->window();
        const auto time = object.value("time").toDouble();
        const auto eventType = object.value("event").toString();
        const auto changedTouch = object.value("changedTouches").toArray().first().toObject();
        const auto clientX = changedTouch.value("clientX").toDouble();
        const auto clientY = changedTouch.value("clientY").toDouble();
        QList<QWindowSystemInterface::TouchPoint> points;
        for (auto changedTouch : object.value("changedTouches").toArray()) {
            QWindowSystemInterface::TouchPoint point; // support more than one
            const auto pageX = changedTouch.toObject().value("pageX").toDouble();
            const auto pageY = changedTouch.toObject().value("pageY").toDouble();
            const auto radiousX = changedTouch.toObject().value("radiousX").toDouble();
            const auto radiousY = changedTouch.toObject().value("radiousY").toDouble();
            point.id = changedTouch.toObject().value("identifier").toInt(0);
            point.pressure = changedTouch.toObject().value("force").toDouble(1.);
            point.area.setX(pageX - radiousX);
            point.area.setY(pageY - radiousY);
            point.area.setWidth(radiousX * 2);
            point.area.setHeight(radiousY * 2);
            point.normalPosition.setX(changedTouch.toObject().value("normalPositionX").toDouble());
            point.normalPosition.setY(changedTouch.toObject().value("normalPositionY").toDouble());
            if (eventType == QStringLiteral("touchstart")) {
                point.state = Qt::TouchPointPressed;
            } else if (eventType == QStringLiteral("touchend")) {
                qDebug() << "end" << object;
                point.state = Qt::TouchPointReleased;
            } else if (eventType == QStringLiteral("touchcancel")) {
                QWindowSystemInterface::handleTouchCancelEvent(window,
                                                               time,
                                                               &m_touchDevice,
                                                               Qt::NoModifier);
                return;
            } else {
                point.state = Qt::TouchPointMoved;
            }
            point.rawPositions = {{ clientX, clientY }};
            points.append(point);
        }

        QWindowSystemInterface::handleTouchEvent(window,
                                                 time,
                                                 &m_touchDevice,
                                                 points,
                                                 Qt::NoModifier);
    } else if (type.toString().startsWith("key")) {
        const auto timestamp = static_cast<ulong>(object.value("time").toDouble(-1));
        const auto keyName = object.value("key").toString();
        const auto specialKey = keyMap.find(keyName);
        QEvent::Type eventType;
        if (type == QStringLiteral("keydown"))
            eventType = QEvent::KeyPress;
        else if (type == QStringLiteral("keyup"))
            eventType = QEvent::KeyRelease;
        else
            return;
        QString string(object.value("key").toString());
        int key = object.value("which").toInt(0);
        if (specialKey != keyMap.end()) {
            key = *specialKey;
            string.clear();
        }

        QWindowSystemInterface::handleKeyEvent(clientData->platformWindows.last()->window(),
                                               timestamp,
                                               eventType,
                                               key,
                                               keyboardModifiers(),
                                               string);
    } else if (type == QStringLiteral("canvas_resize")){
        const auto width = object["width"].toInt();
        const auto height = object["height"].toInt();
        const auto physicalWidth = object["physicalWidth"].toDouble();
        const auto physicalHeight = object["physicalHeight"].toDouble();
        Q_EMIT canvasResized(socket, width, height, physicalWidth, physicalHeight);
    }
}

void QWebGLWebSocketServer::onBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message);
}

void QWebGLWebSocketServer::send(quint32 threadId, const QByteArray &data)
{
    Q_UNUSED(threadId);
    Q_UNUSED(data);
}

QWebGLIntegration *QWebGLWebSocketServer::platformItengration()
{
#ifdef QT_DEBUG
        auto iface = dynamic_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
        Q_ASSERT(iface);
#else
        auto iface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
#endif // QT_DEBUG
        return iface;
}

QT_END_NAMESPACE
