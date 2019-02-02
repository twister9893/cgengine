#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QElapsedTimer>
#include <getopt.h>

#include <core/code-generator.h>
#include "ansi-colors.h"
#include "qt-msg-mute-handler.h"

class Options {
public:
    QString inputDir;
    QString outputDir = ".";
    QStringList templates;
    QStringList environments;
    QStringList preprocessors;
};

void printUsage(int argc, char *argv[])
{
    Q_UNUSED(argc);

    qInfo("Usage: %s [-i | --input PATH] [OPTIONS]\n", argv[0]);
    qInfo("REQUIRED\n"
          "-i, --input=DIR\n"
          "    Path pattern for template files\n\n"
          "OPTIONS\n"
          "-o, --output=DIR\n"
          "    Output directory, executing directory by default\n\n"
          "-t, --template=STRING\n"
          "    Template files filter string\n\n"
          "-e, --environment=STRING\n"
          "    Environment JS files filter string, templates processing runs for each found file\n\n"
          "-s, --silent\n"
          "    Silent mode, no console output\n\n"
          "-h, --help\n"
          "    Prints this message\n\n"
          );
}

Options options(int argc, char *argv[])
{
    static struct option longOpts[] =
    {
        { "input",        required_argument, nullptr,  'i' },
        { "output",       required_argument, nullptr,  'o' },
        { "template",     required_argument, nullptr,  't' },
        { "environment",  required_argument, nullptr,  'e' },
        { "silent",       no_argument,       nullptr,  's' },
        { "help",         no_argument,       nullptr,  'h' },
        { nullptr,        0,                 nullptr,   0  }
    };

    Options opts;
    int longOptIndex = 0;
    int opt = 0;
    while ( (opt = getopt_long(argc, argv, "i:o:t:e:sh", longOpts, &longOptIndex)) != -1 )
    {
        switch (opt)
        {
            case 'i':
                opts.inputDir = QString(optarg);
                break;
            case 'o':
                opts.outputDir = QString(optarg);
                break;
            case 't':
                opts.templates = QString(optarg).split(" ", QString::SkipEmptyParts);
                break;
            case 'e':
                opts.environments = QString(optarg).split(" ", QString::SkipEmptyParts);
                break;
            case 's':
                qInstallMessageHandler(qtMsgMuteHandler);
                break;
            case 'h':
            case '?':
            default:
                printUsage(argc, argv);
                exit(EXIT_FAILURE);
        }
    }

    if (opts.inputDir.isEmpty())
    {
        qCritical("Missing required options");
        printUsage(argc, argv);
        exit(EXIT_FAILURE);
    }

    return opts;
}

bool pass(const QStringList &filter, const QString &path)
{
    if (filter.isEmpty()) {
        return true;
    }
    for (const auto &entry : filter) {
        if (path.contains( QRegExp(entry, Qt::CaseSensitive, QRegExp::WildcardUnix) )) {
            return true;
        }
    }
    return false;
}

bool process(cgengine::core::CodeGenerator *generator, const Options &opts)
{
    bool hasErrors = false;
    QDirIterator it(opts.inputDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        if (!pass(opts.templates,
                  it.fileInfo().filePath()))
        {
            continue;
        }
        if (!pass( generator->value("__filter__").split(" ", QString::SkipEmptyParts),
                   it.fileInfo().filePath()))
        {
            continue;
        }

        const auto &generatedFilePath = generator->generate(it.fileInfo(), opts.outputDir);
        hasErrors |= generator->hasErrors();

        qDebug("[ %s ] %s --> %s", (generator->hasErrors() ? RED "ER" RESET : GREEN "OK" RESET),
                                    it.fileName().toLatin1().data(),
                                    QFileInfo(generatedFilePath).fileName().toLatin1().data());

        if (generator->hasErrors()) {
            qCritical(RED "    %s\n" RESET, generator->errorString().toLatin1().data());
        }
    }
    return !hasErrors;
}

bool generate(cgengine::core::CodeGenerator *generator, const Options &opts)
{
    bool hasErrors = false;

    const auto & preprocessorPath = generator->value("__preprocessor__", "");
    if (!preprocessorPath.isEmpty())
    {
        generator->exec(QFileInfo(opts.inputDir + QDir::separator() + preprocessorPath));
        hasErrors |= generator->hasErrors();

        if (generator->hasErrors()) {
            qCritical(RED "    %s\n" RESET, generator->errorString().toLatin1().data());
        }
    }

    hasErrors |= !process(generator, opts);

    return !hasErrors;
}

QStringList collectEnvironments(const QString &inputDir, const QStringList &filter)
{
    QDirIterator it(inputDir, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QStringList envFiles;
    while (it.hasNext())
    {
        it.next();
        if (pass(filter, it.fileInfo().filePath()))
        {
            envFiles << it.filePath();
        }
    }
    envFiles.sort();
    return envFiles;
}

bool run(cgengine::core::CodeGenerator *generator, const Options &opts)
{
    bool hasErrors = false;

    const auto &envFiles = collectEnvironments(opts.inputDir, opts.environments);
    for (const auto &envPath : envFiles)
    {
        generator->exec(QFileInfo(envPath));
        hasErrors |= generator->hasErrors();

        if (generator->hasErrors()) {
            qCritical(RED "    %s\n" RESET, generator->errorString().toLatin1().data());
        }

        int repeats = generator->value("__repeat__", "0").toInt();
        while (repeats--) {
            hasErrors |= !generate(generator, opts);
        }
    }
    return !hasErrors;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Options opts = options(argc, argv);
    cgengine::core::CodeGenerator generator;

    QElapsedTimer timer;
    timer.start();

    qDebug() << "Generating files...";
    bool hasErrors = !run(&generator, opts);

    qDebug().nospace() << "Finished" << (hasErrors ? " with " RED "errors " RESET : " ") << "in " << timer.elapsed() << " msec";
    return hasErrors;
}
