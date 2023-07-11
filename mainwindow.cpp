#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);
    connect(serial, SIGNAL(readyRead()), this, SLOT(readDataSlot()));

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->portBox->addItem(serialPortInfo.portName());
    }

    ui->motor1SpeedComboBox->addItem("100");
    ui->motor1SpeedComboBox->addItem("1000");
    ui->motor1SpeedComboBox->addItem("10000");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readDataSlot()
{
    if (serial->bytesAvailable()==11) {
        data = serial->readAll();
        serial->clear();

        for (int i = 0; i < 12; i++){
            dataIn[i] = data[i] & 0xFF;
        }

        if (dataIn[0] == 3){
            adc1 = dataIn[1] * 256 + dataIn[2];
            adc2 = dataIn[3] * 256 + dataIn[4];
            adc3 = dataIn[5] * 256 + dataIn[6];
            adc4 = dataIn[7] * 256 + dataIn[8];

            volt1 = adc1 * 0.1875 / 1000;
            volt2 = adc2 * 0.1875 / 1000;
            volt3 = adc3 * 0.1875 / 1000;
            volt4 = adc4 * 0.1875 / 1000;

            mm1 = 65 + (5-volt1) * 14 + sens1Offset;
            mm2 = 65 + (5-volt2) * 14 + sens2Offset;
            mm3 = 65 + (5-volt3) * 14 + sens3Offset;
            mm4 = 30 + (5-volt4) * 2;

            double r1 = 140.5 - mm1;
            double r2 = 140.5 - mm2;
            double r3 = 140.5 - mm3;

            linearScaleRaw = dataIn[9]*256 + dataIn[10];
            linearScaleMM = linearScaleRaw * 0.02;
            ui->linearScaleDataLCD->display(linearScaleMM);

            diameter = (r1+r2+r3)/3*2; //------------------------------------заменить формулу


            if (volt1 > 5.1) {
                ui->sensor1DataLCD->display("----");
                diameter = 0;
            } else ui->sensor1DataLCD->display(r1);
            if (volt2 > 5.1) {
                ui->sensor2DataLCD->display("----");
                diameter = 0;
            } else ui->sensor2DataLCD->display(r2);
            if (volt3 > 5.1) {
                ui->sensor3DataLCD->display("----");
                diameter = 0;
            } else ui->sensor3DataLCD->display(r3);
            /*if (volt4 > 5.1) {
                ui->sensor4DataLCD->display("----");
            } else ui->sensor4DataLCD->display(mm4);*/

            if (diameter == 0) ui->diameterDataLCD->display("----");
            else ui->diameterDataLCD->display(diameter);

            if (currentCountOfMeasures < targetCountOfMeasures || targetCountOfMeasures == -1) {
                if (targetCountOfMeasures > 0){
                currentCountOfMeasures++;
                ui->countOfMeasuresProgressBar->setValue((currentCountOfMeasures*100/targetCountOfMeasures));
                }

                QFile file(filename + ".csv");
                file.open(QIODevice::Append);
                QString log = QTime::currentTime().toString("hh:mm:ss") + ";" + QString::number(currentCountOfMeasures) + ";" +
                        QString::number(adc1) + ";" + QString::number(mm1) + ";" +
                        QString::number(adc2) + ";" + QString::number(mm2) + ";" +
                        QString::number(adc3) + ";" + QString::number(mm3) + ";" +
                        QString::number(diameter) + QString::number(linearScaleMM) + "\n";
                log.replace(".", ",");
                file.write(log.toLocal8Bit());
            }
        }
    }
}

void MainWindow::on_connectButton_clicked()
{
    serial->setPortName(ui->portBox->currentText());
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setParity(QSerialPort::EvenParity);

    if (!serial->open(QIODevice::ReadWrite)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось подключится к порту");
        return;
    }
    serial->clear();
}


void MainWindow::on_refreshPortButton_clicked()
{
    ui->portBox->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->portBox->addItem(serialPortInfo.portName());
    }
}

void MainWindow::on_startCountOfMeasuresButton_clicked()
{
    targetCountOfMeasures = ui->countOfMesasuresLineEdit->text().toInt();
    currentCountOfMeasures = 0;
    QString text = QString::number(targetCountOfMeasures) + " measures";

    filename = ui->fileNameLineEdit->text();
    if (filename == "") filename = "log";

    QFile log(filename + ".csv");
    log.open(QIODevice::Append);
    log.write("/---;---;---;---;---/;" + text.toLocal8Bit() + ";/---;---;---;---;---/;\n");
    log.write("Time;Number;D1-adc;D1-mm;D2-adc;D2-mm;D3-adc;D3-mm;Diameter;LinearScale\n");
}


void MainWindow::on_forwardButton_pressed()
{
    dataOut[0] = 1;
    dataOut[1] = 1;
    uint16_t spd = ui->motor1SpeedComboBox->currentText().toInt();
    dataOut[2] = spd >> 8;
    dataOut[3] = spd;
    serial->write((char*)dataOut, 4);
    //send comand to move forward w/speed of motor1SpeedComboBox
}


void MainWindow::on_forwardButton_released()
{
    dataOut[0] = 1;
    dataOut[1] = 0;
    dataOut[2] = 0;
    dataOut[3] = 0;
    serial->write((char*)dataOut, 4);
    //send comand to stop
}


void MainWindow::on_backwardButton_pressed()
{
    dataOut[0] = 1;
    dataOut[1] = 2;
    uint16_t spd = ui->motor1SpeedComboBox->currentText().toInt();
    dataOut[2] = spd >> 8;
    dataOut[3] = spd;
    serial->write((char*)dataOut, 4);
    //send comand to move backward w/speed of motor1SpeedComboBox
}


void MainWindow::on_backwardButton_released()
{
    dataOut[0] = 1;
    dataOut[1] = 0;
    dataOut[2] = 0;
    dataOut[3] = 0;
    serial->write((char*)dataOut, 4);
    //send comand to stop
}


void MainWindow::on_homeButton_clicked()
{
    dataOut[0] = 1;
    dataOut[1] = 4;
    dataOut[2] = 0;
    dataOut[3] = 0;
    serial->write((char*)dataOut, 4);
    //send comand to move backward until KONCEVIK
}


void MainWindow::on_moveToPos1Button_clicked()
{
    dataOut[0] = 1;
    dataOut[1] = 8;

    //200mm from home
    uint16_t targetPos = 200*50;

    dataOut[2] = targetPos >> 8;
    dataOut[3] = targetPos;
    serial->write((char*)dataOut, 4);
}


void MainWindow::on_moveToPos2Button_clicked()
{
    dataOut[0] = 1;
    dataOut[1] = 8;

    //400mm from home
    uint16_t targetPos = 400*50;

    dataOut[2] = targetPos >> 8;
    dataOut[3] = targetPos;
    serial->write((char*)dataOut, 4);
}


void MainWindow::on_moveToPos3Button_clicked()
{
    dataOut[0] = 1;
    dataOut[1] = 8;

    //600mm from home
    uint16_t targetPos = 600*50;

    dataOut[2] = targetPos >> 8;
    dataOut[3] = targetPos;
    serial->write((char*)dataOut, 4);
}

