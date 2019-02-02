#include "code-generator.h"
#include "code-generator-private.h"

#include <QDebug>
#include <QDir>
#include <QTextStream>

#include "jsconsole.h"

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
    return d_ptr->exec(js).toString();
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

QString CodeGenerator::generate(const QString &tmpl, const QString &path)
{
    d_ptr->hasErrors = false;
    d_ptr->errorString.clear();

    return d_ptr->generate(tmpl, path);
}

QString CodeGenerator::generate(const QFileInfo &tmpl, const QString &path)
{
    d_ptr->hasErrors = false;
    d_ptr->errorString.clear();

    return d_ptr->generate( d_ptr->readFile(tmpl), path );
}

QString CodeGenerator::value(const QString &name, const QString &defaultValue)
{
    const auto &value = d_ptr->jse.evaluate(name);
    return (value.isUndefined() ? defaultValue : value.toString());
}

bool CodeGenerator::hasErrors() const
{
    return d_ptr->hasErrors;
}

QString CodeGenerator::errorString() const
{
    return d_ptr->errorString;
}

CodeGeneratorPrivate::CodeGeneratorPrivate()
{
    JSConsole *console = new JSConsole(&jse);
    const auto & jsconsole = jse.newQObject(console);
    jse.globalObject().setProperty("console", jsconsole);
}

QJSValue CodeGeneratorPrivate::exec(const QString &js)
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
    }
    return value;
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
        return (value.isUndefined() ? QString() : value.toString());
    }

    return expr;
}

QString CodeGeneratorPrivate::generate(const QString &tmpl, const QString &path)
{
    bool err;
    const auto &src = process(tmpl, &err, true);
    if (hasErrors) {
        return QString();
    }

    const auto &fileName = exec("__fileName__");
    if (hasErrors) {
        return QString();
    }

    const QString filePath = path + QDir::separator() + fileName.toString();
    saveFile(src, filePath);
    return filePath;
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

bool CodeGeneratorPrivate::saveFile(const QString &string, const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << string;
        return true;
    } else {
        hasErrors = true;
        errorString = file.errorString();
    }
    return false;
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

}}
