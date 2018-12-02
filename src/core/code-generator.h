#ifndef CGENGINE_CORE_CODEGENERATOR_H
#define CGENGINE_CORE_CODEGENERATOR_H

#include <QScopedPointer>
#include <QString>

class QFileInfo;

namespace cgengine { namespace core {

class CodeGeneratorPrivate;
class CodeGenerator {
public:
    ~CodeGenerator();
    CodeGenerator();
    CodeGenerator(const QString &js);
    CodeGenerator(const QFileInfo &js);

public:
    QString exec(const QString &js);
    QString exec(const QFileInfo &js);

    QString process(const QString &tmpl);
    QString process(const QFileInfo &tmpl);

    QString generate(const QString &tmpl, const QString &path);
    QString generate(const QFileInfo &tmpl, const QString &path);

    QString value(const QString &name, const QString &defaultValue = QString());

public:
    bool hasErrors() const;
    QString errorString() const;

private:
    QScopedPointer<CodeGeneratorPrivate> d_ptr;
};

}}

#endif // CGENGINE_CORE_CODEGENERATOR_H
