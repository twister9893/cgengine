#ifndef CGENGINE_CORE_CODEGENERATORPRIVATE_H
#define CGENGINE_CORE_CODEGENERATORPRIVATE_H

#include <QJSEngine>
#include <QFileInfo>

namespace cgengine { namespace core {

class CodeGeneratorPrivate {
public:
    QJSEngine jse;
    bool hasErrors = false;
    QString errorString;

public:
    QString process(const QString &tmpl);
    QString readFile(const QFileInfo &fileInfo);
};

}}

#endif // CGENGINE_CORE_CODEGENERATORPRIVATE_H
