/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QDir>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickView> //Not using QQmlApplicationEngine because many examples don't have a Window{}
#define DECLARATIVE_EXAMPLE_MAIN(NAME) QGuiApplication *app; \
QQuickView *view; \
int main(int argc, char* argv[]) \
{ \
    app = new QGuiApplication(argc,argv); \
    app->setOrganizationName("QtProject"); \
    app->setOrganizationDomain("qt-project.org"); \
    app->setApplicationName(QFileInfo(app->applicationFilePath()).baseName()); \
    class EventFilter : public QObject \
    { \
    public: \
        virtual bool eventFilter(QObject *, QEvent *event) override \
        { \
            if (event->type() == QEvent::User + 100) { \
                view = new QQuickView; \
                if (qgetenv("QT_QUICK_CORE_PROFILE").toInt()) { \
                    QSurfaceFormat f = view->format(); \
                    f.setProfile(QSurfaceFormat::CoreProfile); \
                    f.setVersion(4, 4); \
                    view->setFormat(f); \
                } \
                view->connect(view->engine(), &QQmlEngine::quit, app, &QCoreApplication::quit); \
                new QQmlFileSelector(view->engine(), view); \
                view->setSource(QUrl("qrc:///" #NAME ".qml")); \
                if (view->status() == QQuickView::Error) \
                    return -1; \
                view->setResizeMode(QQuickView::SizeRootObjectToView); \
                if (QGuiApplication::platformName() == QLatin1String("qnx") || \
                      QGuiApplication::platformName() == QLatin1String("eglfs") || \
                      QGuiApplication::platformName() == QLatin1String("webgl")) { \
                    view->showFullScreen(); \
                } else { \
                    view->show(); \
                } \
                return true; \
            } else if (event->type() == QEvent::User + 101) { \
                view->close(); \
                view->deleteLater(); \
                return true; \
            } \
            return false; \
        } \
    }; \
    app->installEventFilter(new EventFilter); \
    auto ret = app->exec(); \
    return ret; \
}