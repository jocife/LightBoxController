#ifndef PRESETFILEMANAGER_H
#define PRESETFILEMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>

class PresetFileManager
{
public:
    static const QStringList& ledNames();

    static bool save(const QString& filePath, const QVector<int>& values, QString* errorMessage = nullptr);
    static bool load(const QString& filePath, QVector<int>& values, QString* errorMessage = nullptr);
};

#endif // PRESETFILEMANAGER_H