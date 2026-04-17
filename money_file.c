
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>

#include "money_file.h"
#include"model.h"
#include"global.h"
#include"tool.h"

// 将充值/退费记录保存到文本文件（追加）
int saveMoney(const Money* pMoney, const char* pPath)
{
	if (pMoney == NULL || pPath == NULL)
	{
		return FALSE;
	}
	// 以追加文本方式写入（若不存在则创建）
	FILE* fp = fopen(pPath, "a");
	if (fp == NULL)
	{
		printf("打开文件失败！\n");
		return FALSE;
	}

	// 格式: 卡号##时间##状态##金额##删除标记\n
	char timeBuf[TIMELENGTH] = { 0 };
	timeToString(pMoney->tTime, timeBuf);

	int ret = fprintf(fp, "%s##%s##%d##%.1f##%d\n",
		pMoney->aCardName,
		timeBuf,
		pMoney->nStatus,
		pMoney->fAmount,
		pMoney->nDel);

	fclose(fp);

	if (ret < 0)
	{
		return FALSE;
	}
	return TRUE;
}

// 从文件读取所有 money 记录到 pMoney（数组），按文件行序填充
int readMoney(Money* pMoney, const char* pPath)
{
	if (pMoney == NULL || pPath == NULL)
	{
		return FALSE;
	}

	FILE* fp = fopen(pPath, "r");
	if (fp == NULL)
	{
		// 文件打开失败
		return FALSE;
	}
	char line[512];// 假设每行不超过512字符
	int index = 0;

	while (fgets(line, sizeof(line), fp) != NULL)
	{
		// 去掉行末的换行符
		size_t len = strlen(line);
		if (len == 0) continue;
		if (line[len - 1] == '\n') line[len - 1] = '\0';

		// 分隔字段: cardName##time##status##amount##ndel
		char tmp[512];// 临时缓冲区
		strncpy(tmp, line, sizeof(tmp) - 1);
		tmp[sizeof(tmp) - 1] = '\0';

		char* token = NULL;
		const char* delim = "##";
		token = strtok(tmp, delim);// 卡号
		if (token == NULL) continue;
		strncpy(pMoney[index].aCardName, token, sizeof(pMoney[index].aCardName) - 1);

		token = strtok(NULL, delim);// 时间
		pMoney[index].tTime = token ? stringToTime(token) : 0;

		token = strtok(NULL, delim);// 状态
		pMoney[index].nStatus = token ? atoi(token) : 0;

		token = strtok(NULL, delim);// 金额
		pMoney[index].fAmount = token ? (float)atof(token) : 0.0f;

		token = strtok(NULL, delim);// 删除标记
		pMoney[index].nDel = token ? atoi(token) : 0;

		index++;
	}
	fclose(fp);
	return TRUE;
}

// 获取 money.ams 文件的记录数，失败返回 -1
int getMoneyCount(const char* pPath)
{
	if (pPath == NULL)
	{
		return -1;
	}
	FILE* fp = fopen(pPath, "r");
	if (fp == NULL)
	{
		// 文件打开失败
		return -1;
	}
	char line[512];// 假设每行不超过512字符
	int count = 0;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		// 判断是否全为空白行
		int allws = 1;
		for (char* p = line; *p != '\0'; ++p)
		{
			if (*p != '\n' && *p != '\r' && *p != ' ' && *p != '\t')
			{
				allws = 0;
				break;
			}
		}
		if (!allws) count++;
	}
	fclose(fp);
	return count;
}