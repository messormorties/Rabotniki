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
      // QString age = QString::number(rab->Getage());
       //QString exp = QString::number(rab->Getexp());
       ui->Age->setValue(rab->Getage());
       ui->Gender->setCurrentText(rab->Getgender());
       ui->Exp->setValue(rab->Getexp());
       ui->Number->setText(rab->Getnumber());
   }
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

    QDataStream out(&file);
    out<<rabotniki.size();
    QByteArray fileData;
    for (const auto rab : rabotniki) {
        QString name = rab->Getname();
        QString gender = rab->Getgender();
        int age = rab->Getage();
        int experience = rab->Getexp();
        QString phone = rab->Getnumber();

        QByteArray XorData;
        QDataStream tempStream(&XorData, QIODevice::WriteOnly);
        tempStream << name << gender << age << experience << phone;
        for (char &byte : XorData) {
            byte ^=number;
        }
        out<<XorData;
        fileData.append(XorData);
    }

    file.close();

    QFile readFile(FileName);
    if (!readFile.open(QIODevice::ReadOnly)) return;
    qint32 summa = 0;
    QByteArray data = readFile.readAll();

    for (char byte : data) {
        summa+=static_cast<unsigned char>(byte);
    }
    readFile.close();

    QFile crc(FileName+".crc");
    if (crc.open(QIODevice::WriteOnly)) {
        QDataStream crcOut(&crc);
        crcOut<<summa;
        crc.close();
    }
    ui->ControlSum->setText(QString::number(summa));

}

void MainWindow::checkFile() {
 QString fileName =  QFileDialog::getOpenFileName(this, "Open File", "", "Rabotmik Files (*.emp)");
 if (fileName.isEmpty()) return;

 QFile file(fileName);
 if (!file.open(QIODevice::ReadOnly)) {
     QMessageBox::warning(this, "Error", "cannot open the file");
     return;
 }

 QFile crcfile(fileName+".crc");
 if (!crcfile.open(QIODevice::ReadOnly)) {
     QMessageBox::warning(this, "Error", "cannot open crc file");
     return;
 }

 quint32 checkSumma = 0;
 QByteArray data = file.readAll();
 for (char byte : data) {
     checkSumma +=static_cast<unsigned char>(byte);
 }
 file.seek(0);

 QDataStream in(&file);
 rabotniki.clear();
 int k;
 in>>k;
 int number = ui->Number0_255->value();

 QDataStream crcIn(&crcfile);
 quint32 storedChecksum;
 crcIn >> storedChecksum;
 crcfile.close();

 if (checkSumma == storedChecksum) {
     QMessageBox::information(this, "Sucsess", "Control sum is correct");
 } else {
     QMessageBox::warning(this, "Erorr", "Control sum is not correct");
 }


 for (int i=0;i<k;++i) {
     QByteArray XorData;
     in>>XorData;

     for (char &byte : XorData) {
         byte ^=number;
     }
     QDataStream tempStream(&XorData, QIODevice::ReadOnly);
     QString name, gender, phone;
     int age, exp;
     tempStream >> name >> gender >> age >> exp >> phone;
     rabotniki.append(QSharedPointer<Emploer>::create(name, gender, age, exp, phone));
 }
 file.close();
 updateList();
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





