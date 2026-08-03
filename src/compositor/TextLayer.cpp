#include "compositor/TextLayer.h"

namespace vesper {

QJsonObject TextLayer::toJson() const {
    return QJsonObject{
        {QStringLiteral("id"), id},
        {QStringLiteral("text"), text},
        {QStringLiteral("x"), x},
        {QStringLiteral("y"), y},
        {QStringLiteral("width"), width},
        {QStringLiteral("font_size"), fontSize},
        {QStringLiteral("font_family"), fontFamily},
        {QStringLiteral("alignment"), alignment},
        {QStringLiteral("color"), color.name(QColor::HexRgb)},
        {QStringLiteral("line_height"), lineHeight},
        {QStringLiteral("letter_spacing"), letterSpacing},
        {QStringLiteral("rotation"), rotation},
        {QStringLiteral("opacity"), opacity},
        {QStringLiteral("bold"), bold},
        {QStringLiteral("italic"), italic},
    };
}

TextLayer TextLayer::fromJson(const QJsonObject& object) {
    TextLayer layer;
    layer.id = object.value(QStringLiteral("id")).toString();
    layer.text = object.value(QStringLiteral("text")).toString();
    layer.x = object.value(QStringLiteral("x")).toDouble(0.1);
    layer.y = object.value(QStringLiteral("y")).toDouble(0.1);
    layer.width = object.value(QStringLiteral("width")).toDouble(0.3);
    layer.fontSize = object.value(QStringLiteral("font_size")).toDouble(0.04);
    layer.fontFamily = object.value(QStringLiteral("font_family")).toString(QStringLiteral("Georgia"));
    layer.alignment = object.value(QStringLiteral("alignment")).toString(QStringLiteral("left"));
    layer.color = QColor(object.value(QStringLiteral("color")).toString(QStringLiteral("#1e1a22")));
    layer.lineHeight = object.value(QStringLiteral("line_height")).toDouble(1.35);
    layer.letterSpacing = object.value(QStringLiteral("letter_spacing")).toDouble(0.0);
    layer.rotation = object.value(QStringLiteral("rotation")).toDouble(0.0);
    layer.opacity = object.value(QStringLiteral("opacity")).toDouble(1.0);
    layer.bold = object.value(QStringLiteral("bold")).toBool(false);
    layer.italic = object.value(QStringLiteral("italic")).toBool(false);
    return layer;
}

}
