#include "inference/ProgressParser.h"

#include <QRegularExpression>

namespace vesper {

namespace {
const QRegularExpression& ansiPattern() {
    static const QRegularExpression pattern(QStringLiteral("\x1B\\[[0-9;?]*[a-zA-Z]"));
    return pattern;
}

const QRegularExpression& progressPattern() {
    static const QRegularExpression pattern(
        QStringLiteral("(\\d+)\\s*/\\s*(\\d+)\\s*-\\s*([0-9]*\\.?[0-9]+)\\s*(s/it|it/s)"));
    return pattern;
}
}

QString stripAnsi(const QString& text) {
    QString cleaned = text;
    cleaned.remove(ansiPattern());
    return cleaned;
}

std::optional<SamplingProgress> parseProgressLine(const QString& line) {
    const QRegularExpressionMatch match = progressPattern().match(stripAnsi(line));
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    SamplingProgress progress;
    progress.step = match.captured(1).toInt();
    progress.totalSteps = match.captured(2).toInt();

    const double rate = match.captured(3).toDouble();
    progress.secondsPerStep =
        match.captured(4) == QLatin1String("it/s") ? (rate > 0.0 ? 1.0 / rate : 0.0) : rate;

    if (progress.totalSteps <= 0 || progress.step < 0) {
        return std::nullopt;
    }
    return progress;
}

void ProgressParser::append(const QString& chunk) {
    m_buffer += chunk;

    int start = 0;
    for (int i = 0; i < m_buffer.size(); ++i) {
        const QChar character = m_buffer.at(i);
        if (character == QLatin1Char('\n') || character == QLatin1Char('\r')) {
            const QString line = m_buffer.mid(start, i - start);
            if (!line.trimmed().isEmpty()) {
                m_lines.append(stripAnsi(line).trimmed());
            }
            start = i + 1;
        }
    }
    m_buffer = m_buffer.mid(start);
}

QStringList ProgressParser::takeLines() {
    QStringList lines;
    lines.swap(m_lines);
    return lines;
}

}
