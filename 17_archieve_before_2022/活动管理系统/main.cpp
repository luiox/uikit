#include"head.h"
#define MAX_ACT 100

ACT act[MAX_ACT];//最大活动数量
int curAct=-1;//当前活动数量

USER ru;//正确的账号密码
//登陆
int login()
{
	USER u = { 0 };
	int asc = 0, num = 0;
	printf("账号:");
	scanf("%s", &u.uesrname);
	printf("密码：");
	while (1)
	{
		asc = getch();
		if (asc != 13)//数字'13'是回车键的ASCII代码值
		{
			u.uesrname[num] += (char)asc;
			num++;
			printf("*");//密码掩盖
		}
		else
		{
			break;
		}
	}
	printf("\n");
	printf("%s\n", u.password);
	if (strcmp(u.uesrname, ru.uesrname) == 0 && strcmp(u.password, ru.password) == 0)
		return 1;
	return 0;
}
//创建活动
void createAct()
{
	printf("请输入活动名字,发起人,内容,时间,人数,要求\n");
	curAct++;
	scanf("%s,%s,%s,%s,%d,%s", &act[curAct].name, &act[curAct].uesrname,
		&act[curAct].content, &act[curAct].time, &act[curAct].num,
		&act[curAct].requirements);
	act[curAct].id = (curAct + 1);

}
//发表言论
void speak()
{
	printf("你要对哪个活动发表评论？（活动ID,内容）");
	char str[50] = "";
	int nid;
	scanf("%d,%s", &nid, &str);
	//查找
	for (int i = 0; i <= curAct; i++)
	{
		if (act[i].id == nid)
		{
			strcat(act[i].personnelList, str);
		}
	}

}
//输出所有活动
void printAllAct()
{
	for (int i = 0; i < curAct; i++)
	{
		printf("活动名字:%s,发起人:%s,内容:%s,时间:%s,人数:%d,要求:%s\n", 
			act[i].name,act[i].uesrname,
		act[i].content, act[i].time, act[i].num, act[i].requirements);
	}
}
//查找--还有一点没写完
void search()
{
	printf("请输入你要查找的活动名");
	char str[50];    
	char str2[10][10];//要查找的字符串
	scanf("%s", &str);
	//处理各个关键词,把每个关键词放到数组里面
	int a = 0, i = 0,j=0;//a用来遍历str,i,j分别控制行和列，同时把字符长度放到末尾
	while (str[a] != '\0')
	{
		if (str[a] != '+')
		{
			str2[i][j] = str[a];
			j++;
		}
		else{
			str2[i][j] = (j+1);
			j = 0;
			i++;
		}
		a++;
	}

	//字符匹配
	for (int k = 0; k <= i; i++)
	{
		int l = 0;
		while (!(act[l].name[l] >= '0' && act[l].name[l] <= '9'))//由于末尾放的是长度
		{
			/*
			判断字符是否相同
			*/
			//if ()
			//{
			//
			//}
		}
	}

}
//参加活动
void join()
{
	printf("你要对参加活动？（活动ID）");
	int nid;
	scanf("%d", &nid);
	//查找
	for (int i = 0; i <= curAct; i++)
	{
		if (act[i].id == nid)
		{
			act[i].num++;
		}
	}
}
//统计
void statistical()
{

}
//读用户文件和活动--未写完
void writeUserAct()
{
	FILE* f1 = fopen("user.txt", "r+");
	FILE* f2 = fopen("act.txt", "r+");
	while (!feof(f1))//读用户
	{
		
	}
	while (!feof(f2))//读活动
	{

	}
	fclose(f1);
	fclose(f2);
}
//写用户文件--未写完
void readUser()
{
	FILE* f = fopen("user.txt", "w+");
	for (int i = 0; i <= curAct; i++)
	{
		//fwrite();
	}
	fclose(f);
}
//写活动文件--未写完
void readUser()
{
	FILE* f = fopen("act.txt", "w+");
	for (int i = 0; i <= curAct; i++)
	{
		//fwrite();
	}
	fclose(f);
}
//主函数
int main()
{
	writeUserAct();//读入数据
	/*
	正确的账号密码，你可以用读写文件的方法，我没时间写了
	*/
	strcpy(ru.uesrname, "admin");
	strcpy(ru.password, "123456");
	if (0 == login())//登陆失败，退出程序
		exit(0);
	while (1)
	{
		system("cls");
		printf("****************************\n");
		printf("1.查看所有征集活动\n");
		printf("2.发表言论\n");
		printf("3.统计\n");
		printf("4.查询\n");
		printf("5.创建活动\n");
		printf("6.参加活动\n");
		printf("7.退出\n");
		printf("****************************\n");
		printf("请输入你的选择:");
		int n = 0;
		scanf("%d", &n);//这个可以用getch就不需要回车
		switch (n)
		{
		case 1://查看所有征集活动
			printAllAct();
			break;
		case 2://发表言论
			speak();
			break;
		case 3://统计
			statistical();
			break;
		case 4://查询
			search();
			break;
		case 5://创建活动
			createAct();
			break;
		case 6://参加一个活动
			join();
			break;
		case 7://退出
			exit(0);
			break;
		}
		system("pause");
	}

	return 0;
}