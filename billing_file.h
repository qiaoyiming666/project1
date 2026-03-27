
#include"model.h"

#ifndef BILLING_FILE_H
#define BILLING_FILE_H

int saveBilling(const Billing* pBilling, const char* pPath); //将信息保存到billing.ams文件中
int readBilling(Billing* pBilling, const char* pPath);
int getBillingCount(const char* pPath);
#endif