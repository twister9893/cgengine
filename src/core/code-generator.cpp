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
    return d_ptr->exec(js);
}

QString CodeGenerator::exec(const QFileInfo &js)
{
    return exec( d_ptr->readFile(js) );
}

QString CodeGenerator::process(const QString &tmpl)
{
    d_ptr->hasErrors = false;
    d_ptr->errorString.clear();

    bool error = false;
    return d_ptr->process(tmpl, &error, true);
}

QString CodeGenerator::process(const QFileInfo &tmpl)
{
    d_ptr->hasErrors = false;
    d_ptr->errorString.clear();

    bool error = false;
    return d_ptr->process( d_ptr->readFile(tmpl), &error, true );
}

bool CodeGenerator::hasErrors() const
{
    return d_ptr->hasErrors;
}

QString CodeGenerator::errorString() const
{
    return d_ptr->errorString;
}

QString CodeGeneratorPrivate::exec(const QString &js)
{
    const auto &value = jse.evaluate(js);
    if (value.isError()) {
        hasErrors = true;
        errorString = "<<<<<< JS runtime ERROR >>>>>>\n\n"
                    + error(js,
                            value.property("message").toString(),
                            value.property("lineNumber").toInt())
                    + "\n\n"
                    + value.toString();
        return QString();
    }
    return (value.isUndefined() ? QString() : value.toString());
}

QString CodeGeneratorPrivate::process(const QString &tmpl, bool *err, bool origin)
{
    QString expr(tmpl);
    QRegExp rx("<%\\n?([^%]+)\\n?%>", Qt::CaseSensitive, QRegExp::RegExp);
    rx.setMinimal(true);

    while ( rx.indexIn(expr) != -1)
    {
        expr.replace(rx.cap(0), process(rx.cap(1), err));
        if (err && *err) { return QString(); }
    }

    if (!origin)
    {
        const auto &value = exec(expr);
        *err = hasErrors;
        return value;
    }

    return expr;
}

QString CodeGeneratorPrivate::error(const QString &pattern, const QString &message, int line)
{
    auto lines = pattern.split(QChar('\n'));
    if (line >= 0 && line < lines.count())
    {
        lines[line-1].append(QString("    <---- %1").arg(message));
    }
    return lines.join(QChar('\n'));
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
