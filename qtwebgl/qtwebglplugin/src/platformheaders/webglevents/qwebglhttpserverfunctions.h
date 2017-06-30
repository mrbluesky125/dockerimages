#ifndef QWEBGLFUNCTIONS_H
#define QWEBGLFUNCTIONS_H

#include <QtCore/qglobal.h>
#include <QtGui/qguiapplication.h>

QT_BEGIN_NAMESPACE

class QWebGLHttpServerFunctions
{
public:
    typedef void (*CustomRequestFunction)(const QUrl &, QByteArray *);

    static CustomRequestFunction customRequestFunction()
    {
        typedef CustomRequestFunction (*Declaration)();
        static auto func = reinterpret_cast<Declaration>(
                    QGuiApplication::platformFunction("customRequestFunction"));
        Q_ASSERT(func);
        return func();
    }

    static void setCustomRequestFunction(CustomRequestFunction functionPointer)
    {
        typedef void (*Declaration)(CustomRequestFunction);
        static auto func = reinterpret_cast<Declaration>(
                    QGuiApplication::platformFunction("setCustomRequestFunction"));
        Q_ASSERT(func);
        func(functionPointer);
    }
};

QT_END_NAMESPACE


#endif // QWEBGLFUNCTIONS_H
