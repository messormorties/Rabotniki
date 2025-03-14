#include "employer.h"

Emploer::Emploer(const QString& name, const QString& gender, int age, int exp, const QString& number)
    : name(name), gender(gender), age(age), exp(exp), number(number) {}

QString Emploer::Getname() const { return name; }
QString Emploer::Getgender() const { return gender; }
int Emploer::Getage() const { return age; }
int Emploer::Getexp() const { return exp; }
QString Emploer::Getnumber() const { return number; }
