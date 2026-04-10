
#include"model.h"

#ifndef BILLING_FILE_H
#define BILLING_FILE_H

int saveBilling(const Billing* pBilling, const char* pPath); //将信息保存到billing.ams文件中
int readBilling(Billing* pBilling, const char* pPath);//从billing.ams文件中读取信息
int getBillingCount(const char* pPath);//获取billing.ams文件中的记录数
int updateBilling(const Billing* pBilling, const char* pPath, int nIndex);//更新billing.ams文件中的指定记录
#endif