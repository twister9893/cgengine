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
    QJSValue exec(const QString &js);
    QString process(const QString &tmpl, bool *err, bool origin = false);
    QString generate(const QString &tmpl, const QString &path);

    QString readFile(const QFileInfo &fileInfo);
    bool saveFile(const QString &string, const QString &filePath);

private:
    QString error(const QString &pattern, const QString &message, int line);
};

}}

#endif // CGENGINE_CORE_CODEGENERATORPRIVATE_H
