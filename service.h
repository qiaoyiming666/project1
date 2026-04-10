#ifndef SERVICE_H
#define SERVICE_H

int addCardInfo(Card card);//添加卡信息

Card* queryCardsInfo(const char* pName, int* pIndex);//模糊查询

// doLogonInfo 现在返回状态码，并通过 pInfo 输出上机信息（如果成功）
int doLogonInfo(const char* pName, const char* pPwd, LogonInfo* pInfo);
// doLogoffInfo 现在返回状态码，并通过 pInfo 输出下机信息（如果成功）
int doLogoffInfo(const char* pName, const char* pPwd, logoffInfo* pInfo);

// 计算消费金额的函数，单位时间起算，每单位时间单位价格，不足单位时间按单位时间计算
double calculateAmount(time_t tlogon, time_t tlogoff);

int doAddBalance(const char* pName, const char* pwd, float fAmount);//充值
int doAddMoney(const char* pName, const char* pwd, MoneyInfo* pMoneyInfo);//充值（业务层，记录充值日志并更新卡余额）

int doRefund(const char* pName, const char* pwd, float fAmount);//退费
int doRefundMoney(const char* pName, const char* pwd, MoneyInfo* pMoneyInfo);//退费（业务层，记录退费日志并更新卡余额）

int annulCard(Card* pCard); // 注销卡：符合条件时退还余额并将卡状态置为已注销

void releaseList();//释放链表内存
#endif