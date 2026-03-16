#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"model.h"
#include"global.h"
#include"tool.h"
#include"card_file.h"

//将卡信息保存到文件中
int saveCard(const Card* pcard, const char* pPath)
{
	FILE* fp = NULL;
	//打开文件
	if ((fp = fopen(pPath, "a")) == NULL)
	{
		fp = fopen(pPath, "w");
		if (fp == NULL)
		{
			printf("打开文件失败！");
			return FALSE;
		}
	}
	
	//将数据写入文件
	//将时间转化为字符串
	char startTime[TIMELENGTH] = { 0 };//开卡时间字符串
	char endTime[TIMELENGTH] = { 0 };//截止时间字符串
	char LastTime[TIMELENGTH] = { 0 };//最后使用时间字符串

	timeToString(pcard->tStart, startTime);
	timeToString(pcard->tEnd, endTime);
	timeToString(pcard->tLastUse, LastTime);
	// 按照：卡号##密码##状态##开卡时间##截止时间##累积金额##最后使用时间##使用次数##当前余额##删除标识
	fprintf(fp, "%s##%s##%d##%s##%s##%.1f##%s##%d##%.1f##%d\n",
		pcard->aName,
		pcard->aPwd,
		pcard->nStatus,
		startTime,
		endTime,
		pcard->fTotalUse,
		LastTime,
		pcard->nUseCount,
		pcard->fBalance,
		pcard->nDel);

	//关闭文件
	fclose(fp);


	return TRUE;
}

//将文件中的卡信息读取到内存中
int readCard(Card* pCard, const char* pPath)
{
	//打开文件
	FILE* fp = fopen(pPath, "r");
	char aBuf[CARDCHARNUM] = { 0 };//保存从文件中读取的卡信息字符串
	int i = 0;
	
	if (fp == NULL)
	{
		printf("打开文件失败！");
		return FALSE;
	}

	//从文件中读取数据
	while (fgets(aBuf, CARDCHARNUM, fp) != NULL)      //原feof(fp) != NULL
	{
		size_t len = strlen(aBuf);
		if (len > 0)    //原fgets(aBuf, CARDCHARNUM, fp) != NULL
		{
			if (aBuf[len - 1] == '\n')
			{
				aBuf[len - 1] = '\0';
			}
			pCard[i] = praseCard(aBuf);
			i++;
		}
		memset(aBuf, 0, sizeof(aBuf)); //清空aBuf数组

		
	}

	//关闭文件
	fclose(fp);

	return TRUE;
}

//解析函数
Card praseCard(const char* pBuf)
{
	Card card = { 0 };

	const char* delims = "##";
	char* buf = NULL;
	char* str = NULL;
	char flag[10][20] = { 0 };//保存解析后的字符串
	int index = 0;

	buf = pBuf;//第一次调用strtok函数时，buf为解析字符串

	while (str = strtok(pBuf, delims) != NULL)
	{
		strcpy(flag[index], str);
		buf = NULL;//后面调用strtok函数时，第一个参数为NULL
		index++;
	}

	strcpy(card.aName, flag[0]);
	strcpy(card.aPwd, flag[1]);
	card.nStatus = atoi(flag[2]);             //卡状态
	//card.tStart = stringToTime(flag[3]);   //开卡时间
	//card.tEnd = stringToTime(flag[4]);     //截止时间
	card.fTotalUse = atof(flag[5]);       //累计使用金额
	//card.tLastUse = stringToTime(flag[6]); //最后使用时间
	card.nUseCount = atoi(flag[7]);       //使用次数
	card.fBalance = atof(flag[8]);       //当前余额
	card.nDel = atoi(flag[9]);         //删除标记

	//char startTime[TIMELENGTH] = { 0 };//开卡时间字符串
	//char endTime[TIMELENGTH] = { 0 };//截止时间字符串
	//char LastTime[TIMELENGTH] = { 0 };//最后使用时间字符串
	
	return card;
}

//读取卡数量
int getCardCount(const char* pPath)
{
	//打开文件
	FILE* fp = fopen(pPath, "r");
	char aBuf[CARDCHARNUM] = { 0 };//保存从文件中读取的卡信息字符串
	int nCount = 0;

	if (fp == NULL)
	{
		printf("打开文件失败！");
		return FALSE;
	}

	//从文件中读取数据
	while (fgets(aBuf, CARDCHARNUM, fp) != NULL)
	{
		
		if (strlen(aBuf) > 0)
		{
			nCount++;
		}
		memset(aBuf, 0, sizeof(aBuf)); //清空aBuf数组

	}

	//关闭文件
	fclose(fp);

	return nCount;
}