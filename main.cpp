#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QStringBuilder>

struct AppArgs {
    QDir inputDir, outputDir;
    bool removeSources, modifyOutput;
    int timerDuration;
    quint8 xorValue;

    void print() {
        qDebug() << "inputDir: " << inputDir;
        qDebug() << "outputDir: " << outputDir;
        qDebug() << "removeSources: " << removeSources;
        qDebug() << "modifyOutput: " << modifyOutput;
        qDebug() << "timerDuration: " << timerDuration;
        qDebug() << "xorValue: " << xorValue;
    }
};

AppArgs parseArguments(const QCoreApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("XOR files processor");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption maskOption("mask", "source files mask", "mask", "*");
    QCommandLineOption removeSourcesOption("remove", "remove source files");
    QCommandLineOption inputPathOption("input", "input files path", "path");
    QCommandLineOption outputPathOption("output", "output files path", "path");
    QCommandLineOption modifyOutputNameOption("modify_output", "modify output file names on conflicts (default: overrite)");
    QCommandLineOption timerDurationOption("timer", "timer duration in seconds (default: single start)", "duration", "0");
    QCommandLineOption xorValueOption("value", "xor value number", "number", "0");

    parser.addOption(maskOption);
    parser.addOption(removeSourcesOption);
    parser.addOption(inputPathOption);
    parser.addOption(outputPathOption);
    parser.addOption(modifyOutputNameOption);
    parser.addOption(timerDurationOption);
    parser.addOption(xorValueOption);

    parser.process(app);

    auto mask = parser.value(maskOption);
    auto inputPath = parser.value(inputPathOption);
    auto outputPath = parser.value(outputPathOption);

    AppArgs args;

    args.inputDir = QDir(inputPath);
    args.inputDir.setNameFilters(QStringList() << mask);
    args.inputDir.setFilter(QDir::Files);

    args.outputDir = QDir(outputPath);
    args.outputDir.setFilter(QDir::Files);

    args.removeSources = parser.isSet(removeSourcesOption);
    args.modifyOutput = parser.isSet(modifyOutputNameOption);
    args.timerDuration = parser.value(timerDurationOption).toInt();

    bool ok;
    int xorVal = parser.value(xorValueOption).toInt(&ok);
    args.xorValue = (ok && xorVal >= 0 && xorVal <= 255) ? static_cast<quint8>(xorVal) : 0;

    return args;
}

void processFiles(const AppArgs& args) {
    try {
        if (!args.inputDir.exists()) {
            throw std::invalid_argument("input path is unknown");
        }

        if (!args.outputDir.exists()) {
            throw std::invalid_argument("output path is unknown");
        }

        for (auto& inputFileInfo : args.inputDir.entryInfoList()) {
            QFile inputFile(inputFileInfo.absoluteFilePath());

            if (!inputFile.open(QIODevice::ReadOnly)) {
                qWarning() << "Cannot open input file " << inputFile.errorString();
                continue;
            }

            QFileInfo outputFileInfo(args.outputDir, inputFileInfo.fileName());

            if (outputFileInfo.exists() && args.modifyOutput) {
                auto timestamp = QDateTime::currentSecsSinceEpoch();
                QString newName = inputFileInfo.baseName() % '_' % QString::number(timestamp) % '.' % inputFileInfo.suffix();
                outputFileInfo.setFile(args.outputDir, newName);
            }

            QFile outputFile(outputFileInfo.absoluteFilePath());

            if (!outputFile.open(QIODevice::WriteOnly)) {
                qWarning() << "Cannot open input file " << inputFile.errorString();
                continue;
            }

            const int bufferSize = 8192;
            char buffer[bufferSize];

            while (!inputFile.atEnd()) {
                auto bytesRead = inputFile.read(buffer, bufferSize);

                if (bytesRead == -1) {
                    qWarning() << "Read error";
                    break;
                }

                for (int i = 0; i < bytesRead; ++i) {
                    buffer[i] ^= args.xorValue;
                }

                outputFile.write(buffer, bytesRead);
            }

            inputFile.close();
            outputFile.close();

            if (args.removeSources) {
                inputFile.remove();
            }
        }
    } catch (const std::exception& e) {
        qCritical() << "Error: " << e.what();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("xor_files");
    QCoreApplication::setApplicationVersion("1.0");

    try {
        auto args = parseArguments(app);
        args.print();

        if (args.xorValue == 0) {
            throw std::invalid_argument("xor value is not valid byte");
        }

        if (args.timerDuration < 0) {
            throw std::invalid_argument("timer duration is negative");
        }

        if (args.timerDuration == 0) {
            processFiles(args);
        } else {
            QTimer timer;

            QObject::connect(&timer, &QTimer::timeout, [args]() {
                processFiles(args);
            });

            timer.start(args.timerDuration * 1000);

            return app.exec();
        }
    } catch (const std::exception& e) {
        qCritical() << "Error: " << e.what();
        return 1;
    }

    return 0;
}
