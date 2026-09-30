#include "cie1931pagewidget.h"
#include "ui_cie1931pagewidget.h"
#include <QPixmap>
#include <QPainter>
#include <cmath>

Cie1931PageWidget::Cie1931PageWidget(const QString& imagesFolderAbsolutePath, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Cie1931PageWidget)
    , imagesFolderAbsolutePath(imagesFolderAbsolutePath)
    , seriesCIE(new QSplineSeries(this))
    , chartCIE(new QChart())
    , chartViewCIE(new QChartView(chartCIE, this))
{
    ui->setupUi(this);

    // Initialize Chart
    seriesCIE->clear();

    double alpha = 0.05;
    int k_max = 100;
    for (int k = 0; k <= k_max; ++k) {
        double exp_loss = std::exp(-alpha * k);
        seriesCIE->append(k, exp_loss);
    }

    chartCIE->setTitle("CIE 1931 loss function");
    chartCIE->legend()->hide();
    chartCIE->addSeries(seriesCIE);
    chartCIE->createDefaultAxes();
    
    auto axesH = chartCIE->axes(Qt::Horizontal);
    if (!axesH.isEmpty()) {
        axesH.first()->setRange(0, 100);
        axesH.first()->setTitleText("Iteration number [step]");
    }
    
    auto axesV = chartCIE->axes(Qt::Vertical);
    if (!axesV.isEmpty()) {
        axesV.first()->setRange(0, 1);
        axesV.first()->setTitleText("Normalized loss function [-]");
    }

    chartCIE->setVisible(true);

    chartViewCIE->setRenderHint(QPainter::Antialiasing);
    chartViewCIE->setVisible(true);
    chartViewCIE->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->gridLayoutPageCIE1931->addWidget(chartViewCIE, 0, 0);

    // Initialize Image View
    labelDisplayCie1931Diagram = new QLabel("");
    imageCie1931Diagram = new QImage(imagesFolderAbsolutePath + "/cie1931-color-diagram.jpg");
    labelDisplayCie1931Diagram->setPixmap(QPixmap::fromImage(*imageCie1931Diagram));
    labelDisplayCie1931Diagram->setScaledContents(true);

    ui->gridLayoutPageCIE1931_2->addWidget(labelDisplayCie1931Diagram, 0, 0, -1, -1, Qt::AlignHCenter);

    connect(ui->pushButton, &QPushButton::clicked, this, &Cie1931PageWidget::triggerCalculate);
}

Cie1931PageWidget::~Cie1931PageWidget()
{
    delete imageCie1931Diagram;
    delete ui;
}

void Cie1931PageWidget::triggerCalculate()
{
    //emit logMessage("CIE1931", "Running MATLAB Fmincon Algorithm (Mocked/Disabled)");
    //fmincon->runMatlabAlgorithm(); 
    // Depending on fmincon behavior, we may need to emit requestNetworkSend() if it updates LEDs

    // 1. Get inputs from your UI (e.g., file dialog, combo box)
    QString csvPath = QCoreApplication::applicationDirPath() + "/configs/LED_SPD_CALIBRATION.csv";
    int illuminant = 0; // Placeholder: replace with actual value from UI, e.g., ui->illuminantComboBox->currentIndex();
    //int illuminant = ui->illuminantComboBox->currentIndex(); // e.g., 0 for D50

    // 2. Call the runner and get the weights
    QVector<double> weights = runFminconOptimization(csvPath, illuminant, ',', ';');

    // 3. Use the weights to populate a table or plot the weights
    for (int i = 0; i < weights.size(); ++i) {
        qDebug() << "LED" << i << "optimal weight:" << weights[i];
        // e.g., ui->weightsTableWidget->setItem(i, 0, new QTableWidgetItem(...));
    }

}
