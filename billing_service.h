
#include"model.h"

#ifndef BILLING_SERVICE_H
#define BILLING_SERVICE_H

int addBilling(Billing billing); //添加消费记录

void initBillingList(); //初始化链表
void releaseBillingList(); //释放内存

Billing* queryBilling(const char* pCardName, int* pIndex); //查询消费记录

#endif