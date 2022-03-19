#include "ServerCalculator.h"

ServerCalculator::ServerCalculator()
{

}

ServerCalculator::~ServerCalculator()
{

}



Complx ServerCalculator::summOfCmplx(float real1, float im1, float real2, float im2)
{
	a.real = real1;
	a.im = im1;
	b.real = real2;
	b.im = im2;
	return a + b;
}

Complx ServerCalculator::differenceOfCmplx(float real1, float im1, float real2, float im2)
{
	a.real = real1;
	a.im = im1;
	b.real = real2;
	b.im = im2;
	return a - b;
}

Complx ServerCalculator::multOfCmplx(float real1, float im1, float real2, float im2)
{
	a.real = real1;
	a.im = im1;
	b.real = real2;
	b.im = im2;
	return a * b;
}

Complx ServerCalculator::divOfCmplx(float real1, float im1, float real2, float im2)
{
	a.real = real1;
	a.im = im1;
	b.real = real2;
	b.im = im2;
	return a / b;
}