#include "compositor/CompositionRenderer.h"
#include "compositor/TextLayer.h"

#include <QImage>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

using namespace vesper;

class TestComposition : public QObject {
    Q_OBJECT

private slots:
    void layerSurvivesJsonRoundTrip();
    void rendersAtRequestedResolution();
    void normalizedCoordinatesScaleWithCanvas();
    void rendersWithoutBackground();
    void missingBackgroundIsReported();
};

void TestComposition::layerSurvivesJsonRoundTrip() {
    TextLayer original;
    original.id = QStringLiteral("title");
    original.text = QStringLiteral("With love\nand care");
    original.x = 0.06;
    original.y = 0.14;
    original.width = 0.24;
    original.fontSize = 0.075;
    original.fontFamily = QStringLiteral("Segoe Script");
    original.alignment = QStringLiteral("center");
    original.color = QColor(QStringLiteral("#5c2b4a"));
    original.lineHeight = 1.45;
    original.letterSpacing = 0.06;
    original.rotation = -2.5;
    original.opacity = 0.9;
    original.italic = true;

    const TextLayer restored = TextLayer::fromJson(original.toJson());

    QCOMPARE(restored.id, original.id);
    QCOMPARE(restored.text, original.text);
    QCOMPARE(restored.x, original.x);
    QCOMPARE(restored.width, original.width);
    QCOMPARE(restored.fontSize, original.fontSize);
    QCOMPARE(restored.fontFamily, original.fontFamily);
    QCOMPARE(restored.alignment, original.alignment);
    QCOMPARE(restored.color.name(), original.color.name());
    QCOMPARE(restored.lineHeight, original.lineHeight);
    QCOMPARE(restored.rotation, original.rotation);
    QCOMPARE(restored.italic, original.italic);
}

void TestComposition::rendersAtRequestedResolution() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString backgroundPath = dir.filePath(QStringLiteral("bg.png"));
    QImage background(400, 300, QImage::Format_ARGB32);
    background.fill(Qt::darkCyan);
    QVERIFY(background.save(backgroundPath));

    TextLayer layer;
    layer.text = QStringLiteral("Export me");
    layer.x = 0.1;
    layer.y = 0.1;
    layer.width = 0.5;
    layer.fontSize = 0.06;

    const RenderResult small = renderComposition(backgroundPath, {layer}, QSize(800, 600));
    QVERIFY2(small.ok(), qPrintable(small.error));
    QCOMPARE(small.image.size(), QSize(800, 600));

    const RenderResult large = renderComposition(backgroundPath, {layer}, QSize(3200, 2400));
    QVERIFY(large.ok());
    QCOMPARE(large.image.size(), QSize(3200, 2400));
}

void TestComposition::normalizedCoordinatesScaleWithCanvas() {
    TextLayer layer;
    layer.text = QStringLiteral("A");
    layer.x = 0.5;
    layer.y = 0.5;
    layer.width = 0.4;
    layer.fontSize = 0.1;
    layer.color = QColor(Qt::black);

    const RenderResult base = renderComposition(QString(), {layer}, QSize(500, 500));
    const RenderResult doubled = renderComposition(QString(), {layer}, QSize(1000, 1000));

    QVERIFY(base.ok());
    QVERIFY(doubled.ok());

    const auto inkColumns = [](const QImage& image) {
        int first = image.width();
        int last = -1;
        for (int x = 0; x < image.width(); ++x) {
            for (int y = 0; y < image.height(); ++y) {
                if (qGray(image.pixel(x, y)) < 200) {
                    first = qMin(first, x);
                    last = qMax(last, x);
                    break;
                }
            }
        }
        return QPair<int, int>(first, last);
    };

    const auto baseInk = inkColumns(base.image.convertToFormat(QImage::Format_RGB32));
    const auto doubledInk = inkColumns(doubled.image.convertToFormat(QImage::Format_RGB32));

    QVERIFY(baseInk.second > 0);
    QVERIFY(doubledInk.second > 0);
    QVERIFY(qAbs(doubledInk.first - baseInk.first * 2) <= 6);
}

void TestComposition::rendersWithoutBackground() {
    TextLayer layer;
    layer.text = QStringLiteral("No background");

    const RenderResult result = renderComposition(QString(), {layer}, QSize(640, 480));
    QVERIFY(result.ok());
    QCOMPARE(result.image.size(), QSize(640, 480));
}

void TestComposition::missingBackgroundIsReported() {
    const RenderResult result =
        renderComposition(QStringLiteral("/definitely/not/here.png"), {}, QSize(100, 100));
    QVERIFY(!result.ok());
}

QTEST_MAIN(TestComposition)
#include "tst_composition.moc"
