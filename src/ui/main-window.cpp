#include <QtGui>
#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>

#include "QtAwesome.h"
#include "cloud-view.h"
#include "seafile-applet.h"
#include "configurator.h"
#include "tray-icon.h"
#include "login-dialog.h"
#include "utils/utils.h"

#include "main-window.h"

namespace {

enum WIDGET_INDEX {
    INDEX_CLOUD_VIEW = 0,
    INDEX_LOCAL_VIEW
};

} // namespace


MainWindow::MainWindow()
{
    setWindowIcon(QIcon(":/images/seafile.png"));
    setWindowTitle(getBrand());

    // Qt::Tool hides the taskbar entry on windows
    // setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

    setWindowFlags(Qt::Window
                   | Qt::FramelessWindowHint
                   | Qt::WindowSystemMenuHint
                   | Qt::WindowMinimizeButtonHint);

    cloud_view_ = new CloudView;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(cloud_view_);

    QWidget *wrapper = new QWidget;
    wrapper->setObjectName("mainWrapper");
    wrapper->setLayout(layout);

    setCentralWidget(wrapper);

    createActions();
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void MainWindow::hide()
{
    writeSettings();
    QMainWindow::hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

bool MainWindow::event(QEvent *ev)
{
    bool ret = QMainWindow::event(ev);

    if (isMinimized() && ev->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent *wev = (QWindowStateChangeEvent *)ev;
        if (wev->oldState() != Qt::WindowMinimized) {
            writeSettings();
        }
    }
    return ret;
}

void MainWindow::changeEvent(QEvent *event)
{
#ifdef Q_WS_WIN
    /*
     * Solve the problem of restoring a minimized frameless window on Windows
     * See http://stackoverflow.com/questions/18614661/how-to-not-hide-taskbar-item-during-using-hide
     */
    if(event->type() == QEvent::WindowStateChange) {
        if(windowState() & Qt::WindowMinimized ) {
            //do something after minimize
        } else {
            cloud_view_->hide();
            cloud_view_->show();
        }
    }
#endif
}

void MainWindow::showEvent(QShowEvent *event)
{
    readSettings();
    QMainWindow::showEvent(event);
}

void MainWindow::createActions()
{
    refresh_qss_action_ = new QAction(QIcon(":/images/refresh.png"), tr("Refresh"), this);
    connect(refresh_qss_action_, SIGNAL(triggered()), this, SLOT(refreshQss()));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F5) {
        refreshQss();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::showWindow()
{
    showNormal();
    show();
    raise();
    activateWindow();
}

void MainWindow::refreshQss()
{
    seafApplet->refreshQss();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

QPoint MainWindow::getDefaultPosition()
{
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QPoint top_right = screen.topRight();

    int top_margin = rect().width() + qMin(150, (int)(0.1 * screen.width()));
    int right_margin = qMin(150, (int)(0.1 * screen.width()));
    QPoint default_pos(top_right.x() -top_margin, top_right.y() + right_margin);

    return default_pos;
}

void MainWindow::readSettings()
{
    QPoint pos;
    QSize size;
    QSettings settings;
    settings.beginGroup("MainWindow");

    static bool first_show = true;

    if (first_show && seafApplet->configurator()->firstUse()) {
        pos = getDefaultPosition();
    } else {
        pos = settings.value("pos", getDefaultPosition()).toPoint();
        size = settings.value("size", QSize()).toSize();
    }

    first_show = false;

    move(pos);
    resize(size);
}
