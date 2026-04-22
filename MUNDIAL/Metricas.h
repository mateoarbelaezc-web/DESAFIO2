#ifndef METRICAS_H
#define METRICAS_H
#include <string>

void incIter(long long n = 1);
std::string intAString(int valor, int ancho);
int parsearInt(const std::string& s, int inicio, int fin);
int stringAInt(const std::string& s);
#endif
