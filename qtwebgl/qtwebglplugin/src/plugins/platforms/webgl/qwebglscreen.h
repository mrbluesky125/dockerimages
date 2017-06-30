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

#ifndef QWEBGLSCREEN_H
#define QWEBGLSCREEN_H

#include <QtCore/QPointer>

#include <QtCore/qsize.h>

#include <QtGui/qimage.h>

#include <qpa/qplatformscreen.h>

QT_BEGIN_NAMESPACE

class QPixmap;
class QWindow;
class QWebGLWindow;
class QOpenGLContext;

class QWebGLScreen : public QPlatformScreen
{
public:
    QWebGLScreen();
    QWebGLScreen(const QSize size, const QSizeF physicalSize);
    ~QWebGLScreen();

    QRect geometry() const override;
    int depth() const override;
    QImage::Format format() const override;

    QSizeF physicalSize() const override;
    QDpi logicalDpi() const override;
    qreal pixelDensity() const override;
    Qt::ScreenOrientation nativeOrientation() const override;
    Qt::ScreenOrientation orientation() const override;

    qreal refreshRate() const override;

    QPixmap grabWindow(WId wid, int x, int y, int width, int height) const override;

    void setGeometry(int width, int height, const int physicalWidth, const int physicalHeight);

private:
    QPointer<QWindow> m_pointerWindow;
    QSize m_size;
    QSizeF m_physicalSize;

    friend class QWebGLWindow;
};

QT_END_NAMESPACE

#endif // QWEBGLSCREEN_H
