#pragma once

#include <QString>
#include <QStringList>
#include <optional>

namespace vesper {

struct SamplingProgress {
    int step = 0;
    int totalSteps = 0;
    double secondsPerStep = 0.0;

    double fraction() const {
        return totalSteps > 0 ? static_cast<double>(step) / static_cast<double>(totalSteps) : 0.0;
    }
};

QString stripAnsi(const QString& text);

std::optional<SamplingProgress> parseProgressLine(const QString& line);

class ProgressParser {
public:
    void append(const QString& chunk);
    QStringList takeLines();

private:
    QString m_buffer;
    QStringList m_lines;
};

}
