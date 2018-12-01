#include "code-generator.h"
#include "code-generator-private.h"

#include <QDebug>
#include <QTextStream>

namespace cgengine { namespace core {

CodeGenerator::~CodeGenerator()
{}

CodeGenerator::CodeGenerator() :
    d_ptr(new CodeGeneratorPrivate())
{}

CodeGenerator::CodeGenerator(const QString &js) :
    d_ptr(new CodeGeneratorPrivate())
{
    exec(js);
}

CodeGenerator::CodeGenerator(const QFileInfo &js) :
    d_ptr(new CodeGeneratorPrivate())
{
    exec(js);
}

QString CodeGenerator::exec(const QString &js)
{
    return d_ptr->jse.evaluate(js).toString();
}

QString CodeGenerator::exec(const QFileInfo &js)
{
    return exec( d_ptr->readFile(js) );
}

QString CodeGenerator::process(const QString &tmpl)
{
    d_ptr->hasErrors = false;
    d_ptr->errorString.clear();

    return d_ptr->process(tmpl);
}

QString CodeGenerator::process(const QFileInfo &tmpl)
{
    d_ptr->hasErrors = false;
    d_ptr->errorString.clear();

    return d_ptr->process( d_ptr->readFile(tmpl) );
}

bool CodeGenerator::hasErrors() const
{
    return d_ptr->hasErrors;
}

QString CodeGenerator::errorString() const
{
    return d_ptr->errorString;
}

QString CodeGeneratorPrivate::process(const QString &tmpl)
{
    QString expr(tmpl);
    QRegExp rx("%\\{([^%]+)\\}", Qt::CaseSensitive, QRegExp::RegExp);
    rx.setMinimal(true);

    while ( rx.indexIn(expr) != -1)
    {
        expr.replace(rx.cap(0), process(rx.cap(1)));
    }

    const auto &value = jse.evaluate(expr);
    return (value.isUndefined() ? "" : value.toString());
}

QString CodeGeneratorPrivate::readFile(const QFileInfo &fileInfo)
{
    QFile file(fileInfo.filePath());
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        return textStream.readAll();
    } else {
        hasErrors = true;
        errorString = fileInfo.filePath() + " opening failed, " + file.errorString();
    }
    return QString();
}

}}
