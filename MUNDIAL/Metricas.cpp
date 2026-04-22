#include "Metricas.h"
#include "Torneo.h"

void incIter(long long n) {
    Torneo::incIteraciones(n);
}

std::string intAString(int valor, int ancho) {
    char digits[10];
    int len = 0;
    if (valor == 0) {
        digits[len++] = '0';
    } else {
        int tmp = valor;
        while (tmp > 0) {
            digits[len++] = '0' + (tmp % 10);
            tmp /= 10;
        }
        for (int i = 0; i < len/2; ++i) {
            char t = digits[i];
            digits[i] = digits[len-1-i];
            digits[len-1-i] = t;
        }
    }
    std::string resultado = "";
    for (int i = len; i < ancho; ++i) resultado += '0';
    for (int i = 0; i < len; ++i) resultado += digits[i];
    return resultado;
}

int parsearInt(const std::string& s, int inicio, int fin) {
    int resultado = 0;
    for (int i = inicio; i < fin; ++i)
        resultado = resultado * 10 + (s[i] - '0');
    return resultado;
}

int stringAInt(const std::string& s) {
    int resultado = 0;
    int i = 0;
    bool negativo = false;
    if (i < (int)s.size() && s[i] == '-') { negativo = true; i++; }
    for (; i < (int)s.size(); ++i) {
        if (s[i] >= '0' && s[i] <= '9')
            resultado = resultado * 10 + (s[i] - '0');
    }
    return negativo ? -resultado : resultado;
}
