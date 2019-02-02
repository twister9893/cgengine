#ifndef CGENGINE_CORE_JSCONSOLE_H
#define CGENGINE_CORE_JSCONSOLE_H

#include <QObject>
#include <QDebug>

namespace cgengine { namespace core {

class JSConsole : public QObject {
    Q_OBJECT
public:
    explicit JSConsole(QObject *parent = nullptr) {}

public slots:
    void log(const QString &msg) const {
        qDebug() << msg;
    }
};

}}

#endif // CGENGINE_CORE_JSCONSOLE_H
