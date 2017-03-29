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

#ifndef QWEBGLFUNCTIONCALL_H
#define QWEBGLFUNCTIONCALL_H

#include <QtCore/qcoreevent.h>

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qatomic.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QThread;
class QPlatformSurface;

class QWebGLFunctionCall : public QEvent
{
public:
    enum {
        WebGLFunctionCall = QEvent::User + 1
    };
    QWebGLFunctionCall(const QString &functionName, QPlatformSurface *surface, bool wait = false);

    int id() const;
    QThread *thread() const;
    bool isBlocking() const;
    QPlatformSurface *surface() const;

    QString functionName() const;
    void setFunctionName(const QString &name);

    void addString(const QString &value);
    void addInt(int value);
    void addUInt(uint value);
    void addFloat(float value);
    void addData(const QByteArray &data);
    void addNull();

    void startArray();
    void endArray();

    QVariantList parameters() const;

private:
    QString m_functionName;
    QPlatformSurface *m_surface;
    QVariantList m_parameters;
    bool m_wait;
    int m_id = -1;
    QThread *m_thread = nullptr;
    static QAtomicInt s_id;
};

QT_END_NAMESPACE

#endif // QWEBGLFUNCTIONCALL_H
