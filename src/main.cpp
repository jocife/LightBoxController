#include "lightboxcontroller.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + "/images/led-gui.ico"));

    LightBoxController w;
    QFile styleSheetFile(w.getThemeFileAbsolutePath());
    if (!styleSheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while loading style sheet file | Error: " << styleSheetFile.error();
    }
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    a.setStyleSheet(styleSheet);
    // a.setStyleSheet(""); // Use default Qt style for better native look

    w.show();
    return a.exec();
}
