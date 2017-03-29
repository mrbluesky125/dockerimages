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

#include "qwebglfunctioncall.h"

#include <QtCore/qstring.h>
#include <QtCore/qthread.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonobject.h>

#include <QtGui/qpa/qplatformsurface.h>

QAtomicInt QWebGLFunctionCall::s_id(1);

QWebGLFunctionCall::QWebGLFunctionCall(const QString &functionName,
                                       QPlatformSurface *surface,
                                       bool wait) :
    QEvent(Type(QEvent::User + 1)),
    m_functionName(functionName),
    m_surface(surface),
    m_wait(wait),
    m_id(s_id.fetchAndAddOrdered(1)),
    m_thread(QThread::currentThread())
{}

int QWebGLFunctionCall::id() const
{
    return m_id;
}

QThread *QWebGLFunctionCall::thread() const
{
    return m_thread;
}

bool QWebGLFunctionCall::isBlocking() const
{
    return m_wait;
}

QPlatformSurface *QWebGLFunctionCall::surface() const
{
    return m_surface;
}

QString QWebGLFunctionCall::functionName() const
{
    return m_functionName;
}

void QWebGLFunctionCall::setFunctionName(const QString &name)
{
    m_functionName = name;
}

void QWebGLFunctionCall::addString(const QString &value)
{
    m_parameters.append(value);
}

void QWebGLFunctionCall::addInt(int value)
{
    m_parameters.append(value);
}

void QWebGLFunctionCall::addUInt(uint value)
{
    m_parameters.append(value);
}

void QWebGLFunctionCall::addFloat(float value)
{
    m_parameters.append(static_cast<double>(value));
}

void QWebGLFunctionCall::addData(const QByteArray &data)
{
    m_parameters.append(data);
}

void QWebGLFunctionCall::addNull()
{
    m_parameters.append(QVariant());
}

QVariantList QWebGLFunctionCall::parameters() const
{
    return m_parameters;
}

