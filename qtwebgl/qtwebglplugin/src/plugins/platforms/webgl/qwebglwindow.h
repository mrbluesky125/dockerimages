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

#ifndef QWEBGLWINDOW_H
#define QWEBGLWINDOW_H

#include <qpa/qplatformwindow.h>
#include <private/qopenglcompositor_p.h>

#include "qwebglintegration.h"
#include "qwebglscreen.h"

#include <QtCore/qmap.h>

#include <QtGui/qsurfaceformat.h>

#include <future>

QT_BEGIN_NAMESPACE

class QRect;
class QPlatformTextureList;
class QOpenGLCompositorBackingStore;

class QWebGLWindow : public QPlatformWindow, public QOpenGLCompositorWindow
{
    friend class QWebGLContext;
    friend class QWebGLWebSocketServer;

public:
    QWebGLWindow(QWindow *w);
    ~QWebGLWindow();

    void create();
    void destroy();

    void setGeometry(const QRect &rect) override;
    QRect geometry() const override;
    void setVisible(bool visible) override;
    void requestActivateWindow() override;
    void raise() override;
    void lower() override;

    void propagateSizeHints() override { }
    void setMask(const QRegion &) override { }
    bool setKeyboardGrabEnabled(bool) override { return false; }
    bool setMouseGrabEnabled(bool) override { return false; }
    void setOpacity(qreal) override;
    WId winId() const override;

    QSurfaceFormat format() const override;

    QWebGLScreen *screen() const;

    bool hasNativeWindow() const { return m_flags.testFlag(HasNativeWindow); }

    virtual void invalidateSurface() override;
    virtual void resetSurface();

    QOpenGLCompositorBackingStore *backingStore() { return m_backingStore; }
    void setBackingStore(QOpenGLCompositorBackingStore *backingStore);
    bool isRaster() const;

    QWindow *sourceWindow() const override;
    const QPlatformTextureList *textures() const override;
    void endCompositing() override;

protected:
    QOpenGLCompositorBackingStore *m_backingStore;
    bool m_raster;

    QSurfaceFormat m_format;

    enum Flag {
        Created = 0x01,
        HasNativeWindow = 0x02,
        IsFullScreen = 0x04
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Flags m_flags;

private:
    std::promise<QMap<GLenum, QVariant>> m_defaultData;
    int m_id = -1;
    static QAtomicInt s_id;
};

QT_END_NAMESPACE

#endif // QWEBGLWINDOW_H
