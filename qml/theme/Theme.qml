pragma Singleton
import QtQuick

QtObject {
    readonly property color canvas: "#0d0b12"
    readonly property color canvasTint: "#171326"
    readonly property color glass: Qt.rgba(1, 1, 1, 0.07)
    readonly property color glassStrong: Qt.rgba(1, 1, 1, 0.12)
    readonly property color glassEdge: Qt.rgba(1, 1, 1, 0.16)
    readonly property color glassEdgeSoft: Qt.rgba(1, 1, 1, 0.08)

    readonly property color accent: "#c58af0"
    readonly property color accentDeep: "#7a4bd6"
    readonly property color accentWarm: "#f0a68a"
    readonly property color positive: "#63d3a6"
    readonly property color warning: "#f0c674"
    readonly property color danger: "#f2708a"

    readonly property color text: "#f2eff7"
    readonly property color textMuted: Qt.rgba(0.95, 0.93, 0.98, 0.62)
    readonly property color textFaint: Qt.rgba(0.95, 0.93, 0.98, 0.38)

    readonly property int radiusLarge: 22
    readonly property int radiusMedium: 14
    readonly property int radiusSmall: 9

    readonly property int gapLarge: 24
    readonly property int gapMedium: 16
    readonly property int gapSmall: 9

    readonly property int fontDisplay: 26
    readonly property int fontTitle: 18
    readonly property int fontBody: 14
    readonly property int fontSmall: 12
    readonly property int fontMicro: 11

    readonly property int durationFast: 130
    readonly property int durationBase: 220
    readonly property int durationSlow: 380
}
