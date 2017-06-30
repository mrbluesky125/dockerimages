#define WINDOW
#include <QDebug>
#include <QByteArray>
#include <QtPlatformHeaders/QWebGLHttpServerFunctions>
#ifdef WINDOW
#include <QQuickView>
#include <QApplication>
#include <QOpenGLWindow>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#else
#include <QTimer>
#include <QWidget>
#include <QApplication>
#include <QCalendarWidget>
#endif

#ifdef WINDOW
QQuickView *window = nullptr;
#else
QWidget *widget = nullptr;
#endif

void createWindow(bool fullscreen = false)
{
#ifdef WINDOW
    window = new QQuickView(QUrl("qrc:/main.qml"));
    window->setResizeMode(QQuickView::SizeRootObjectToView);
    window->setGeometry(0, 0, 640, 480);
    window->setColor(Qt::gray);
    window->setTitle("WebGL test");
    if (fullscreen)
        window->showFullScreen();
    else
        window->show();
#else
    widget = new QWidget;
    QTimer::singleShot(2000, [widget] { widget->update(); });
    new QCalendarWidget(widget);
    if (fullscreen)
        widget->showFullScreen();
    else
        widget->show();
#endif
}

class EventFilter : public QObject
{
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override
    {
        Q_UNUSED(watched);
        if (event->type() == QEvent::User + 100) {
            createWindow(true);
            return true;
        } else if (event->type() == QEvent::User + 101) {
            qDebug() << "Disconnected";
#ifdef WINDOW
            window->close();
            window->deleteLater();
#else
            widget->close();
            widget->deleteLater();
#endif
            return true;
        }

        return false;
    }
};

int main(int argc, char **argv)
{
#ifdef WINDOW
    QApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    EventFilter eventFilter;
    const auto platformName = app.platformName();
    if (platformName == "webgl") {
        app.installEventFilter(&eventFilter);
        QWebGLHttpServerFunctions::setCustomRequestFunction([](const QUrl &url, QByteArray *answer)
        {
            const auto data = url.toString().toUtf8();
            const auto dataSize = QString::number(data.size()).toUtf8();
            *answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n") +
                    QByteArrayLiteral("Content-Type: text/html\r\n") +
                    QByteArrayLiteral("Content-Length: ") + dataSize +
                    QByteArrayLiteral("\r\n\r\n") +
                    data;

            qDebug() << url << *answer;
        });
        qDebug() << "WebGL Streaming server listening on:" << QWebGLHttpServerFunctions::urls();
    } else {
        createWindow();
    }

    return app.exec();
}
