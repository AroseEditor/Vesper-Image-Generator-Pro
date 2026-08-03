#pragma once

#include "inference/GenerationRequest.h"
#include "inference/ProgressParser.h"

#include <QElapsedTimer>
#include <QObject>
#include <QProcess>
#include <QStringList>
#include <qqmlintegration.h>

namespace vesper {

class ModelCatalog;
class GalleryModel;
class AppSettings;

class InferenceBridge : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool engineAvailable READ engineAvailable CONSTANT)
    Q_PROPERTY(QString enginePath READ enginePath CONSTANT)
    Q_PROPERTY(int currentStep READ currentStep NOTIFY progressChanged)
    Q_PROPERTY(int totalSteps READ totalSteps NOTIFY progressChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QStringList log READ log NOTIFY logChanged)
    Q_PROPERTY(QStringList samplers READ samplers CONSTANT)
    Q_PROPERTY(QStringList schedulers READ schedulers CONSTANT)

public:
    explicit InferenceBridge(QObject* parent = nullptr);
    ~InferenceBridge() override;

    bool isRunning() const;
    bool engineAvailable() const;
    QString enginePath() const;
    int currentStep() const;
    int totalSteps() const;
    double progress() const;
    QString statusText() const;
    QStringList log() const;
    static QStringList samplers();
    static QStringList schedulers();

    Q_INVOKABLE void generate(const QVariantMap& options);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void clearLog();

signals:
    void runningChanged();
    void progressChanged();
    void statusTextChanged();
    void logChanged();
    void succeeded(const QString& imagePath, const QString& metadataPath);
    void failed(const QString& reason);
    void cancelled();

private:
    void handleStdout();
    void handleStderr();
    void handleFinished(int exitCode, QProcess::ExitStatus status);
    void appendLog(const QString& line);
    void setStatus(const QString& text);
    QStringList collectOutputs() const;

    QProcess* m_process = nullptr;
    ProgressParser m_parser;
    GenerationRequest m_request;
    QString m_enginePath;
    QString m_outputDirectory;
    QString m_outputStem;
    QStringList m_log;
    QString m_status;
    QElapsedTimer m_timer;
    int m_currentStep = 0;
    int m_totalSteps = 0;
    bool m_cancelling = false;
};

}
