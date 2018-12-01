#ifndef CGENGINE_UTILITY_QTMSGMUTEHANDLER_H
#define CGENGINE_UTILITY_QTMSGMUTEHANDLER_H

#include <qapplication.h>

void qtMsgMuteHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    Q_UNUSED(msg)

    switch (type)
    {
        case QtDebugMsg:
        case QtInfoMsg:
        case QtWarningMsg:
        case QtCriticalMsg:
            return;
        case QtFatalMsg:
            abort();
    }
}

#endif // CGENGINE_UTILITY_QTMSGMUTEHANDLER_H
