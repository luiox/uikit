#define _CRT_SECURE_NO_WARNINGS
#include "MyString.h"
MyString::MyString(int size=0) :size(size)
{
	this->c_str = new char(size+1);
	strcpy(c_str, "");
}
MyString::MyString(const char*c_str)
{
	size = strlen(c_str);
	this->c_str = new char(size + 1);
	strcpy(this->c_str, c_str);
}
MyString::MyString(const MyString& other)
{
	size = other.size;
	this->c_str = new char(size);
	strcpy(this->c_str, other.c_str);
}
MyString::~MyString()
{
	if (c_str != NULL)
	{
		delete(c_str);
		c_str = NULL;
	}
}
const char* MyString::data()
{
	return c_str;
}
const char* MyString::MyStrcat(const MyString& other)
{
	//大小是否够
	if (other.size == 0)
	{
		return this->c_str;
	}
	else
	{
		//保存当前，并且释放
		char *temp = new char(this->size+1);
		strcpy(temp, this->c_str);
		delete(this->c_str);
		//重新开辟内存
		this->size = this->size + other.size;
		this->c_str = new char(this->size + 1);
		strcpy(this->c_str, temp);
		strcat(this->c_str, other.c_str);
		delete(temp);
	}
	return this->c_str;
}
const char* MyString::MyStrcat(const char* str)
{
	//大小是否够
	if (strlen(str) == 0)
	{
		return this->c_str;
	}
	else
	{
		//保存当前，并且释放
		char *temp = new char(this->size + 1);
		strcpy(temp, this->c_str);
		delete(this->c_str);
		//重新开辟内存
		this->c_str = new char(this->size + strlen(str) + 1);
		strcpy(this->c_str, temp);
		strcat(this->c_str, str);
		delete(temp);
		
	}
	return this->c_str;
}
const char* MyString::MyStrcpy(const MyString& other)
{
	//大小是否够
	if (other.size == 0)
	{
		return this->c_str;
	}
	else
	{
		//直接释放，重新申请
		delete(this->c_str);
		this->size = other.size;
		this->c_str = new char(size + 1);
		strcpy(this->c_str, other.c_str);
	}
	return this->c_str;
}
const char* MyString::MyReverse()
{
	for (int i = 0; i < size / 2; i++)
	{
		char temp = this->c_str[i];
		this->c_str[i] = this->c_str[size - i - 1];
		this->c_str[size - i - 1] = temp;
	}
	return this->c_str;
}
bool MyString::MyStrcmp(const MyString& other)
{
	if (1 == strcmp(this->c_str, other.c_str))
	{
		return true;
	}
	else
	{
		return false;
	}
}
MyString MyString::operator+=(MyString& other)
{
	char*temp = new char[this->size + 1];
	strcpy(temp, this->c_str);
	delete[]c_str;
	size = size + other.size;
	c_str = new char[size + 1];
	strcpy(c_str, temp);
	strcat(c_str, other.c_str);
	delete[] temp;
	return *this;
}
MyString MyString::operator=(MyString& other)
{
	delete[]c_str;
	size = other.size;
	c_str = new char[size + 1];
	strcpy(c_str, other.c_str);
	return *this;
}
ostream& operator<<(ostream& out, MyString& other)
{
	cout << other.data() << endl;
	return out;
}
bool MyString::operator==(MyString& other)
{
	return this->c_str == other.c_str ? true : false;
}
bool MyString::operator==(const char* str)
{
	return this->c_str == str ? true : false;
}
bool MyString::operator<(MyString& other)
{
	return this->c_str < other.c_str ? true : false;
}
bool MyString::operator<(const char* str)
{
	return this->c_str < str ? true : false;
}
bool MyString::operator<=(MyString& other)
{
	return this->c_str <= other.c_str ? true : false;
}
bool MyString::operator<=(const char* str)
{
	return this->c_str <= str ? true : false;
}
bool MyString::operator>(MyString& other)
{
	return this->c_str > other.c_str ? true : false;
}
bool MyString::operator>(const char* str)
{
	return this->c_str > str ? true : false;
}
bool MyString::operator>=(MyString& other)
{
	return this->c_str >= other.c_str ? true : false;
}
bool MyString::operator>=(const char* str)
{
	return this->c_str >= str ? true : false;
}
