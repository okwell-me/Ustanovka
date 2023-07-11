#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QSerialPort* serial;

    QByteArray data;

    QString filename;

    uint8_t dataIn[11];
    uint8_t dataOut[4];
    uint16_t adc1, adc2, adc3, adc4;
    uint32_t linearScaleRaw;
    uint16_t currentCountOfMeasures;
    int targetCountOfMeasures;
    double volt1, volt2, volt3, volt4;
    double mm1, mm2, mm3, mm4;
    double diameter;
    double linearScaleMM;

    double sens1Offset = -4, sens2Offset = -5, sens3Offset = 2;

private slots:
    void readDataSlot();

    void on_connectButton_clicked();

    void on_refreshPortButton_clicked();

    void on_startCountOfMeasuresButton_clicked();

    void on_forwardButton_pressed();

    void on_forwardButton_released();

    void on_backwardButton_pressed();

    void on_backwardButton_released();

    void on_homeButton_clicked();


    void on_moveToPos1Button_clicked();

    void on_moveToPos2Button_clicked();

    void on_moveToPos3Button_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

/*
 * uint8_t dataOut[x];
 *  dataOut[0] --- тип команды
 *      1 --- двигатель
 *
 *  dataOut[1] смысл команды
 *      Двигатель:
 *      0 --- СТОП
 *      1 --- движение вперёд со скоростью
 *      2 --- движение назад со скоростью
 *      4 --- домой
 *
 *  dataOut[2] аргумент команды
 *
 *  dataOut[3] аргумент команды
 *
 *
 *
 */
