#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>

#include"model.h"
#include"card_service.h"
#include"tool.h"


//输出菜单
void outputMenu() 
{
	printf("---------------菜单---------------\n");
	printf("1.添加卡\n");
	printf("2.查询卡\n");
	printf("3.上机\n");
	printf("4.下机\n");
	printf("5.充值\n");
	printf("6.退费\n");
	printf("7.查询系统\n");
	printf("8.注销卡\n");
	printf("0.退出\n");
	//提示选择菜单编号
	printf("请选择菜单编号（0~8）：");
}
//添加卡
void add()
{
	Card card;
	char aName[32] = { '\0' };
	char aPwd[20] = { '\0' };

	printf("---------------添加卡---------------\n");
	printf("请输入卡号（长度1~18）：");
	scanf("%s", aName);
	printf("请输入密码（长度1~8）：");
	scanf("%s", aPwd);
	
	//判断卡号和密码是否合法
	int nNameSize = getSize(aName);
	int nPwdSize = getSize(aPwd);

	if (nNameSize > 18 || nPwdSize > 8)
	{
		printf("卡号或者密码超过规定长度！");
		return;
	}
	else
	{
		strcpy(card.aName, aName);
		strcpy(card.aPwd, aPwd);
	}

	printf("请输入开卡金额（RMB）：");
	scanf("%f", &card.fBalance);
	
	card.fTotalUse = card.fBalance;  //初始化累计使用金额
	card.nUseCount = 0;           //初始化使用次数
	card.nStatus = 0;             //初始化卡状态
	card.nDel = 0;                //初始化删除标记

	/*初始化开卡时间、截止时间、最后使用时间*/
	card.tStart = time(NULL); 
	card.tEnd = time(NULL);
	card.tLastUse = time(NULL);

	struct tm* startTime;
	struct tm* endTime;

	startTime = localtime(&card.tStart);       //保存开卡时间
	endTime = localtime(&card.tEnd);          //转化截止时间类型，便于截止时间计算
	endTime->tm_year = startTime->tm_year + 1; //卡的有效期为一年
	card.tEnd = mktime(endTime);               //重新转化截止时间类型

	addCard(card);     //统计卡数量

	printf("\n");
	printf("----------添加的卡信息如下----------\n");
	printf("卡号\t密码\t卡状态\t余额\n");
	printf("%s\t%s\t%d\t%0.1f\n",aName,aPwd,card.nDel,card.fBalance);
}
//获取字符串长度
int getSize(const char* pInfo)
{
	int nSize = 0;
	while (*(pInfo + nSize) != '\0')
	{
		nSize++;
	}
	return nSize;
}
//查询卡信息
void query()      
{
	int aName[18] = { 0 };//卡号
	char aTime[20];
	Card* pCard = NULL;//保存卡信息

	int nIndex = 0;
	int i;

	printf("请输入查询的卡号：");
	scanf("%s", aName); 

	//查询卡
	pCard = queryCards(aName, &nIndex);
	if (pCard == NULL || nIndex == 0)
	{
		printf("没有该卡的信息！\n");
	}
	else
	{
		//显示
		printf("查询到的卡信息如下：\n");
		for (i = 0;i < nIndex;i++)
		{
			
			//将时间转化为字符串
			timeToString(pCard[i].tLastUse, aTime);

			printf("卡号\t状态\t余额\t累计使用\t使用次数\t上次使用时间\n");
			printf("%s\t%d\t%0.1f\t%0.1f\t\t%d\t\t%s\t\n", pCard[i].aName, pCard[i].nStatus, pCard[i].fBalance, pCard[i].fTotalUse, pCard[i].nUseCount, aTime);

		}
		
	}
	
}
//退出
void exitApp()
{
	releaseCardList();
}
