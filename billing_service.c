
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<stdio.h>

#include"billing_file.h"
#include"model.h"
#include"global.h"

lpBillingNode billingList = NULL;//消费记录链表

//添加消费记录
int addBilling(Billing billing)
{
	return saveBilling(&billing, BILLINGPATH);
}

//初始化链表
void initBillingList()
{
	lpBillingNode head = NULL;
	head = (lpBillingNode)malloc(sizeof(BillingNode));
	if (head != NULL)
	{
		head->next = NULL;
		billingList = head;
	}
}

//释放
void releaseBillingList()
{
	lpBillingNode cur;
	if (billingList != NULL)
	{
		cur = billingList->next;
		while (cur != NULL)
		{
			lpBillingNode tmp = cur->next;
			free(cur);
			cur = tmp;
		}
		free(billingList);
		billingList = NULL;
	}
}

// 将文件中的消费记录加载到链表中（按文件顺序）
int getBilling()
{
	int nCount = getBillingCount(BILLINGPATH);
	if (nCount < 0)
	{
		// 打开或读取失败
		fprintf(stderr, "getBilling: 无法读取文件或打开失败: %s\n", BILLINGPATH);
		return FALSE;
	}
	if (nCount == 0)
	{
		// 没有记录，确保链表为空
		if (billingList != NULL)
		{
			billingList->next = NULL;
		}
		return TRUE;
	}

	// 释放旧链表
	if (billingList != NULL)
	{
		releaseBillingList();
	}

	// 为所有记录分配临时数组
	Billing* pBilling = (Billing*)malloc(sizeof(Billing) * nCount);
	if (pBilling == NULL)
	{
		return FALSE;
	}
	if (FALSE == readBilling(pBilling, BILLINGPATH))// 读取失败
	{
		free(pBilling);
		return FALSE;
	}

	// 创建头结点并按文件顺序追加节点（保持文件顺序）
	lpBillingNode head = (lpBillingNode)malloc(sizeof(BillingNode));
	if (head == NULL)
	{
		free(pBilling);
		return FALSE;
	}
	head->next = NULL;
	billingList = head;

	lpBillingNode tail = head;

	for (int i = 0; i < nCount; i++)
	{
		lpBillingNode cur = (lpBillingNode)malloc(sizeof(BillingNode));
		if (cur == NULL)
		{
			free(pBilling);
			// 释放已构建节点
			releaseBillingList();
			return FALSE;
		}
		memset(cur, 0, sizeof(BillingNode));//初始化新空间
		cur->data = pBilling[i];
		cur->next = NULL;

		tail->next = cur;
		tail = cur;
	}
	free(pBilling);
	
	return TRUE;
}

// 查询卡号的未结算消费记录，返回链表中对应记录指针，并通过 pIndex 返回在文件中的索引（0基）
Billing* queryBilling(const char* pName, int* pIndex)
{
	if (pName == NULL) return NULL;

	//调用getBilling函数将文件中的消费记录保存在链表中
	if (FALSE == getBilling())
	{
		return NULL;
	}

	if (billingList == NULL) return NULL;//链表未初始化或读取失败

	lpBillingNode cur = billingList->next;//从链表头开始遍历
	int index = 0;
	Billing* found = NULL;//保存找到的记录指针
	int foundIndex = -1;//保存找到的记录在文件中的索引

	while (cur != NULL)//遍历链表
	{
		if (strcmp(cur->data.aCardName, pName) == 0 && cur->data.nStatus == 0 && cur->data.nDel == 0)
		{
			// 保存最近匹配（继续遍历以便找到最新的未结算）
			found = &cur->data;
			foundIndex = index;
		}
		cur = cur->next;
		index++;
	}

	if (found != NULL)
	{
		if (pIndex != NULL) *pIndex = foundIndex;//返回找到的记录在文件中的索引
		return found;//返回找到的记录指针
	}
	return NULL;

}


// 根据卡号和时间段查询消费记录。pCardName 为 NULL 表示不按卡过滤（查询所有）。
Billing* queryBillingByCardAndRange(const char* pCardName, time_t tStart, time_t tEnd, int* pCount)
{
	if (pCount == NULL) return NULL;
	*pCount = 0;

	int nCount = getBillingCount(BILLINGPATH);// 获取总记录数
	if (nCount <= 0) return NULL;

	Billing* all = (Billing*)malloc(sizeof(Billing) * nCount);
	if (all == NULL) return NULL;
	if (readBilling(all, BILLINGPATH) == FALSE)
	{
		free(all);
		return NULL;
	}

	// 暂存匹配项（最多 nCount），最后压缩
	Billing* matches = (Billing*)malloc(sizeof(Billing) * nCount);
	if (matches == NULL)
	{
		// 分配失败，释放已分配的 all 数组
		free(all);
		return NULL;
	}
	int m = 0;//匹配记录数
	for (int i = 0; i < nCount; i++)
	{
		// 只统计已完成消费且未删除的记录
		if (all[i].nStatus != 1 || all[i].nDel != 0) continue;

		time_t t = all[i].tLogoff;//优先使用下机时间作为消费时间
		if (t == 0) t = all[i].tLogon;//如果下机时间无效则使用上机时间

		if (tStart != 0 && t < tStart) continue;//如果 tStart 不为 0 则过滤掉早于 tStart 的记录
		if (tEnd != 0 && t > tEnd) continue;//如果 tEnd 不为 0 则过滤掉晚于 tEnd 的记录

		if (pCardName != NULL && pCardName[0] != '\0')
		{
			// 如果 pCardName 不为 NULL 且不为空字符串，则过滤掉卡号不匹配的记录
			if (strcmp(all[i].aCardName, pCardName) != 0) continue;
		}
		matches[m++] = all[i];
	}
	free(all);

	if (m == 0)
	{
		free(matches);
		*pCount = 0;
		return NULL;
	}

	// 压缩数组
	Billing* ret = (Billing*)realloc(matches, sizeof(Billing) * m);
	if (ret == NULL)
	{
		// realloc 失败仍可返回 matches
		ret = matches;
	}
	*pCount = m;
	return ret;
}

// 统计时间段内总营业额（已完成消费）
double getTotalTurnover(time_t tStart, time_t tEnd)
{
	double total = 0.0;
	int nCount = getBillingCount(BILLINGPATH);
	if (nCount <= 0) return 0.0;

	Billing* all = (Billing*)malloc(sizeof(Billing) * nCount);
	if (all == NULL) return 0.0;
	if (readBilling(all, BILLINGPATH) == FALSE)
	{
		// 读取失败，释放已分配的 all 数组
		free(all);
		return 0.0;
	}

	for (int i = 0; i < nCount; i++)
	{
		if (all[i].nStatus != 1 || all[i].nDel != 0) continue;//只统计已完成消费且未删除的记录
		time_t t = all[i].tLogoff;
		if (t == 0) t = all[i].tLogon;
		if (tStart != 0 && t < tStart) continue;
		if (tEnd != 0 && t > tEnd) continue;
		total += (double)all[i].fAmount;
	}
	free(all);
	return total;
}

// 获取指定年份每个月营业额（已完成消费），months[0] 对应 1 月
int getMonthlyTurnover(int year, double months[12])
{
	if (months == NULL) return FALSE;
	for (int i = 0; i < 12; i++) months[i] = 0.0;

	int nCount = getBillingCount(BILLINGPATH);
	if (nCount <= 0) return TRUE; // 无记录，视为成功但全部为 0

	Billing* all = (Billing*)malloc(sizeof(Billing) * nCount);
	if (all == NULL) return FALSE;
	if (readBilling(all, BILLINGPATH) == FALSE)
	{
		free(all);
		return FALSE;
	}

	for (int i = 0; i < nCount; i++)
	{
		if (all[i].nStatus != 1 || all[i].nDel != 0) continue;
		time_t t = all[i].tLogoff;
		if (t == 0) t = all[i].tLogon;
		if (t <= 0) continue;
		struct tm* tm = localtime(&t);
		if (tm == NULL) continue;
		if (tm->tm_year == year - 1900)
		{
			// tm_year 是从 1900 年开始的，所以需要减去 1900 来比较年份
			int mon = tm->tm_mon; // tm_mon 从 0 开始，0 表示 1 月，11 表示 12 月
			if (mon >= 0 && mon < 12)
			{
				// 累加到对应月份，注意 tm_mon 从 0 开始，所以直接使用 mon 作为索引
				months[mon] += (double)all[i].fAmount;
			}
		}
	}
	free(all);
	return TRUE;
}

// 统计时间段内充值(in)和退费(out)
int getCashFlow(time_t tStart, time_t tEnd, double* inSum, double* outSum)
{
	if (inSum == NULL || outSum == NULL) return FALSE;
	*inSum = 0.0;
	*outSum = 0.0;

	int nCount = getMoneyCount(MONEYPATH);
	if (nCount <= 0) return TRUE; // 没记录视为成功但为 0

	Money* all = (Money*)malloc(sizeof(Money) * nCount);
	if (all == NULL) return FALSE;
	if (readMoney(all, MONEYPATH) == FALSE)
	{
		free(all);
		return FALSE;
	}

	for (int i = 0; i < nCount; i++)
	{
		if (all[i].nDel != 0) continue;
		time_t t = all[i].tTime;
		if (tStart != 0 && t < tStart) continue;
		if (tEnd != 0 && t > tEnd) continue;
		if (all[i].nStatus == 0) // 充值
		{
			*inSum += (double)all[i].fAmount;
		}
		else if (all[i].nStatus == 1) // 退费
		{
			*outSum += (double)all[i].fAmount;
		}
	}
	free(all);
	return TRUE;
}