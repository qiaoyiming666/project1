
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