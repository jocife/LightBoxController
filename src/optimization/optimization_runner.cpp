#include "optimization_runner.h"
#include "optimize_led_weights.h" // The MATLAB generated header

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <cmath>
#include <QDebug>

namespace {
constexpr int WLS_COUNT = 81;
constexpr int LED_COUNT = 22;

// Helper function to determine if a given LED index is selected based on the provided list of selected indices.
bool isLedSelected(int ledIndex, const QVector<int> &selectedLedIndices)
{
    return selectedLedIndices.isEmpty() || selectedLedIndices.contains(ledIndex);
}
}

/**
 * @brief Runs the fmincon optimization using the generated MATLAB DLL
 * @param csvFilePath Path to the LED calibration CSV file
 * @param illuminantType 0=D50, 1=D55, 2=D65, 3=D75
 * @param decimalSeparator The decimal separator used in the CSV (e.g., ',' or '.')
 * @param colSeparator The column delimiter (e.g., ';' or ',')
 */
QVector<double> runFminconOptimization(const QString &csvFilePath, int illuminantType, 
                                     QChar decimalSeparator, QChar colSeparator,
                                     const QVector<int> &selectedLedIndices)
{
    QVector<double> outputWeights;
    
    // As seen in the MATLAB generated signature:
    // led_wls is 81x1 (81 elements)
    // led_spd is 81x22 (1782 elements, COLUMN-MAJOR)
    double led_wls[WLS_COUNT] = {0};
    double led_spd[WLS_COUNT * LED_COUNT] = {0}; // itt lehetne dinamikusan allokálva a tömb
    double weights[LED_COUNT] = {0};

    // 1. Read CSV File
    QFile file(csvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV:" << csvFilePath;
        return outputWeights;
    }

    QTextStream in(&file);
    
    // Read the header row (skip it)
    if (!in.atEnd()) {
        in.readLine(); 
    }

    // The calibration CSV contains 1nm-resolution rows starting at 300nm.
    // We only need values for 380:5:780 (81 samples). Map each CSV row
    // into the corresponding 5nm bin if its wavelength falls on that grid.
    int found = 0;
    int fileLine = 0;
    while (!in.atEnd() && found < WLS_COUNT) {
        QString line = in.readLine();
        fileLine++;
        if (line.trimmed().isEmpty())
            continue;

        QStringList tokens = line.split(colSeparator);
        if (tokens.size() < (LED_COUNT + 1)) {
            qWarning() << "CSV row" << fileLine << "does not have enough columns.";
            continue;
        }

        // Parse wavelength and map to 380:5:780 index
        QString wlStr = tokens[0].trimmed();
        if (decimalSeparator != '.') wlStr.replace(decimalSeparator, '.');
        bool ok = false;
        double wl = wlStr.toDouble(&ok);
        if (!ok)
            continue;

        if (wl < 380.0 || wl > 780.0)
            continue; // ignore wavelengths outside 380..780

        double idxd = (wl - 380.0) / 5.0;
        int idx = static_cast<int>(std::round(idxd));
        if (std::fabs(idxd - idx) > 1e-6)
            continue; // not exactly on the 5nm grid

        if (idx < 0 || idx >= WLS_COUNT)
            continue;

        // Fill wavelength and LED columns into column-major led_spd
        if (led_wls[idx] == 0.0) {
            led_wls[idx] = wl;
            found++;
        }

        for (int ledIdx = 0; ledIdx < LED_COUNT; ++ledIdx) {
            QString valStr = tokens[ledIdx + 1].trimmed();
            if (decimalSeparator != '.') valStr.replace(decimalSeparator, '.');
            int flatIndex = (ledIdx * WLS_COUNT) + idx;
            led_spd[flatIndex] = isLedSelected(ledIdx, selectedLedIndices)
                ? valStr.toDouble()
                : 0.0; // dobja ki a led oszlopokat, ha nincsenek kiválasztva
        }
    }
    file.close();

    // 2. Initialize the MATLAB runtime
    optimize_led_weights_initialize();

    // 3. Call the generated function
    optimize_led_weights(
        led_spd, 
        led_wls, 
        static_cast<double>(illuminantType), 
        weights
    );

    // 4. Terminate the MATLAB runtime
    optimize_led_weights_terminate();

    // 5. Store the weights
    for (int i = 0; i < LED_COUNT; ++i) {
        outputWeights.append(weights[i]);
    }

    qDebug() << "Optimization finished! Weights returned." << outputWeights;
             
    return outputWeights;
}

EvaluatedMetrics runMetricsEvaluation(const QVector<double> &weights, const QString &csvFilePath, 
                                      int illuminantType, QChar decimalSeparator, QChar colSeparator,
                                      const QVector<int> &selectedLedIndices)
{
    EvaluatedMetrics outputMetrics{0.0, 0.0, 0.0, 0.0, 0.0};
    
    if (weights.size() != 22) {
        qWarning() << "runMetricsEvaluation: Invalid weights size. Expected 22.";
        return outputMetrics;
    }

    double in_weights[LED_COUNT] = {0};
    double led_wls[WLS_COUNT] = {0};
    double led_spd[WLS_COUNT * LED_COUNT] = {0};

    // Copy QVector to double array
    for (int i = 0; i < LED_COUNT; ++i) {
        in_weights[i] = weights[i];
    }

    // 1. Read CSV File 
    QFile file(csvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV in runMetricsEvaluation:" << csvFilePath;
        return outputMetrics;
    }

    QTextStream in(&file);
    if (!in.atEnd()) {
        in.readLine(); 
    }

    // Map CSV rows to the 380:5:780 grid (81 samples)
    int found = 0;
    int fileLine = 0;
    while (!in.atEnd() && found < WLS_COUNT) {
        QString line = in.readLine();
        fileLine++;
        if (line.trimmed().isEmpty())
            continue;

        QStringList tokens = line.split(colSeparator);
        if (tokens.size() < (LED_COUNT + 1)) {
            qWarning() << "CSV row" << fileLine << "does not have enough columns.";
            continue;
        }

        QString wlStr = tokens[0].trimmed();
        if (decimalSeparator != '.') wlStr.replace(decimalSeparator, '.');
        bool ok = false;
        double wl = wlStr.toDouble(&ok);
        if (!ok)
            continue;

        if (wl < 380.0 || wl > 780.0)
            continue;

        double idxd = (wl - 380.0) / 5.0;
        int idx = static_cast<int>(std::round(idxd));
        if (std::fabs(idxd - idx) > 1e-6)
            continue;

        if (idx < 0 || idx >= WLS_COUNT)
            continue;

        if (led_wls[idx] == 0.0) {
            led_wls[idx] = wl;
            found++;
        }

        for (int ledIdx = 0; ledIdx < LED_COUNT; ++ledIdx) {
            QString valStr = tokens[ledIdx + 1].trimmed();
            if (decimalSeparator != '.') valStr.replace(decimalSeparator, '.');
            int flatIndex = (ledIdx * WLS_COUNT) + idx;
            led_spd[flatIndex] = isLedSelected(ledIdx, selectedLedIndices)
                ? valStr.toDouble()
                : 0.0;
        }
    }
    file.close();

    // Compute the weighted SPD in C++ (unnormalized) so the GUI can display
    // the actual spectrum resulting from applying the weights to each LED.
    double weighted_spd[WLS_COUNT] = {0};
    for (int wlIdx = 0; wlIdx < WLS_COUNT; ++wlIdx) {
        double sum = 0.0;
        for (int ledIdx = 0; ledIdx < LED_COUNT; ++ledIdx) {
            int flatIndex = (ledIdx * WLS_COUNT) + wlIdx;
            sum += in_weights[ledIdx] * led_spd[flatIndex];
        }
        weighted_spd[wlIdx] = sum;
    }

    // 2. Initialize the MATLAB runtime
    optimize_led_weights_initialize();

    double test_spd_arr[81] = {0};
    double ref_spd_arr[81] = {0};

    // 3. Call the generated evaluate function
    evaluate_metrics(
        in_weights,
        led_spd, 
        led_wls, 
        static_cast<double>(illuminantType), 
        &outputMetrics.Mu, 
        &outputMetrics.Mv, 
        &outputMetrics.Duv,
        &outputMetrics.CRI,
        &outputMetrics.CCT,
        test_spd_arr,
        ref_spd_arr
    );

    for (int i = 0; i < WLS_COUNT; ++i) {
        // Use the C++ computed, weighted (unnormalized) test SPD for plotting
        outputMetrics.testSPD.append(weighted_spd[i]);
        outputMetrics.refSPD.append(ref_spd_arr[i]);
        outputMetrics.wavelengths.append(380.0 + i * 5.0); // 380 to 780 nm in 5nm steps
    }

    // 4. Terminate the MATLAB runtime
    optimize_led_weights_terminate();

    qDebug() << "Metrics evaluation finished! Mu:" << outputMetrics.Mu << "Mv:" << outputMetrics.Mv 
             << "Duv:" << outputMetrics.Duv << "CRI:" << outputMetrics.CRI << "CCT:" << outputMetrics.CCT;
             
    return outputMetrics;
}