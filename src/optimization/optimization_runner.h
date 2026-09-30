#ifndef OPTIMIZATIONRUNNER_H
#define OPTIMIZATIONRUNNER_H

#include <QString>
#include <QVector>

// Struct to hold expanded metrics
struct EvaluatedMetrics {
    double Mu;
    double Mv;
    double Duv;
    double CRI;
    double CCT;
    QVector<double> testSPD;
    QVector<double> refSPD;
    QVector<double> wavelengths;
};

// Function declarations
QVector<double> runFminconOptimization(const QString &csvFilePath, int illuminantType, 
                                     QChar decimalSeparator = ',', QChar colSeparator = ';',
                                     const QVector<int> &selectedLedIndices = {});

EvaluatedMetrics runMetricsEvaluation(const QVector<double> &weights, const QString &csvFilePath, 
                                      int illuminantType, QChar decimalSeparator = ',', QChar colSeparator = ';',
                                      const QVector<int> &selectedLedIndices = {});

#endif // OPTIMIZATIONRUNNER_H