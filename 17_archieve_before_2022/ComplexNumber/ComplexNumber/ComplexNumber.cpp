//ComplexNumber.cpp

#include "ComplexNumber.h"


ComplexNumber::ComplexNumber()
{
	m_number = new CpxNum;
	m_number->realPart = 0;
	m_number->imaginaryPart = 0;
}

ComplexNumber::ComplexNumber(LPCpxNum number) :m_number(number)
{

}

ComplexNumber::~ComplexNumber()
{
	delete m_number;
	m_number = COMPLEXNUMBER_NULL;
}

//清空
void ComplexNumber::clear()
{
	m_number->realPart = 0;
	m_number->imaginaryPart = 0;
}

//销毁
void ComplexNumber::destory()
{
	delete m_number;
	m_number = COMPLEXNUMBER_NULL;
}

//设置复数
int ComplexNumber::setNum(LPCpxNum number)
{
	if (m_number == COMPLEXNUMBER_NULL)
		m_number = new CpxNum;

	m_number->realPart = number->realPart;
	m_number->imaginaryPart = number->imaginaryPart;
	return COMPLEXNUMBER_TRUE;
}

int ComplexNumber::setNum(double realPart, double imaginaryPart)
{
	if (m_number == COMPLEXNUMBER_NULL)
		m_number = new CpxNum;

	m_number->realPart = realPart;
	m_number->imaginaryPart = imaginaryPart;
	return COMPLEXNUMBER_TRUE;
}

//获取m_number
LPCpxNum ComplexNumber::getThis() const
{
	return m_number;
}

//加
int ComplexNumber::add(LPCpxNum number)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart += number->realPart;
	m_number->imaginaryPart += number->imaginaryPart;
	return COMPLEXNUMBER_TRUE;
}

int ComplexNumber::add(double realPart, double imaginaryPart)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart += realPart;
	m_number->imaginaryPart += imaginaryPart;
	return COMPLEXNUMBER_TRUE;

}

//减		默认m_number-number
int ComplexNumber::minus(LPCpxNum number)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart -= number->realPart;
	m_number->imaginaryPart -= number->imaginaryPart;
	return COMPLEXNUMBER_TRUE;
}

int ComplexNumber::minus(double realPart, double imaginaryPart)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart += realPart;
	m_number->imaginaryPart += imaginaryPart;
	return COMPLEXNUMBER_TRUE;
}

//比较		若number比m_number返回true否则false
int ComplexNumber::compare(LPCpxNum number)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	if ((m_number->imaginaryPart != 0) || (number->imaginaryPart != 0))
		return COMPLEXNUMBER_ERROR_CANT_COMPARE;

	if (m_number->realPart >= number->realPart)
		return COMPLEXNUMBER_TRUE;
	else
		return COMPLEXNUMBER_FLASE;
}

int ComplexNumber::compare(double realPart, double imaginaryPart)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	if ((m_number->imaginaryPart != 0) || (imaginaryPart != 0))
		return COMPLEXNUMBER_ERROR_CANT_COMPARE;

	if (m_number->realPart >= realPart)
		return COMPLEXNUMBER_TRUE;
	else
		return COMPLEXNUMBER_FLASE;
}

//乘
int ComplexNumber::multiply(LPCpxNum number)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart = (m_number->realPart*number->imaginaryPart) - (m_number->imaginaryPart*number->realPart);
	m_number->imaginaryPart = (m_number->imaginaryPart*number->realPart) + (m_number->realPart*number->imaginaryPart);
	return COMPLEXNUMBER_TRUE;
}

int ComplexNumber::multiply(double realPart, double imaginaryPart)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart = (m_number->realPart*imaginaryPart) - (m_number->imaginaryPart*realPart);
	m_number->imaginaryPart = (m_number->imaginaryPart*realPart) + (m_number->realPart*imaginaryPart);
	return COMPLEXNUMBER_TRUE;
}

//除
int ComplexNumber::divide(LPCpxNum number)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart = ((m_number->imaginaryPart*number->imaginaryPart) + (m_number->realPart*number->realPart)) / (number->realPart *number->realPart + m_number->realPart*m_number->realPart);
	m_number->imaginaryPart = ((m_number->imaginaryPart*number->realPart) - (m_number->realPart*number->imaginaryPart)) / (number->realPart *number->realPart + m_number->realPart*m_number->realPart);
	return COMPLEXNUMBER_TRUE;
}

int ComplexNumber::divide(double realPart, double imaginaryPart)
{
	if (m_number == COMPLEXNUMBER_NULL)
		return COMPLEXNUMBER_FLASE;

	m_number->realPart = ((m_number->imaginaryPart*imaginaryPart) + (m_number->realPart*realPart)) / (realPart *realPart + m_number->realPart*m_number->realPart);
	m_number->imaginaryPart = ((m_number->imaginaryPart*realPart) - (m_number->realPart*imaginaryPart)) / (realPart *realPart + m_number->realPart*m_number->realPart);
	return COMPLEXNUMBER_TRUE;
}

/*运算符重载*/
//LPCpxNum ComplexNumber::operator+(LPCpxNum number)
//{
//	if (add(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator-(LPCpxNum number)
//{
//	if (minus(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator*(LPCpxNum number)
//{
//	if (multiply(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator/(LPCpxNum number)
//{
//	if (divide(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator+=(LPCpxNum number)
//{
//	if (add(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator-=(LPCpxNum number)
//{
//	if (add(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator*=(LPCpxNum number)
//{
//	if (add(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}
//
//LPCpxNum ComplexNumber::operator/=(LPCpxNum number)
//{
//	if (add(number) == COMPLEXNUMBER_FLASE)
//		return COMPLEXNUMBER_NULL;
//	return m_number;
//}

int ComplexNumber::operator==(LPCpxNum number)
{
	return compare(number);
}

int ComplexNumber::operator!=(LPCpxNum number)
{
	return compare(number) ? COMPLEXNUMBER_FLASE : COMPLEXNUMBER_TRUE;
}