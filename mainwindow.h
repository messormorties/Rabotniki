#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QSharedPointer>
#include <employer.h>
#include <QDir>
#include <QCloseEvent>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void addEmployer();
    void removeEmployer();
    void updateList();
    void onEmployerSelected(int ind);
    void saveToFile();
    void checkFile();

private:
    Ui::MainWindow *ui;
    QList<QSharedPointer<Emploer>> rabotniki;
    void deleteFiles();
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
