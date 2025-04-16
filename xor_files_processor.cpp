#include "xor_files_processor.h"

XorFilesProcessor::XorFilesProcessor(const AppArgs& args, QObject *parent): QObject{parent}
{
    this->args = args;
}

void XorFilesProcessor::run()
{
    try {
        if (!args.inputDir.exists()) {
            throw std::invalid_argument("input path is unknown");
        }

        if (!args.outputDir.exists()) {
            throw std::invalid_argument("output path is unknown");
        }

        const auto inputFiles = args.inputDir.entryInfoList();
        const int bufferSize = 8192;
        char buffer[bufferSize];

        for (const auto& inputFileInfo : inputFiles) {
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
                qWarning() << "Cannot open output file " << outputFile.errorString();
                continue;
            }

            while (!inputFile.atEnd()) {
                const auto bytesRead = inputFile.read(buffer, bufferSize);

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

    emit finished();
}
