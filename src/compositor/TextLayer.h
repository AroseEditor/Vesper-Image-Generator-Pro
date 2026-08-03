#pragma once

#include <QColor>
#include <QJsonObject>
#include <QString>

namespace vesper {

struct TextLayer {
    QString id;
    QString text;
    double x = 0.1;
    double y = 0.1;
    double width = 0.3;
    double fontSize = 0.04;
    QString fontFamily = QStringLiteral("Georgia");
    QString alignment = QStringLiteral("left");
    QColor color = QColor(30, 26, 34);
    double lineHeight = 1.35;
    double letterSpacing = 0.0;
    double rotation = 0.0;
    double opacity = 1.0;
    bool bold = false;
    bool italic = false;

    QJsonObject toJson() const;
    static TextLayer fromJson(const QJsonObject& object);
};

}
