/****************************************************************************
**
**
**
****************************************************************************/

#include "mainwindow/mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    // TODO uncomment this line before release
    // right now we always need it
    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

    QApplication app(argc, argv);
    QApplication::setOrganizationName("ZJU");
    QApplication::setOrganizationDomain("https:");
    QApplication::setApplicationName("SuperConsole");
    QApplication::setWindowIcon(QIcon(":/images/skin/images/ico.ico"));

    MainWindow *main = MainWindow::getInstance();
    main->setAttribute(Qt::WA_DeleteOnClose);
    main->show();
    return app.exec();
}
