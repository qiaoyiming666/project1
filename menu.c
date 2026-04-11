#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

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
	printf("9.修改账号名或密码\n");
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
		printf("卡号或者密码超过规定长度！\n");
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

	// 处理查询失败、未找到或其它异常的情况，给出明确反馈并保证内存安全
	if (pCard == NULL && nIndex == 0)
	{
		// 完全未获取到数据（可能是读取文件错误或query内部失败）
		printf("查询失败：无法读取卡信息（可能是数据文件损坏或读取失败）。\n");
		return;
	}
	// nIndex == 0 表示没有匹配项，但 pCard 不为 NULL 可能是 queryCardsInfo 内部分配了内存但未找到匹配项的情况
	if (nIndex == 0)
	{
		// 没有匹配项，释放可能分配的内存并给出友好提示
		if (pCard != NULL)
		{
			free(pCard);
			pCard = NULL;
		}
		printf("未找到匹配的卡号：'%s'。\n", aName);
		printf("提示：\n");
		printf(" - 请检查卡号是否输入正确（大小写/字符）\n");
		printf(" - 支持部分匹配查询，例如输入一部分名称\n");
		return;
	}
	//显示
	printf("查询到的卡信息如下：\n");
	for (i = 0;i < nIndex;i++)
	{
		if (pCard == NULL)
		{
			printf("内部错误\n");//卡信息指针为空。
			break;
		}
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
		
	// 用完释放内存
	if (pCard != NULL)
	{
		free(pCard);
		pCard = NULL;
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
	logoffInfo info = { 0 };//保存下机信息
	int rc = 0;//调用新的 doLogoffInfo（返回状态码，并通过 info 输出下机信息）

	printf("---------------下机---------------\n");
	printf("请输入卡号：");
	//提示用户输入下机的卡号和密码
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

	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}
	//根据输入的卡号和密码进行验证，判断能否进行下机操作

	//调用下机函数进行下机操作
	rc = doLogoffInfo(aName, aPwd, &info); // 目前不处理下机信息输出
	//显示下机信息（下机时间、消费金额、余额等）
	if (rc == LOGOFFSUCCESS)
	{
		char timeBuf[TIMELENGTH] = { 0 };
		timeToString(info.tLogoff, timeBuf);
		printf("下机成功！\n");
		printf("卡号：%s\n", info.aCardName);
		printf("下机时间：%s\n", timeBuf);
		printf("本次消费：%.1f\n", info.fAmount);
		printf("当前余额：%.1f\n", info.fBalance);
	}
	else if (rc == UNUSED)
	{
		printf("下机失败：该卡当前未上机。\n");
	}
	else if (rc == BALANCEINSUFFICIENT)
	{
		printf("下机失败：余额不足，无法完成结算。\n");
	}
	else if (rc == LOGOFFFAILURE)
	{
		printf("下机失败：系统错误或未找到有效计费信息。\n");
	}
	else
	{
		printf("下机失败：未知错误（%d）。\n", rc);
	}
}

//充值
void addMoney()
{
	// 充值界面
	char aName[18] = { 0 };//卡号最大17字符
	char aPwd[8] = { 0 };//密码最大7字符
	float fAmount = 0.0f;
	MoneyInfo moneyInfo = { 0 };

	//提示用户输入充值的卡号、密码和金额
	printf("---------------充值---------------\n");
	printf("请输入卡号：");
	if (scanf("%17s", aName) != 1)
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
	printf("请输入充值金额：");
	if (scanf("%f", &fAmount) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("金额输入无效。\n");
		return;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	if (fAmount <= 0.0f)
	{
		printf("充值金额必须大于0。\n");
		return;
	}

	moneyInfo.fAmount = fAmount;
	//充值
	//显示充值结果（余额等）
	
	// 调用业务层处理充值（写日志并更新余额）
	if (doAddMoney(aName, aPwd, &moneyInfo) == TRUE)
	{
		printf("充值成功！\n");
		printf("卡号：%s\n充值金额：%.1f\n当前余额：%.1f\n", moneyInfo.aCardName, moneyInfo.fAmount, moneyInfo.fBalance);
	}
	else
	{
		printf("充值失败！请检查卡号/密码是否正确，卡是否被删除或正在使用，或系统错误。\n");
	}
}

//退费
void refundMoney()
{
	// 退费界面
	char aName[18] = { 0 };//卡号最大17字符
	char aPwd[8] = { 0 };//密码最大7字符
	float fAmount = 0.0f;
	MoneyInfo moneyInfo = { 0 };
	//提示用户输入退费的卡号、密码和金额
	printf("---------------退费---------------\n");
	printf("请输入卡号：");
	if (scanf("%17s", aName) != 1)
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
	printf("请输入退费金额：");
	if (scanf("%f", &fAmount) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("金额输入无效。\n");
		return;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	if (fAmount <= 0.0f)
	{
		printf("退费金额必须大于0。\n");
		return;
	}

	moneyInfo.fAmount = fAmount;

	//退费
	//显示退费结果（余额等）

	// 调用业务层处理退费（写日志并更新余额）
	if (doRefundMoney(aName, aPwd, &moneyInfo) == TRUE)
	{
		printf("退费成功！\n");
		printf("卡号：%s\n退费金额：%.1f\n当前余额：%.1f\n", moneyInfo.aCardName, moneyInfo.fAmount, moneyInfo.fBalance);
	}
	else
	{
		printf("退费失败！请检查卡号/密码是否正确、卡是否被删除或正在使用，或余额是否足够，或系统错误。\n");
	}
}

//注销卡
void annul()
{
	char aName[18] = { 0 };//卡号
	char aPwd[8] = { 0 };//密码
	Card cardParam = { 0 };

	printf("---------------注销卡---------------\n");
	printf("请输入卡号：");
	//提示用户输入退费的卡号、密码和金额
	if (scanf("%17s", aName) != 1)
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

	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	// 准备参数并调用业务层注销函数
	strncpy(cardParam.aName, aName, sizeof(cardParam.aName) - 1);
	strncpy(cardParam.aPwd, aPwd, sizeof(cardParam.aPwd) - 1);
	cardParam.fBalance = 0.0f;

	if (annulCard(&cardParam) == TRUE)
	{
		// 注销成功，显示卡号和退款金额（两列）
		printf("注销成功！\n");
		printf("卡号\t退款金额\n");
		printf("%s\t%.1f\n", cardParam.aName, cardParam.fBalance);
	}
	else
	{
		printf("注销卡失败！\n");
	}
}


// 修改：menu 层仅负责交互，调用 service.doModifyAccount
//修改账号或密码
void modifyAccount()
{
	char aName[18] = { 0 };// 当前卡号
	char aPwd[8] = { 0 };// 当前密码
	char newName[18] = { 0 };
	char newPwd[8] = { 0 };
	int modifyNameFlag = 0;//是否修改账号名
	int modifyPwdFlag = 0;//是否修改密码

	printf("---------------修改账号名或密码---------------\n");
	printf("请输入当前卡号：");
	if (scanf("%17s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入当前密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	// 清理残余输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	printf("请选择要修改的项目：\n");
	printf("1. 修改账号名\n");
	printf("2. 修改密码\n");
	printf("3. 同时修改\n");
	printf("0. 取消\n");
	printf("请选择：");
	int opt = -1;
	if (scanf("%d", &opt) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	// 清理残余输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	if (opt == 0)
	{
		printf("已取消。\n");
		return;
	}
	if (opt == 1 || opt == 3)
	{
		printf("请输入新账号名（长度1~17）：");
		if (scanf("%17s", newName) != 1)
		{
			int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
			printf("输入无效。\n");
			return;
		}
		modifyNameFlag = 1;
	}
	if (opt == 2 || opt == 3)
	{
		printf("请输入新密码（长度1~7）：");
		if (scanf("%7s", newPwd) != 1)
		{
			int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
			printf("输入无效。\n");
			return;
		}
		modifyPwdFlag = 1;
	}
	// 清理残余输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	// 调用业务层
	if (doModifyAccount(aName, aPwd, newName, newPwd, modifyNameFlag, modifyPwdFlag) == TRUE)
	{
		printf("修改成功！\n");
	}
	else
	{
		printf("修改失败！请检查卡号/密码或新账号名是否已存在，或系统错误。\n");
	}
}
//退出
void exitApp()
{
	releaseList(); 
}
