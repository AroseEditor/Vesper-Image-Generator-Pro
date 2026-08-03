import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Dialogs
import Vesper
import Vesper.theme
import Vesper.components

Item {
    id: root

    property int seedValue: -1
    property bool seedLocked: false
    property string initImagePath: ""

    function installedModels() {
        return ModelCatalog.installedModelIds();
    }

    function applyModelDefaults(modelId) {
        const defaults = ModelCatalog.defaultsFor(modelId);
        if (!defaults.sampler)
            return;
        samplerCombo.currentIndex = Math.max(0, InferenceBridge.samplers.indexOf(defaults.sampler));
        stepsSlider.value = defaults.steps;
        cfgSlider.value = defaults.cfgScale;
        widthSlider.value = defaults.width;
        heightSlider.value = defaults.height;
    }

    function currentModelId() {
        return modelCombo.count > 0 ? modelCombo.currentText : "";
    }

    function launch() {
        const seed = root.seedLocked && root.seedValue >= 0 ? root.seedValue : -1;
        InferenceBridge.generate({
            mode: root.initImagePath.length > 0 ? "img2img" : "txt2img",
            modelId: root.currentModelId(),
            prompt: promptField.text,
            negativePrompt: negativeField.text,
            initImage: root.initImagePath,
            strength: strengthSlider.value,
            steps: Math.round(stepsSlider.value),
            cfgScale: cfgSlider.value,
            sampler: samplerCombo.currentText,
            scheduler: schedulerCombo.currentText,
            width: Math.round(widthSlider.value),
            height: Math.round(heightSlider.value),
            seed: seed,
            threads: AppSettings.threadCount,
            backend: AppSettings.backend,
            offloadToCpu: AppSettings.offloadToCpu,
            flashAttention: AppSettings.flashAttention
        });
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapMedium
        spacing: Theme.gapMedium

        GlassPanel {
            Layout.preferredWidth: 372
            Layout.fillHeight: true
            elevated: true

            ScrollView {
                anchors.fill: parent
                anchors.margins: Theme.gapLarge
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    width: parent.parent.width - Theme.gapLarge * 2
                    spacing: Theme.gapMedium

                    SectionLabel { text: "Model" }

                    GlassCombo {
                        id: modelCombo
                        Layout.fillWidth: true
                        model: ModelCatalog.installedModelIds()
                        onActivated: {
                            AppSettings.selectedModelId = currentText;
                            root.applyModelDefaults(currentText);
                        }

                        Connections {
                            target: ModelCatalog
                            function onInstalledCountChanged() {
                                modelCombo.model = ModelCatalog.installedModelIds();
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: modelCombo.count === 0
                        text: "No models installed yet. Open the Models screen to download one."
                        color: Theme.warning
                        font.pixelSize: Theme.fontSmall
                        wrapMode: Text.WordWrap
                    }

                    SectionLabel { text: "Prompt" }

                    GlassField {
                        id: promptField
                        Layout.fillWidth: true
                        multiline: true
                        rows: 6
                        placeholder: "Describe the image you want"
                    }

                    SectionLabel { text: "Negative prompt" }

                    GlassField {
                        id: negativeField
                        Layout.fillWidth: true
                        multiline: true
                        rows: 3
                        placeholder: "What to avoid"
                    }

                    SectionLabel { text: "Reference image" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapSmall

                        Text {
                            Layout.fillWidth: true
                            text: root.initImagePath.length > 0
                                  ? root.initImagePath.split(/[\\/]/).pop()
                                  : "None. Generation runs as txt2img."
                            color: root.initImagePath.length > 0 ? Theme.text : Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideMiddle
                        }

                        GlassButton {
                            text: root.initImagePath.length > 0 ? "Clear" : "Choose"
                            implicitWidth: 80
                            onClicked: {
                                if (root.initImagePath.length > 0)
                                    root.initImagePath = "";
                                else
                                    initPicker.open();
                            }
                        }
                    }

                    GlassSlider {
                        Layout.fillWidth: true
                        id: strengthSlider
                        visible: root.initImagePath.length > 0
                        label: "Denoising strength"
                        from: 0.1
                        to: 1.0
                        stepSize: 0.05
                        decimals: 2
                        value: 0.75
                        onMoved: function(v) { value = v }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Theme.glassEdgeSoft
                    }

                    SectionLabel { text: "Sampling" }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: Theme.gapSmall
                        rowSpacing: Theme.gapSmall

                        GlassCombo {
                            id: samplerCombo
                            Layout.fillWidth: true
                            model: InferenceBridge.samplers
                        }

                        GlassCombo {
                            id: schedulerCombo
                            Layout.fillWidth: true
                            model: InferenceBridge.schedulers
                        }
                    }

                    GlassSlider {
                        id: stepsSlider
                        Layout.fillWidth: true
                        label: "Steps"
                        from: 1
                        to: 60
                        stepSize: 1
                        value: 20
                        onMoved: function(v) { value = v }
                    }

                    GlassSlider {
                        id: cfgSlider
                        Layout.fillWidth: true
                        label: "CFG scale"
                        from: 1
                        to: 20
                        stepSize: 0.5
                        decimals: 1
                        value: 7
                        onMoved: function(v) { value = v }
                    }

                    GlassSlider {
                        id: widthSlider
                        Layout.fillWidth: true
                        label: "Width"
                        from: 256
                        to: 1536
                        stepSize: 64
                        value: 512
                        suffix: " px"
                        onMoved: function(v) { value = v }
                    }

                    GlassSlider {
                        id: heightSlider
                        Layout.fillWidth: true
                        label: "Height"
                        from: 256
                        to: 1536
                        stepSize: 64
                        value: 512
                        suffix: " px"
                        onMoved: function(v) { value = v }
                    }

                    SectionLabel { text: "Seed" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapSmall

                        GlassField {
                            id: seedField
                            Layout.fillWidth: true
                            text: root.seedValue >= 0 ? String(root.seedValue) : ""
                            placeholder: "Random"
                            onEdited: {
                                const parsed = parseInt(text, 10);
                                root.seedValue = isNaN(parsed) ? -1 : parsed;
                            }
                        }

                        GlassButton {
                            text: root.seedLocked ? "Locked" : "Random"
                            implicitWidth: 96
                            primary: root.seedLocked
                            onClicked: root.seedLocked = !root.seedLocked
                        }
                    }

                    Item { Layout.preferredHeight: Theme.gapSmall }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.gapMedium

            GlassPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Image {
                    id: preview
                    anchors.fill: parent
                    anchors.margins: Theme.gapMedium
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: false
                    visible: source.toString().length > 0
                }

                Column {
                    anchors.centerIn: parent
                    spacing: Theme.gapSmall
                    visible: !preview.visible
                    width: parent.width * 0.6

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: "Nothing generated yet"
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontTitle
                    }

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: InferenceBridge.engineAvailable
                              ? "Write a prompt, pick an installed model, and press Generate."
                              : "The sd-cli engine was not found next to the application."
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                        wrapMode: Text.WordWrap
                    }
                }
            }

            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 132

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.gapMedium
                    spacing: Theme.gapSmall

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapMedium

                        Text {
                            Layout.fillWidth: true
                            text: InferenceBridge.statusText
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideRight
                        }

                        Text {
                            visible: InferenceBridge.totalSteps > 0
                            text: InferenceBridge.currentStep + " / " + InferenceBridge.totalSteps
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 5
                        radius: 3
                        color: Qt.rgba(1, 1, 1, 0.08)

                        Rectangle {
                            width: parent.width * InferenceBridge.progress
                            height: parent.height
                            radius: parent.radius
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: Theme.accentDeep }
                                GradientStop { position: 1.0; color: Theme.accent }
                            }

                            Behavior on width { NumberAnimation { duration: Theme.durationFast } }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapSmall

                        GlassButton {
                            text: "Copy seed"
                            enabled: root.seedValue >= 0
                            onClicked: root.seedLocked = true
                        }

                        Item { Layout.fillWidth: true }

                        GlassButton {
                            text: "Cancel"
                            visible: InferenceBridge.running
                            onClicked: InferenceBridge.cancel()
                        }

                        GlassButton {
                            text: InferenceBridge.running ? "Generating" : "Generate"
                            primary: true
                            implicitWidth: 132
                            enabled: !InferenceBridge.running && modelCombo.count > 0
                                     && promptField.text.trim().length > 0
                                     && InferenceBridge.engineAvailable
                            onClicked: root.launch()
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: initPicker
        title: "Choose a reference image"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.webp)"]
        onAccepted: root.initImagePath = selectedFile
    }

    Connections {
        target: InferenceBridge

        function onSucceeded(imagePath, metadataPath) {
            preview.source = "";
            preview.source = "file:///" + imagePath.replace(/\\/g, "/");
            GalleryModel.refresh();
        }

        function onFailed(reason) {
            errorToast.show(reason);
        }
    }

    Rectangle {
        id: errorToast

        function show(message) {
            toastText.text = message;
            opacity = 1;
            hideTimer.restart();
        }

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.gapLarge * 2
        width: Math.min(parent.width - Theme.gapLarge * 2, toastText.implicitWidth + 36)
        height: toastText.implicitHeight + 24
        radius: Theme.radiusMedium
        color: "#3a1723"
        border.width: 1
        border.color: Theme.danger
        opacity: 0

        Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }

        Text {
            id: toastText
            anchors.centerIn: parent
            width: parent.width - 32
            color: Theme.text
            font.pixelSize: Theme.fontSmall
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Timer {
            id: hideTimer
            interval: 6000
            onTriggered: errorToast.opacity = 0
        }
    }
}
