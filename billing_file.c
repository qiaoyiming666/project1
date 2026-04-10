#define _CRT_SECURE_NO_WARNINGS


#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"model.h"
#include"global.h"
#include"tool.h"

//将信息保存到billing.ams文件中
int saveBilling(const Billing* pBilling, const char* pPath)
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
	char logonTime[TIMELENGTH] = { 0 };//上机时间字符串
	char logoffTime[TIMELENGTH] = { 0 };//下机时间字符串
	timeToString(pBilling->tLogon, logonTime);
	timeToString(pBilling->tLogoff, logoffTime);
	fprintf(fp, "%s##%s##%s##%.1f##%d##%d\n",
		pBilling->aCardName,
		logonTime,
		logoffTime,
		pBilling->fAmount,
		pBilling->nStatus,
		pBilling->nDel);
	//关闭文件
	fclose(fp);
	return TRUE;
}

// 从文本文件中读取所有消费记录到已分配的 pBilling 数组中
int readBilling(Billing* pBilling, const char* pPath)
{
	if (pBilling == NULL || pPath == NULL)
	{
		return FALSE;
	}

	FILE* fp = fopen(pPath, "r");
	if (fp == NULL)
	{
		// 文件不存在或打开失败
		return FALSE;
	}
	char line[512];//每行最大长度假定为512
	int index = 0;//记录索引

	while (fgets(line, sizeof(line), fp) != NULL)
	{
		// 去掉行尾换行
		size_t len = strlen(line);
		if (len == 0) continue;
		if (line[len - 1] == '\n') line[len - 1] = '\0';

		// 分割字段: cardName##logon##logoff##amount##status##ndel
		char tmp[512];
		strncpy(tmp, line, sizeof(tmp) - 1);
		tmp[sizeof(tmp) - 1] = '\0';

		char* token = NULL;
		const char* delim = "##";
		token = strtok(tmp, delim);
		if (token == NULL) continue;//至少应该有一个字段（卡号），否则跳过
		strncpy(pBilling[index].aCardName, token, sizeof(pBilling[index].aCardName) - 1);//复制卡号

		token = strtok(NULL, delim);
		pBilling[index].tLogon = token ? stringToTime(token) : 0;//解析上机时间

		token = strtok(NULL, delim);
		pBilling[index].tLogoff = token ? stringToTime(token) : 0;//解析下机时间

		token = strtok(NULL, delim);
		pBilling[index].fAmount = token ? (float)atof(token) : 0.0f;//解析消费金额

		token = strtok(NULL, delim);
		pBilling[index].nStatus = token ? atoi(token) : 0;//解析状态

		token = strtok(NULL, delim);
		pBilling[index].nDel = token ? atoi(token) : 0;//解析删除标记

		index++;//准备读取下一行
	}
	fclose(fp);
	return TRUE;
}

// 统计 billing 文件中的记录行数，失败返回 -1
int getBillingCount(const char* pPath)
{
	if (pPath == NULL)
	{
		return -1;
	}
	FILE* fp = fopen(pPath, "r");
	if (fp == NULL)
	{
		return -1;
	}
	char line[512];
	int count = 0;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		// 忽略空行
		int allws = 1;
		for (char* p = line; *p != '\0'; ++p)//检查是否全是空白字符
		{
			if (*p != '\n' && *p != '\r' && *p != ' ' && *p != '\t')
			{
				allws = 0;
				break;
			}
		}
		if (!allws) count++;// 只有非空行才计数
	}
	fclose(fp);
	return count;
}

// 根据索引更新 billing 文件中的指定记录（索引从0开始），成功返回 TRUE
int updateBilling(const Billing* pBilling, const char* pPath, int nIndex)
{
	if (pBilling == NULL || pPath == NULL || nIndex < 0)
	{
		return FALSE;
	}
	// 读取所有行到内存
	FILE* fp = fopen(pPath, "r");
	if (fp == NULL)
	{
		return FALSE;
	}
	char** lines = NULL;//保存所有行的动态数组
	size_t cap = 0;
	size_t len = 0;
	char buffer[512];
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		// 保存这一行
		char* copy = (char*)malloc(strlen(buffer) + 1);
		if (copy == NULL)
		{
			// 释放已有分配
			for (size_t i = 0; i < len; i++) free(lines[i]);//释放已保存的行
			free(lines);
			fclose(fp);
			return FALSE;
		}
		strcpy(copy, buffer);
		// 去掉末尾换行以便重新格式化时一致
		size_t blen = strlen(copy);
		if (blen > 0 && (copy[blen - 1] == '\n' || copy[blen - 1] == '\r')) copy[blen - 1] = '\0';//去掉行尾换行
		if (len + 1 > cap)
		{
			size_t newcap = cap == 0 ? 16 : cap * 2;
			char** tmp = (char**)realloc(lines, newcap * sizeof(char*));
			if (tmp == NULL)
			{
				for (size_t i = 0; i < len; i++) free(lines[i]);
				free(copy);
				free(lines);
				fclose(fp);
				return FALSE;
			}
			lines = tmp;//更新指针
			cap = newcap;//更新容量
		}
		lines[len++] = copy;//保存行指针
	}
	fclose(fp);

	if (nIndex >= (int)len)
	{
		// 索引越界
		for (size_t i = 0; i < len; i++) free(lines[i]);
		free(lines);
		return FALSE;
	}
	// 格式化 pBilling 为新的行文本
	char newLine[512] = { 0 };
	char logonTime[TIMELENGTH] = { 0 };
	char logoffTime[TIMELENGTH] = { 0 };
	timeToString(pBilling->tLogon, logonTime);
	timeToString(pBilling->tLogoff, logoffTime);
	snprintf(newLine, sizeof(newLine), "%s##%s##%s##%.1f##%d##%d",
		pBilling->aCardName,
		logonTime,
		logoffTime,
		pBilling->fAmount,
		pBilling->nStatus,
		pBilling->nDel);

	// 替换第 nIndex 行
	free(lines[nIndex]);
	lines[nIndex] = (char*)malloc(strlen(newLine) + 2);
	if (lines[nIndex] == NULL)
	{
		for (size_t i = 0; i < len; i++) if (lines[i]) free(lines[i]);
		free(lines);
		return FALSE;
	}
	strcpy(lines[nIndex], newLine);

	// 写回文件（覆盖）
	fp = fopen(pPath, "w");
	if (fp == NULL)
	{
		for (size_t i = 0; i < len; i++) free(lines[i]);
		free(lines);
		return FALSE;
	}
	for (size_t i = 0; i < len; i++)
	{
		fprintf(fp, "%s\n", lines[i]);
		free(lines[i]);
	}
	free(lines);
	fclose(fp);
	return TRUE;
}