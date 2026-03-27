#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

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
/*
//解析函数
Card praseCard(const char* pBuf)
{
	Card card = { 0 };

	const char* delims = "##";
	char* buf = NULL;
	char* str = NULL;
	char flag[10][64] = { 0 };//保存解析后的字符串
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
	card.tStart = stringToTime(flag[3]);   //开卡时间
	card.tEnd = stringToTime(flag[4]);     //截止时间
	card.fTotalUse = atof(flag[5]);       //累计使用金额
	card.tLastUse = stringToTime(flag[6]); //最后使用时间
	card.nUseCount = atoi(flag[7]);       //使用次数
	card.fBalance = atof(flag[8]);       //当前余额
	card.nDel = atoi(flag[9]);         //删除标记
	
	return card;
}*/

//解析函数
Card praseCard(const char* pBuf)
{
	Card card = { 0 };
	char tmp[CARDCHARNUM];
	char flag[10][64] = { 0 }; // 增大每字段缓冲以更安全地存放数据
	int index = 0;

	if (pBuf == NULL)
	{
		return card;
	}

	// 拷贝到可写缓冲区，避免修改 const 数据
	strncpy(tmp, pBuf, sizeof(tmp) - 1);
	tmp[sizeof(tmp) - 1] = '\0';

	const char* delims = "##";
	char* token = strtok(tmp, delims);

	while (token != NULL && index < 10)
	{
		// 安全拷贝每个字段
		strncpy(flag[index], token, sizeof(flag[index]) - 1);
		flag[index][sizeof(flag[index]) - 1] = '\0';
		index++;
		token = strtok(NULL, delims);
	}

	// 解析字段到结构体（有些字段可能为空，按需检查）
	if (index > 0) strncpy(card.aName, flag[0], sizeof(card.aName) - 1);
	if (index > 1) strncpy(card.aPwd, flag[1], sizeof(card.aPwd) - 1);
	if (index > 2) card.nStatus = atoi(flag[2]);
	// 如果需要解析时间，请实现 stringToTime 并启用下面两行
	if (index > 3) card.tStart = stringToTime(flag[3]);
	if (index > 4) card.tEnd = stringToTime(flag[4]);
	if (index > 5) card.fTotalUse = (float)atof(flag[5]);
	if (index > 6) card.tLastUse = stringToTime(flag[6]);
	if (index > 7) card.nUseCount = atoi(flag[7]);
	if (index > 8) card.fBalance = (float)atof(flag[8]);
	if (index > 9) card.nDel = atoi(flag[9]);

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
		//return FALSE;
		return -1; // 返回-1表示文件打开失败,0表示文件打开成功但没有数据
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
//根据卡号更新文件里对应的记录（通过比较 aName 字段），成功返回 TRUE
int updateCard(const Card* pCard, const char* pPath, int nIndex)
{
	if (pCard == NULL || pPath == NULL)
	{
		return FALSE;
	}

	char aBuf[CARDCHARNUM] = { 0 };//保存从文件中读取的卡信息字符串
	char startTime[TIMELENGTH] = { 0 };//开卡时间字符串
	char endTime[TIMELENGTH] = { 0 };//截止时间字符串
	char LastTime[TIMELENGTH] = { 0 };//最后使用时间字符串
	int nLine = 0;//文件行数
	long pos = 0;//文件指针位置

	//将时间转化为字符串
	timeToString(pCard->tStart, startTime);
	timeToString(pCard->tEnd, endTime);
	timeToString(pCard->tLastUse, LastTime);

	//以只读方式打开文件
	FILE* fp = fopen(pPath, "r");
	if (fp == NULL)
	{
		return FALSE;
	}
	//遍历文件，找到该条记录，进行更新

	while (!feof(fp) && nLine < nIndex)
	{
		if(fgets(aBuf, CARDCHARNUM, fp) != NULL)
		{
			pos = ftell(fp);//记录文件指针位置
			nLine++;
		}
		else
		{
			fclose(fp);
			return FALSE; // 文件读取错误或行数不足
		}
	}
	//移动文件指针到该记录的开头
	fseek(fp, pos, SEEK_SET);  //SEEK_SET=0
	//将数据写入文件
	fprintf(fp, "%s##%s##%d##%s##%s##%.1f##%s##%d##%.1f##%d\n",
		pCard->aName,
		pCard->aPwd,
		pCard->nStatus,
		startTime,
		endTime,
		pCard->fTotalUse,
		LastTime,
		pCard->nUseCount,
		pCard->fBalance,
		pCard->nDel);
	//关闭文件
	fclose(fp);
	return TRUE;
}