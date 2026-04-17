
#include"model.h"

#ifndef BILLING_SERVICE_H
#define BILLING_SERVICE_H

int addBilling(Billing billing); //添加消费记录

void initBillingList(); //初始化链表
void releaseBillingList(); //释放内存

Billing* queryBilling(const char* pCardName, int* pIndex); //查询消费记录


// 根据卡号（如果 pCardName 为 NULL 表示所有卡）和时间段查询消费记录，返回动态数组，
// *pCount 设置为匹配记录数。调用方需 free() 返回值。
Billing* queryBillingByCardAndRange(const char* pCardName, time_t tStart, time_t tEnd, int* pCount);

// 统计时间段内总营业额（只统计已完成消费 nStatus==1 且未删除 nDel==0）
double getTotalTurnover(time_t tStart, time_t tEnd);

// 统计指定年份每个月的营业额，months 数组长度至少 12，返回 TRUE/FALSE
int getMonthlyTurnover(int year, double months[12]);

// 统计时间段内的现金流：充值（inSum）与退费（outSum），返回 TRUE/FALSE
int getCashFlow(time_t tStart, time_t tEnd, double* inSum, double* outSum);

#endif