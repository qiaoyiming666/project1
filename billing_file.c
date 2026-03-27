#define _CRT_SECURE_NO_WARNINGS


#include<stdio.h>
#include<string.h>

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

//
int readBilling(Billing* pBilling, const char* pPath)
{
	//读取计费信息文件
	return FALSE;
}

//
int getBillingCount(const char* pPath)
{
	//统计计费信息数量
	return 0;
}