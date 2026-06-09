#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_btnSend_clicked();

    void on_btnShowDb_clicked();

    void on_btnDbInfo_clicked();

    void on_btnBisection_clicked();

    void on_btnShortestPath_clicked();

    void on_btnEncrypt_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
