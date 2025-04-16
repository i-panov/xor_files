#include "app_args.h"
#include "xor_files_processor.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QStringBuilder>
#include <QThread>
#include <QFile>
#include <QObject>

void processFiles(const AppArgs& args, bool shouldQuit = false)
{
    const auto thread = new QThread();
    thread->setObjectName("XorFilesProcessor");
    const auto processor = new XorFilesProcessor(args);
    processor->moveToThread(thread);

    QObject::connect(processor, &XorFilesProcessor::finished, thread, &QThread::quit);
    QObject::connect(thread, &QThread::started, processor, &XorFilesProcessor::run);
    QObject::connect(processor, &XorFilesProcessor::finished, processor, &XorFilesProcessor::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    if (shouldQuit) {
        QObject::connect(thread, &QThread::finished, &QCoreApplication::quit);
    }

    thread->start();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("xor_files");
    QCoreApplication::setApplicationVersion("1.0");

    try {
        auto args = parseAppArgs(app);
        args.print();

        if (args.xorValue == 0) {
            throw std::invalid_argument("xor value is not valid byte");
        }

        if (args.timerDuration < 0) {
            throw std::invalid_argument("timer duration is negative");
        }

        if (args.timerDuration > 0) {
            const auto timer = new QTimer(&app);

            processFiles(args);

            QObject::connect(timer, &QTimer::timeout, [args]() {
                processFiles(args);
            });

            timer->start(args.timerDuration * 1000);
        } else {
            processFiles(args, true);
        }
    } catch (const std::exception& e) {
        qCritical() << "Error: " << e.what();
        return 1;
    }

    return app.exec();
}
