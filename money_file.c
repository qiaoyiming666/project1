
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