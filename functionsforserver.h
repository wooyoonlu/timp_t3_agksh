#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>

QString vigenereEncrypt(const QString& text, const QString& key);
QString vigenereDecrypt(const QString& cipher, const QString& key);
QString sha512(const QString& input);
double bisection(double a, double b, double eps);
int shortestPath(const QString& graphDescription, int start, int end);

#endif
