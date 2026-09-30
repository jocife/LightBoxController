#include "presetfilemanager.h"

#include <QDomDocument>
#include <QFile>
#include <QTextStream>

const QStringList& PresetFileManager::ledNames()
{
    static const QStringList names = {
        "Violet", "Royal Blue", "Blue", "Cyan", "Green", "Lime", "Mint",
        "PC Amber", "Amber", "Red Orange", "Red", "Deep Red", "Far Red",
        "UV 345", "UV 365", "UV 385", "UV 395", "UV 405", "UV 415",
        "Warm White", "Neutral White", "Cool White"
    };
    return names;
}

bool PresetFileManager::save(const QString& filePath, const QVector<int>& values, QString* errorMessage)
{
    if (values.size() != ledNames().size()) {
        if (errorMessage) *errorMessage = "A preset must contain one value for every LED.";
        return false;
    }

    QDomDocument document;
    QDomProcessingInstruction declaration = document.createProcessingInstruction(
        "xml", "version=\"1.0\" encoding=\"UTF-8\"");
    document.appendChild(declaration);
    QDomElement root = document.createElement("LightBoxPresets");
    document.appendChild(root);

    for (int index = 0; index < values.size(); ++index) {
        QString elementName = ledNames().at(index);
        elementName.replace(" ", "_");
        QDomElement led = document.createElement(elementName);
        root.appendChild(led);
        led.appendChild(document.createTextNode(QString::number(qBound(0, values.at(index), 100))));
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    QTextStream stream(&file);
    stream << document.toString(4);
    return true;
}

bool PresetFileManager::load(const QString& filePath, QVector<int>& values, QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    QDomDocument document;
    if (!document.setContent(&file)) {
        if (errorMessage) *errorMessage = "The file is not valid XML.";
        return false;
    }

    QDomElement root = document.documentElement();
    if (root.tagName() != "LightBoxPresets") {
        if (errorMessage) *errorMessage = "The XML root element must be LightBoxPresets.";
        return false;
    }

    QVector<int> loadedValues;
    QDomElement led = root.firstChildElement();
    for (int index = 0; index < ledNames().size(); ++index) {
        if (led.isNull()) {
            if (errorMessage) *errorMessage = "The preset does not contain all LED values.";
            return false;
        }

        bool ok = false;
        int value = led.text().toInt(&ok);
        if (!ok || value < 0 || value > 100) {
            if (errorMessage) *errorMessage = "The preset contains an invalid LED value.";
            return false;
        }

        loadedValues.append(value);
        led = led.nextSiblingElement();
    }

    if (!led.isNull()) {
        if (errorMessage) *errorMessage = "The preset contains too many LED values.";
        return false;
    }

    values = loadedValues;
    return true;
}