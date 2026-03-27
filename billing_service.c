
#include<stdlib.h>
#include<string.h>
#include<time.h>

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
		if (billingList->next != NULL)
		{
			cur = billingList->next;
			free(cur);
			cur = NULL;
		}
	}
}

int getBilling()
{
	//获取文件中的消费记录信息,将文件中的消费记录信息保存在链表中
	return FALSE;
}

//查询消费记录
Billing* queryBilling(const char* pName, int* pIndex)
{
	getBilling();

	//根据卡号查询消费记录

}