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

	// 为安全起见，获取文件中记录的最大数量，避免超出 pBilling 分配的容量
	int maxCount = getBillingCount(pPath);
	if (maxCount <= 0)
	{
		fclose(fp);
		return FALSE;
	}

    char line[2048];//每行最大长度提高到2048，防止超长行导致的截断和解析错误
	int index = 0;//记录索引

	while (fgets(line, sizeof(line), fp) != NULL)
	{
        // 去掉行尾换行；如果行被截断（最后没有 '\n'），认为该行过长并跳过整行以避免解析错误
        size_t len = strlen(line);
        if (len == 0) continue;
        int truncated = 0;
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }
        else
        {
            // 行未包含换行，说明被缓冲区截断，丢弃该行剩余部分
            truncated = 1;
            int ch;
            while ((ch = fgetc(fp)) != EOF && ch != '\n') {}
        }
        if (truncated) continue; // 跳过过长被截断的行

		// 分割字段: cardName##logon##logoff##amount##status##ndel
		char tmp[2048];
		strncpy(tmp, line, sizeof(tmp) - 1);
		tmp[sizeof(tmp) - 1] = '\0';

        // 清零目标结构，保证未赋值字段安全
        if (index >= maxCount) break; // 保护性检查，避免溢出
        memset(&pBilling[index], 0, sizeof(Billing));

        /* 手动按分隔符 "##" 切分，避免 strtok 跳过空字段导致字段错位 */
        char* fields[6] = { 0 };
        char* curptr = tmp;
        for (int f = 0; f < 5; f++)
        {
            char* pos = strstr(curptr, "##");
            if (pos == NULL) break;
            *pos = '\0';
            fields[f] = curptr;
            curptr = pos + 2; /* skip "##" */
        }
        /* last field */
        fields[5] = curptr;

        /* fields[0] must be present (card name) */
        if (fields[0] == NULL)
        {
            continue; /* 格式不对，跳过该行 */
        }

        /* 复制并保证终止 */
        strncpy(pBilling[index].aCardName, fields[0], sizeof(pBilling[index].aCardName) - 1);
        pBilling[index].aCardName[sizeof(pBilling[index].aCardName) - 1] = '\0';

        pBilling[index].tLogon = fields[1] ? stringToTime(fields[1]) : 0;
        pBilling[index].tLogoff = fields[2] ? stringToTime(fields[2]) : 0;
        pBilling[index].fAmount = fields[3] ? (float)atof(fields[3]) : 0.0f;
        pBilling[index].nStatus = fields[4] ? atoi(fields[4]) : 0;
        pBilling[index].nDel = fields[5] ? atoi(fields[5]) : 0;

        index++;//准备读取下一行
	}
	fclose(fp);
    // 如果读取到的记录数少于 getBillingCount 的结果，清零剩余条目以保证调用方按 nCount 遍历时安全
    for (; index < maxCount; index++)
    {
        memset(&pBilling[index], 0, sizeof(Billing));
    }
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
	// 使用与 readBilling 相同的行缓冲大小，避免行拆分导致的计数不一致
	char line[2048];
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