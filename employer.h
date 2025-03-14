#ifndef EMPLOYER_H
#define EMPLOYER_H
#include <QString>

class Emploer {
public:
    Emploer(const QString &name, const QString &gender, int age, int exp, const QString &number);
    QString Getname() const;
    QString Getgender() const;
    int Getage() const;
    int Getexp() const;
    QString Getnumber() const;

private:
    QString name;
    QString gender;
    int age;
    int exp;
    QString number;
};

#endif // EMPLOYER_H
