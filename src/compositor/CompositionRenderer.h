#pragma once

#include "compositor/TextLayer.h"

#include <QImage>
#include <QSize>
#include <QVector>

namespace vesper {

struct RenderResult {
    QImage image;
    QString error;

    bool ok() const { return error.isEmpty(); }
};

RenderResult renderComposition(const QString& backgroundPath, const QVector<TextLayer>& layers,
                               const QSize& targetSize);

void paintLayers(QPainter& painter, const QVector<TextLayer>& layers, const QSize& canvasSize);

}
