#include <QDir>
#include <QDebug>
#include <QByteArray>
#include <QtPlatformHeaders/QWebGLHttpServerFunctions>
#include <QQuickView>
#include <QApplication>
#include <QOpenGLWindow>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QOpenGLFunctions>

QQuickView *window = nullptr;

void createWindow(bool fullscreen = false)
{
    window = new QQuickView(QUrl("qrc:/main.qml"));
    window->setResizeMode(QQuickView::SizeRootObjectToView);
    window->setGeometry(0, 0, 640, 480);
    window->setColor(Qt::gray);
    window->setTitle("WebGL test");
    if (fullscreen)
        window->showFullScreen();
    else
        window->show();
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
            window->close();
            window->deleteLater();
            return true;
        }

        return false;
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    EventFilter eventFilter;
    const auto platformName = app.platformName();
    if (platformName == "webgl") {
        app.installEventFilter(&eventFilter);
        QWebGLHttpServerFunctions::setCustomRequestFunction([](const QUrl &url, QByteArray *answer)
        {
            QByteArray data;
            if (url.path() == "/download.csv") {
                const auto filePath = QDir::temp().filePath("download.csv");
                if (QFile::exists(filePath)) {
                    QFile file(filePath);
                    if (file.open(QIODevice::ReadOnly)) {
                        data = file.readAll();
                        const auto dataSize = QString::number(data.size()).toUtf8();
                        *answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n") +
                                QByteArrayLiteral("Content-Disposition: attachment; "
                                                  "filename=\"download.csv\"\r\n") +
                                QByteArrayLiteral("Content-Length: ") + dataSize +
                                QByteArrayLiteral("\r\n\r\n") +
                                data;
                    }
                }
            } else {
                data = url.toString().toUtf8();
            }
            const auto dataSize = QString::number(data.size()).toUtf8();
            *answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n") +
                    QByteArrayLiteral("Content-Type: text/html\r\n") +
                    QByteArrayLiteral("Content-Length: ") + dataSize +
                    QByteArrayLiteral("\r\n\r\n") +
                    data;

//            qDebug() << url << *answer;
        });
        qDebug() << "WebGL Streaming server listening on:" << QWebGLHttpServerFunctions::urls();
    } else {
        createWindow();
    }

    return app.exec();
}
