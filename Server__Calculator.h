#pragma once

#include<string>

struct Complx {                // структура комплексное число, с перегруженными арифметическими операциями

public:
    Complx() {           
        real = 0;
        im = 0;
    }

    Complx(float a, float b) {     
        real = a;
        im = b;
    }

    float real, im;         
    float t, e, m, p;

    Complx operator +(const Complx& X) const    
    {
        Complx result;
        result.real = real + X.real;    
        result.im = im + X.im;
        return result;
    }
    Complx operator -(const Complx& X) const    
    {
        Complx result;
        result.real = real - X.real;
        result.im = im - X.im;
        return result;
    }
    Complx operator *(const Complx& X) const {        
        Complx result;
        result.real = real * X.real - im * X.im;
        result.im = real * X.im + im * X.real;
        return result;
    }
    Complx operator / (const Complx& X) const     
    {
        Complx result;

        result.t = real * X.real;
        result.e = im * X.real;
        result.m = real * X.im * -1;
        result.p = im * X.im;       

        result.t = result.t + result.p;     
        result.e = result.e + result.m;    

        result.m = (X.real * X.real) - (X.im * X.im * -1);   

        result.real = result.t / result.m;    // äåéñòâèòåëüíàÿ ÷àñòü
        result.im = result.e / result.m;      //êîýô. ìíèìîé ÷àñòè

        return result;
    }

};

class ServerCalculator
{
public:
    ServerCalculator();
    ~ServerCalculator();

    Complx summOfCmplx(float, float, float, float);
    Complx differenceOfCmplx(float, float, float, float);
    Complx multOfCmplx(float, float, float, float);
    Complx divOfCmplx(float, float, float, float);

private:
    Complx a, b;
};

