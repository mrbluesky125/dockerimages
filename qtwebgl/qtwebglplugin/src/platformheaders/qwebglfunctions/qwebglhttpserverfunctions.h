#ifndef QWEBGLFUNCTIONS_H
#define QWEBGLFUNCTIONS_H

#include <QtCore/qurl.h>
#include <QtCore/qlist.h>
#include <QtCore/qglobal.h>
#include <QtGui/qguiapplication.h>

QT_BEGIN_NAMESPACE

class QString;
class QIODevice;

class QWebGLHttpServerFunctions
{
public:
    typedef void (*CustomRequestFunction)(const QUrl &, QByteArray *);
    typedef void (*CustomRequestDevice)(const QString &, QIODevice *);

    static CustomRequestFunction customRequestFunction()
    {
        typedef CustomRequestFunction (*F)();
        static auto f = reinterpret_cast<F>(qGuiApp->platformFunction("customRequestFunction"));
        Q_ASSERT(f);
        return f();
    }

    static void setCustomRequestFunction(CustomRequestFunction functionPointer)
    {
        typedef void (*F)(CustomRequestFunction);
        static auto f = reinterpret_cast<F>(qGuiApp->platformFunction("setCustomRequestFunction"));
        Q_ASSERT(f);
        f(functionPointer);
    }

    static QIODevice *customRequestDevice(const QString &name)
    {
        typedef QIODevice *(*F)(const QString &);
        static auto func = reinterpret_cast<F>(qGuiApp->platformFunction("customRequestDevice"));
        Q_ASSERT(func);
        return func(name);
    }

    static void setCustomRequestDevice(const QString &name, QIODevice *device)
    {
        typedef void (*F)(const QString &, QIODevice *);
        static auto func = (F)qGuiApp->platformFunction("setCustomRequestDevice");
        Q_ASSERT(func);
        func(name, device);
    }

    static QList<QUrl> urls()
    {
        typedef QList<QUrl> (*F)();
        static auto f = reinterpret_cast<F>(QGuiApplication::platformFunction("urls"));
        Q_ASSERT(f);
        return f();
    }
};

QT_END_NAMESPACE


#endif // QWEBGLFUNCTIONS_H
