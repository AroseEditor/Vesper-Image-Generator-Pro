#include "inference/ProgressParser.h"

#include <QTest>

using namespace vesper;

class TestProgressParser : public QObject {
    Q_OBJECT

private slots:
    void parsesTheRealEngineFormat();
    void parsesIterationsPerSecondForm();
    void ignoresOrdinaryLogLines();
    void stripsAnsiEscapeSequences();
    void splitsOnCarriageReturn();
    void handlesChunkSplitMidLine();
    void fractionReflectsCompletion();
};

void TestProgressParser::parsesTheRealEngineFormat() {
    const QString line = QStringLiteral("  |=====>         | 5/20 - 1.23s/it");
    const auto progress = parseProgressLine(line);

    QVERIFY(progress.has_value());
    QCOMPARE(progress->step, 5);
    QCOMPARE(progress->totalSteps, 20);
    QCOMPARE(progress->secondsPerStep, 1.23);
}

void TestProgressParser::parsesIterationsPerSecondForm() {
    const auto progress = parseProgressLine(QStringLiteral("  |==| 8/8 - 4.00it/s"));

    QVERIFY(progress.has_value());
    QCOMPARE(progress->step, 8);
    QCOMPARE(progress->totalSteps, 8);
    QCOMPARE(progress->secondsPerStep, 0.25);
}

void TestProgressParser::ignoresOrdinaryLogLines() {
    QVERIFY(!parseProgressLine(QStringLiteral("[INFO ] stable-diffusion.cpp:180  - loading model")));
    QVERIFY(!parseProgressLine(QStringLiteral("sampling completed")));
    QVERIFY(!parseProgressLine(QString()));
}

void TestProgressParser::stripsAnsiEscapeSequences() {
    const QString line = QStringLiteral("\x1B[32m  |===>  | 3/10 - 0.50s/it\x1B[K\x1B[0m");
    const auto progress = parseProgressLine(line);

    QVERIFY(progress.has_value());
    QCOMPARE(progress->step, 3);
    QCOMPARE(progress->totalSteps, 10);
}

void TestProgressParser::splitsOnCarriageReturn() {
    ProgressParser parser;
    parser.append(QStringLiteral("\r  |=>   | 1/4 - 2.00s/it\r  |==>  | 2/4 - 2.00s/it\r"));

    const QStringList lines = parser.takeLines();
    QCOMPARE(lines.size(), 2);
    QVERIFY(parseProgressLine(lines.at(0)).has_value());
    QCOMPARE(parseProgressLine(lines.at(1))->step, 2);
}

void TestProgressParser::handlesChunkSplitMidLine() {
    ProgressParser parser;
    parser.append(QStringLiteral("  |====> | 7/2"));
    QVERIFY(parser.takeLines().isEmpty());

    parser.append(QStringLiteral("0 - 0.90s/it\r"));
    const QStringList lines = parser.takeLines();

    QCOMPARE(lines.size(), 1);
    const auto progress = parseProgressLine(lines.first());
    QVERIFY(progress.has_value());
    QCOMPARE(progress->step, 7);
    QCOMPARE(progress->totalSteps, 20);
}

void TestProgressParser::fractionReflectsCompletion() {
    const auto progress = parseProgressLine(QStringLiteral("| 10/40 - 1.00s/it"));
    QVERIFY(progress.has_value());
    QCOMPARE(progress->fraction(), 0.25);
}

QTEST_MAIN(TestProgressParser)
#include "tst_progressparser.moc"
