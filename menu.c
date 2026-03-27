#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>

#include"model.h"
#include"card_service.h"
#include"service.h"
#include"tool.h"
#include"global.h"
#include"menu.h"


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

	//统计卡数量
	if (!addCardInfo(card))
	{
		printf("添加卡信息失败！\n");
	}
	else
	{
		printf("\n");
		printf("----------添加的卡信息如下----------\n");
		printf("卡号\t密码\t卡状态\t余额\n");
		printf("%s\t%s\t%d\t%0.1f\n",aName,aPwd,card.nDel,card.fBalance);

	}
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
	char aName[18] = { 0 };//卡号
	char aTime[TIMELENGTH] = { 0 };//时间字符串
	Card* pCard = NULL;//保存卡信息

	int nIndex = 0;
	int i;

	printf("请输入查询的卡号：");
	fflush(stdout);            // 确保提示立即输出
	//scanf("%s", aName); 

	if (scanf("%17s", aName) != 1) // 限制最多读入17个字符，留1字节给终止符
	{
		int ch;
		while ((ch = getchar()) != '\n' && ch != EOF) {} // 清理残留输入
		printf("输入无效。\n");
		return;
	}

	//查询卡
	pCard = queryCardsInfo(aName, &nIndex);
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
			printf("%s\t%d\t%0.1f\t%0.1f\t\t%d\t\t%s\t\n", 
				pCard[i].aName, 
				pCard[i].nStatus, 
				pCard[i].fBalance, 
				pCard[i].fTotalUse, 
				pCard[i].nUseCount, 
				aTime);

		}
		
	}
	
}

//上机
void logon()
{
	char aName[20] = { 0 };//卡号
	char aPwd[8] = { 0 };//密码
	Card* pCard = NULL;
	LogonInfo info = { 0 };
	int rc;
	
	printf("---------------上机---------------\n");
	printf("请输入卡号：");
	// 接收用户输入的卡号和密码
	if (scanf("%19s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	//进行上机
	//pCard = doLogonInfo(aName, aPwd);
	// 调用新的 doLogonInfo（返回状态码，并通过 info 输出上机信息）
	rc = doLogonInfo(aName, aPwd, &info);
	
	//显示上机信息
	/*printf("上机信息如下：\n");
	if (pCard == NULL)
	{
		printf("上机失败！\n");
	}
	else
	{
		printf("卡号\t上机时间\t余额\n");
		//上机时间默认为最后使用时间，后面添加计费信息后，使用计费信息中的上机时间
		char logonTime[TIMELENGTH] = { 0 };
		timeToString(pCard->tLastUse, logonTime);
		printf("%s\t%s\t%0.1f\n", pCard->aName, logonTime, pCard->fBalance);
	}*/
	// 根据返回码处理
	if (rc == LOGONSUCCESS)
	{
		// 使用卡号查询并获得 Card*（保持原来通过 pCard 使用的方式）
		pCard = queryCard(aName);
		printf("上机信息如下：\n");
		if (pCard == NULL)
		{
			// 理论上不应发生，但做容错处理
			printf("上机成功，但无法读取卡详情。\n");
		}
		else
		{
			printf("卡号\t余额\t上机时间\n");
			char logonTime[TIMELENGTH] = { 0 };
			// 优先使用 billing/logonInfo 的时间（info.tLogon）
			timeToString(info.tLogon != 0 ? info.tLogon : pCard->tLastUse, logonTime);
			printf("%s\t%0.1f\t%s\n", pCard->aName,info.fBalance , logonTime);
		}
	}
	else if (rc == CARDNOTFOUND)
	{
		printf("上机失败：未找到该卡。\n");
	}
	else if (rc == BALANCEINSUFFICIENT)
	{
		printf("上机失败：余额不足。\n");
	}
	else
	{
		printf("上机失败：系统错误或文件写入失败。\n");
	}
}

//下机
void logoff()
{
	char aName[20] = { 0 };//卡号
	char aPwd[8] = { 0 };//密码

	printf("---------------下机---------------\n");
	printf("请输入卡号：");
	//提示用户输入下机的卡号和密码
	scanf("%19s", aName);
	printf("请输入密码：");
	scanf("%7s", aPwd);
	//根据输入的卡号和密码进行验证，判断能否进行下机操作

	//调用下机函数进行下机操作
	int nResult = doLogoffInfo(aName, aPwd, NULL); // 目前不处理下机信息输出
	//显示下机信息（下机时间、消费金额、余额等）
	switch	(nResult)
	{
		case LOGOFFFAILURE:
		printf("下机失败！\n");
		case LOGOFFSUCCESS:
		printf("下机成功！\n");
		break;
	}
}







//退出
void exitApp()
{
	releaseList(); 
}
