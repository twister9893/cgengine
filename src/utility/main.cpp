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
    QString inputPath;
    QString outputDir = ".";
    QStringList filter;
};

void printUsage(int argc, char *argv[])
{
    Q_UNUSED(argc);

    qInfo("Usage: %s [-i | --input PATH] [OPTIONS]\n", argv[0]);
    qInfo("REQUIRED\n"
          "-i, --input=PATH\n"
          "    Path pattern for template files\n\n"
          "OPTIONS\n"
          "-o, --output=DIR\n"
          "    Output directory, executing directory by default\n\n"
          "-f, --filter=STRING\n"
          "    Input files filter string\n\n"
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
        { "input",  required_argument, nullptr,  'i' },
        { "output", required_argument, nullptr,  'o' },
        { "filter", required_argument, nullptr,  'f' },
        { "silent", no_argument,       nullptr,  's' },
        { "help",   no_argument,       nullptr,  'h' },
        { nullptr,  0,                 nullptr,   0  }
    };

    Options opts;
    int longOptIndex = 0;
    int opt = 0;
    while ( (opt = getopt_long(argc, argv, "i:o:f:sh", longOpts, &longOptIndex)) != -1 )
    {
        switch (opt)
        {
            case 'i':
                opts.inputPath = QString(optarg);
                break;
            case 'o':
                opts.outputDir = QString(optarg);
                break;
            case 'f':
                opts.filter = QString(optarg).split(" ", QString::SkipEmptyParts);
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

    if (opts.inputPath.isEmpty())
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

bool save(const QString &string, const QString &path, QString *errorString = nullptr)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << string;
        return true;
    } else {
        if (errorString) { *errorString = file.errorString(); }
    }
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Options opts = options(argc, argv);

    QElapsedTimer timer;
    timer.start();
    qDebug() << "Generating files...";

    cgengine::core::CodeGenerator generator;
    bool hasErrors = false;

    QDirIterator it(opts.inputPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        if (opts.filter.isEmpty() || pass(opts.filter, it.fileInfo().filePath()))
        {
            const auto &string = generator.process(it.fileInfo());
            bool saveError = false;
            QString saveErrorString;

            if (!generator.hasErrors()) {
                saveError = !save(string, opts.outputDir + QDir::separator() + it.fileInfo().baseName(), &saveErrorString);
            }

            hasErrors |= generator.hasErrors();
            hasErrors |= saveError;

            qDebug("[ %s ] %s", (generator.hasErrors() || saveError ? RED "ER" RESET : GREEN "OK" RESET),
                                it.filePath().toLatin1().data());

            if (generator.hasErrors()) {
                qCritical(RED "    %s\n" RESET, generator.errorString().toLatin1().data());
            }
            if (saveError) {
                qCritical(RED "    %s\n" RESET, saveErrorString.toLatin1().data());
            }
        }
    }

    qDebug().nospace() << "Finished " << (hasErrors ? "with " RED "errors " RESET : " ") << "in " << timer.elapsed() << " msec";
    return hasErrors;
}
