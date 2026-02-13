#include"MyString.h"
void test()
{

	;
}
int main()
{
	MyString str1="6666";
	cout << str1.data()<<endl;

	MyString str2 = str1;
	cout << str2.data() << endl;
	MyString str3="111";
	str3.MyStrcat(str1);
	cout << str3;
	test();

	while (1);
	return 0;
}