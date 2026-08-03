#include "compositor/CompositionRenderer.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QTextOption>

namespace vesper {

namespace {

Qt::Alignment alignmentFor(const QString& name) {
    if (name == QLatin1String("center")) return Qt::AlignHCenter | Qt::AlignTop;
    if (name == QLatin1String("right")) return Qt::AlignRight | Qt::AlignTop;
    if (name == QLatin1String("justify")) return Qt::AlignJustify | Qt::AlignTop;
    return Qt::AlignLeft | Qt::AlignTop;
}

}

void paintLayers(QPainter& painter, const QVector<TextLayer>& layers, const QSize& canvasSize) {
    const double canvasWidth = canvasSize.width();
    const double canvasHeight = canvasSize.height();

    for (const TextLayer& layer : layers) {
        if (layer.text.isEmpty()) {
            continue;
        }

        painter.save();

        const double pixelSize = qMax(1.0, layer.fontSize * canvasHeight);
        QFont font(layer.fontFamily);
        font.setPixelSize(static_cast<int>(std::lround(pixelSize)));
        font.setBold(layer.bold);
        font.setItalic(layer.italic);
        if (!qFuzzyIsNull(layer.letterSpacing)) {
            font.setLetterSpacing(QFont::AbsoluteSpacing, layer.letterSpacing * pixelSize);
        }
        painter.setFont(font);
        painter.setPen(layer.color);
        painter.setOpacity(qBound(0.0, layer.opacity, 1.0));
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        const double blockWidth = layer.width * canvasWidth;
        const QPointF origin(layer.x * canvasWidth, layer.y * canvasHeight);

        painter.translate(origin);
        if (!qFuzzyIsNull(layer.rotation)) {
            painter.rotate(layer.rotation);
        }

        QTextOption option(alignmentFor(layer.alignment));
        option.setWrapMode(QTextOption::WordWrap);

        const QFontMetricsF metrics(font);
        const double lineAdvance = metrics.lineSpacing() * layer.lineHeight;
        const QRectF bounds(0.0, 0.0, blockWidth, canvasHeight);

        const QStringList paragraphs = layer.text.split(QLatin1Char('\n'));
        double cursorY = 0.0;
        for (const QString& paragraph : paragraphs) {
            if (paragraph.isEmpty()) {
                cursorY += lineAdvance;
                continue;
            }
            QRectF measured =
                metrics.boundingRect(QRectF(0, 0, blockWidth, canvasHeight),
                                     static_cast<int>(alignmentFor(layer.alignment)) |
                                         Qt::TextWordWrap,
                                     paragraph);
            const QRectF target(0.0, cursorY, blockWidth, measured.height() + lineAdvance);
            painter.drawText(target, paragraph, option);

            const int lineCount =
                qMax(1, static_cast<int>(std::lround(measured.height() / metrics.lineSpacing())));
            cursorY += lineAdvance * lineCount;
        }

        Q_UNUSED(bounds);
        painter.restore();
    }
}

RenderResult renderComposition(const QString& backgroundPath, const QVector<TextLayer>& layers,
                               const QSize& targetSize) {
    RenderResult result;

    QImage background;
    if (!backgroundPath.isEmpty() && !background.load(backgroundPath)) {
        result.error = QStringLiteral("Could not read the background image");
        return result;
    }

    QSize canvasSize = targetSize;
    if (!canvasSize.isValid() || canvasSize.isEmpty()) {
        canvasSize = background.isNull() ? QSize(1024, 1024) : background.size();
    }

    QImage canvas(canvasSize, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!background.isNull()) {
        const QImage scaled =
            background.scaled(canvasSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const QPoint offset((canvasSize.width() - scaled.width()) / 2,
                            (canvasSize.height() - scaled.height()) / 2);
        painter.drawImage(offset, scaled);
    }

    paintLayers(painter, layers, canvasSize);
    painter.end();

    result.image = canvas;
    return result;
}

}
