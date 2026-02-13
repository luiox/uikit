/* 
	QQ:1517807724	
	这是一个复数计算的类
	Version:1.0
*/
//ComplexNumber.h
#ifndef __COMPLEXNUMBER_H__
#define __COMPLEXNUMBER_H__

#pragma once

/*  dll export  */
#ifdef COMPLEXNUMBER_EXPORT
#define COMPLEXNUMBER_API __declspec(dllexport)
#else
#define COMPLEXNUMBER_API __declspec(dllimport)
#endif

#define COMPLEXNUMBER_ERROR_CANT_COMPARE -1
#define COMPLEXNUMBER_TRUE 1
#define COMPLEXNUMBER_FLASE 0
#define COMPLEXNUMBER_NULL nullptr

COMPLEXNUMBER_API typedef	struct tagCpxNum
{
	double realPart;//实部
	double imaginaryPart;//虚部
}CpxNum, *LPCpxNum;

class COMPLEXNUMBER_API ComplexNumber
{
public:
	ComplexNumber();
	ComplexNumber(LPCpxNum number);
	~ComplexNumber();
	void clear();//清空
	void destory();//销毁
	int setNum(LPCpxNum number);//设置复数
	int setNum(double realPart, double imaginaryPart);
	LPCpxNum getThis() const;//获取m_number
	/*下面这些函数如果m_number为COMPLEXNUMBER_NULL以后调用会返回COMPLEXNUMBER_FLASE*/
	int add(LPCpxNum number);//加
	int add(double realPart, double imaginaryPart);
	int minus(LPCpxNum number);//减		默认m_number-number
	int minus(double realPart, double imaginaryPart);
	int compare(LPCpxNum number);//比较		若number大于等于m_number返回COMPLEXNUMBER_TRUE否则COMPLEXNUMBER_FLASE
	int compare(double realPart, double imaginaryPart);
	int multiply(LPCpxNum number);//乘
	int multiply(double realPart, double imaginaryPart);
	int divide(LPCpxNum number);//除
	int divide(double realPart, double imaginaryPart);
	/*运算符重载*/
	//LPCpxNum operator+(LPCpxNum number);
	//LPCpxNum operator-(LPCpxNum number);
	//LPCpxNum operator*(LPCpxNum number);
	//LPCpxNum operator/(LPCpxNum number);
	//LPCpxNum operator+=(LPCpxNum number);
	//LPCpxNum operator-=(LPCpxNum number);
	//LPCpxNum operator*=(LPCpxNum number);
	//LPCpxNum operator/=(LPCpxNum number);
	int operator==(LPCpxNum number);
	int operator!=(LPCpxNum number);

private:
	LPCpxNum m_number;
};

#endif // __COMPLEXNUMBER_H__