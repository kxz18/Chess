#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QByteArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTimer>
#include "parser.h"
#define TURNTIME 60

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionServer_triggered();

    void on_actionClient_triggered();

    void on_btn_quit_clicked();

    void acceptConnection();

    void getConnected();

    void getCommand();

    void sendCommand(QStringList command);

    void timeCount();

    void on_actionExit_triggered();

    void clientDisconnect();

    void on_actionDisconnect_triggered();

    void on_btn_reset_clicked();

    void slt_gameOver();

    void on_btn_forfeit_clicked();

    void on_btn_castling_clicked();

    void on_actionSave_ending_triggered();

    void on_actionLoad_ending_triggered();

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QTcpSocket *client;
    QTimer *timer;
};

#endif // MAINWINDOW_H
