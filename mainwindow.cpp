#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QDataStream>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->Number->setInputMask("+7(000)000-00-00");
    ui->Number0_255->setRange(0,255);
    ui->Age->setRange(0,120);
    ui->Exp->setRange(0,100);
    connect(ui->addEmp, &QPushButton::clicked, this, &MainWindow::addEmployer);
    connect(ui->RemoveEmp, &QPushButton::clicked, this, &MainWindow::removeEmployer);
    connect(ui->EmpList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onEmployerSelected);
    connect(ui->WriteToFile, &QPushButton::clicked, this, &MainWindow::saveToFile);
    connect(ui->CheckFile, &QPushButton::clicked, this, &MainWindow::checkFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addEmployer() {
    QString name = ui->Name->text();
    QString gender = ui->Gender->currentText();
    int age = ui->Age->text().toInt();
    int exp = ui->Exp->text().toInt();
    QString phone = ui->Number->text();
    if (name.isEmpty()|| phone.contains("_")) {
        QMessageBox::warning(this, "Error", "Enter correct info!");
    }

    rabotniki.append(QSharedPointer<Emploer>::create(name, gender, age, exp, phone));
    updateList();
}


void MainWindow::removeEmployer() {
    int ind = ui->EmpList->currentIndex();
    if (ind>=0) {
    rabotniki.removeAt(ind);
    updateList();
    if (!rabotniki.isEmpty()) {
        ui->EmpList->setCurrentIndex(0);
        onEmployerSelected(0);
    } else {
        ui->Name->clear();
        ui->Gender->setCurrentIndex(-1);
        ui->Age->setValue(0);
        ui->Exp->setValue(0);
        ui->Number->clear();
    }
}
}

void MainWindow::updateList() {
    ui->EmpList->clear();
    for (const auto &emp : rabotniki) {
        ui->EmpList->addItem(emp->Getname());
    }
        if (!rabotniki.isEmpty()) {
            ui->EmpList->setCurrentIndex(0);
            onEmployerSelected(0);
        }
}

void MainWindow::onEmployerSelected(int ind) {
   if (ind>=0 &&ind <rabotniki.size()) {
       auto rab = rabotniki[ind];
       ui->Name->setText(rab->Getname());
       ui->Age->setValue(rab->Getage());
       ui->Gender->setCurrentText(rab->Getgender());
       ui->Exp->setValue(rab->Getexp());
       ui->Number->setText(rab->Getnumber());
   }
}

unsigned int CRC32_function(const QByteArray &data) {
    unsigned long crc_table[256];
    unsigned long crc;
    for (int i = 0; i < 256; i++) {
        crc = i;
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
        crc_table[i] = crc;
    }
    crc = 0xFFFFFFFFUL;
    for (uchar byte : data)
        crc = crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFUL;
}

void MainWindow::saveToFile() {
    int number = ui->Number0_255->value();
    QString FileName = QFileDialog::getSaveFileName(this, "save to file","", "Rabotmik Files (*.emp)" );
    if (FileName.isEmpty()) return;
    QFile file(FileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "cannot open the file");
        return;
    }

    QByteArray allData;
    int size = rabotniki.size();
    allData.append(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto &rab : rabotniki) {
        QByteArray data;
        data.append(rab->Getname().toUtf8() + '\0');
        data.append(rab->Getgender().toUtf8() + '\0');
        data.append(QString::number(rab->Getage()).toUtf8() + '\0');
        data.append(QString::number(rab->Getexp()).toUtf8() + '\0');
        data.append(rab->Getnumber().toUtf8() + '\0');

        for (char &byte : data) byte ^= number;

        int length = data.size();
        allData.append(reinterpret_cast<const char*>(&length), sizeof(length));
        allData.append(data);
    }

    file.write(allData);
    file.close();

    unsigned int crc = CRC32_function(allData);
    ui->ControlSum->setText(QString("CRC32: %1").arg(crc, 8, 16, QChar('0')).toUpper());
    QFile crcFile(FileName + ".crc");
    if (crcFile.open(QIODevice::WriteOnly)) {
        crcFile.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
        crcFile.close();
    }

}

void MainWindow::checkFile() {
 QString fileName =  QFileDialog::getOpenFileName(this, "Open File", "", "Rabotmik Files (*.emp)");
 if (fileName.isEmpty()) return;

 QFile file(fileName);
 if (!file.open(QIODevice::ReadOnly)) {
     QMessageBox::warning(this, "Error", "cannot open the file");
     return;
 }

 QByteArray Data = file.readAll();
 file.close();

 QFile crcfile(fileName+".crc");
 if (!crcfile.open(QIODevice::ReadOnly)) {
     QMessageBox::warning(this, "Error", "cannot open crc file");
     return;
 }

 unsigned int CRCFromFile;
 crcfile.read(reinterpret_cast<char*>(&CRCFromFile), sizeof(CRCFromFile));
 crcfile.close();

 unsigned int CRC = CRC32_function(Data);
 if (CRCFromFile!=CRC) {
     QMessageBox::warning(this, "Error", "CRC is not correct");
     return;
 }

 int number = ui->Number0_255->value();
 int index = 0;
 rabotniki.clear();

 int size;
 memcpy(&size, Data.constData(), sizeof(size));
 index+=sizeof(size);


 for (int i = 0; i < size; i++) {
     int length;
     memcpy(&length, Data.constData() + index, sizeof(length));
     index += sizeof(length);

     QByteArray newdata = Data.mid(index, length);
     index += length;
     for (char &byte : newdata) byte ^= number;

     QList<QByteArray> parts = newdata.split('\0');
     if (parts.size() < 5) {
         QMessageBox::warning(this, "Ошибка", "Ошибка структуры данных!");
         return;
     }

 rabotniki.push_back(QSharedPointer<Emploer>::create(parts[0], parts[1], parts[2].toInt(), parts[3].toInt(), parts[4]));
 }
 updateList();
 ui->ControlSum->setText(QString("CRC32: %1").arg(CRCFromFile, 8, 16, QChar('0')).toUpper());
      QMessageBox::warning(this, "Message", "CRC is correct");

}

void MainWindow::closeEvent(QCloseEvent *event) {
    deleteFiles();
    event->accept();
}

void MainWindow::deleteFiles() {
    QDir dir(QDir::currentPath());
    QStringList rashirenie;
    rashirenie <<"*.emp" << "*.crc";

    QStringList files = dir.entryList(rashirenie, QDir::Files);
    for (const QString &file :files) {
        dir.remove(file);
    }
}





