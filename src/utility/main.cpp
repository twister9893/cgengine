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
    QStringList environment;
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
          "    Environment JS files filter string, templates will be processed as many times as files found\n\n"
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
        { "input",       required_argument, nullptr,  'i' },
        { "output",      required_argument, nullptr,  'o' },
        { "template",    required_argument, nullptr,  't' },
        { "environment", required_argument, nullptr,  'e' },
        { "silent",      no_argument,       nullptr,  's' },
        { "help",        no_argument,       nullptr,  'h' },
        { nullptr,       0,                 nullptr,   0  }
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
                opts.environment = QString(optarg).split(" ", QString::SkipEmptyParts);
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
    for (const auto &entry : filter) {
        if (path.contains( QRegExp(entry) )) {
            return true;
        }
    }
    return false;
}

bool generate(cgengine::core::CodeGenerator *generator, const Options &opts)
{
    bool hasErrors = false;
    QDirIterator it(opts.inputDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        if (opts.templates.isEmpty() || pass(opts.templates, it.fileInfo().filePath()))
        {
            const auto &generatedFilePath = generator->generate(it.fileInfo(), opts.outputDir);
            hasErrors |= generator->hasErrors();

            qDebug("[ %s ] %s --> %s", (generator->hasErrors() ? RED "ER" RESET : GREEN "OK" RESET),
                                        it.filePath().toLatin1().data(),
                                        generatedFilePath.toLatin1().data());

            if (generator->hasErrors()) {
                qCritical(RED "    %s\n" RESET, generator->errorString().toLatin1().data());
            }
        }
    }
    return !hasErrors;
}

bool run(cgengine::core::CodeGenerator *generator, const Options &opts)
{
    bool hasErrors = false;
    QDirIterator it(opts.inputDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        if (opts.environment.isEmpty() || pass(opts.environment, it.fileInfo().filePath()))
        {
            generator->exec(it.fileInfo());
            hasErrors |= generator->hasErrors();
            if (generator->hasErrors()) {
                qCritical(RED "    %s\n" RESET, generator->errorString().toLatin1().data());
            }

            hasErrors |= !generate(generator, opts);
        }
    }
    return !hasErrors;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Options opts = options(argc, argv);

    QElapsedTimer timer;
    timer.start();

    cgengine::core::CodeGenerator generator;

    qDebug() << "Generating files...";
    bool hasErrors = !run(&generator, opts);

    qDebug().nospace() << "Finished" << (hasErrors ? " with " RED "errors " RESET : " ") << "in " << timer.elapsed() << " msec";
    return hasErrors;
}
