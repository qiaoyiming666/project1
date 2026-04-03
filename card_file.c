#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

#include"model.h"
#include"global.h"
#include"tool.h"
#include"card_file.h"

// 将卡信息保存为二进制记录
int saveCard(const Card* pcard, const char* pPath)
{
	if(pcard == NULL || pPath == NULL)
	{
		return FALSE;
	}
	// 以二进制追加方式打开（若不存在则创建）
	FILE* fp = fopen(pPath, "ab");
	if (fp == NULL)
	{
		printf("打开文件失败！\n");
		return FALSE;
	}
	
	size_t written = fwrite(pcard, sizeof(Card), 1, fp);
	fclose(fp);

	if (written != 1)
	{
		return FALSE;
	}


	return TRUE;
}

// 将文件中的卡信息读取到内存（一次性读取所有记录）
// 假定调用者已经为 pCard 分配了足够空间（见 getCard 中的用法）
int readCard(Card* pCard, const char* pPath)
{
	if (pCard == NULL || pPath == NULL)
	{
		return FALSE;
	}
	//打开文件
	FILE* fp = fopen(pPath, "rb");
	if (fp == NULL)
	{
		// 文件可能不存在或打开失败，视为无数据（保持兼容旧逻辑）
		// 但返回 FALSE 以便调用方能区分错误
		printf("打开文件失败！\n");
		return FALSE;
	}

	// 计算记录数并一次性读取
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return FALSE;
	}
	long filesize = ftell(fp);
	if (filesize < 0)
	{
		fclose(fp);
		return FALSE;
	}
	rewind(fp);

	size_t recordCount = (size_t)(filesize / sizeof(Card));
	if (recordCount == 0)
	{
		fclose(fp);
		return TRUE; // 文件为空，认为读取成功但没有数据
	}

	size_t read = fread(pCard, sizeof(Card), recordCount, fp);
	fclose(fp);

	if (read != recordCount)
	{
		return FALSE;
	}

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
// 读取二进制文件中的记录数量
int getCardCount(const char* pPath)
{
	if (pPath == NULL)
	{
		return -1;
	}

	FILE* fp = fopen(pPath, "rb");
	if (fp == NULL)
	{
		// 打开失败（文件不存在或无法访问）
		return -1;
	}

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return -1;
	}
	long filesize = ftell(fp);
	fclose(fp);

	if (filesize < 0)
	{
		return -1;
	}

	return (int)(filesize / sizeof(Card));
}
// 根据索引更新二进制文件中对应的记录（索引从0开始），成功返回 TRUE
int updateCard(const Card* pCard, const char* pPath, int nIndex)
{
	if (pCard == NULL || pPath == NULL || nIndex < 0)
	{
		return FALSE;
	}

	FILE* fp = fopen(pPath, "r+b");
	if (fp == NULL)
	{
		return FALSE;
	}

	// 获取记录数，验证索引合法性
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return FALSE;
	}
	long filesize = ftell(fp);
	if (filesize < 0)
	{
		fclose(fp);
		return FALSE;
	}
	int recordCount = (int)(filesize / sizeof(Card));
	if (nIndex >= recordCount)
	{
		fclose(fp);
		return FALSE;
	}
	// 定位到指定记录并覆盖写入
	if (fseek(fp, (long)nIndex * (long)sizeof(Card), SEEK_SET) != 0)
	{
		fclose(fp);
		return FALSE;
	}
	size_t written = fwrite(pCard, sizeof(Card), 1, fp);
	fclose(fp);

	if (written != 1)
	{
		return FALSE;
	}
	return TRUE;
}