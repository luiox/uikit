//1>字符串拼接
//2>字符串翻转
//3>字符串比较
//3>求字符串长度
//4>可以用一个对象对另一个对象的初始化(拷贝构造问题)
#pragma once
#include<iostream>
#include<string.h>
using namespace std;
class MyString
{
public:
	MyString(int size);
	MyString(const char*c_str);
	MyString(const MyString&other);
	~MyString();
	//获取字符串
	const char* data();
	//字符串拼接	
	const char* MyStrcat(const MyString& other);
	const char* MyStrcat(const char* str);
	//字符串翻转
	const char* MyReverse();
	//字符串拷贝
	const char* MyStrcpy(const MyString& other);
	//字符串比较
	bool MyStrcmp(const MyString& other);
public:
	//重载+
	MyString operator+=(MyString& other);
	//重载=
	MyString operator=(MyString& other);
	//重载<<
	friend ostream& operator<<(ostream& out, MyString& other);
	//重载==
	bool operator==(MyString& other);
	bool operator==(const char* str);
	//重载<
	bool operator<(MyString& other);
	bool operator<(const char* str);
	//重载<=
	bool operator<=(MyString& other);
	bool operator<=(const char* str);
	//重载>
	bool operator>(MyString& other);
	bool operator>(const char* str);
	//重载>=
	bool operator>=(MyString& other);
	bool operator>=(const char* str);

private:
	char *c_str;
	int size;
};
