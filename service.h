#ifndef SERVICE_H
#define SERVICE_H

int addCardInfo(Card card);
Card* queryCardsInfo(const char* pName, int* pIndex);

// doLogonInfo 现在返回状态码，并通过 pInfo 输出上机信息（如果成功）
int doLogonInfo(const char* pName, const char* pPwd, LogonInfo* pInfo);
int doLogoffInfo(const char* pName, const char* pPwd, logoffInfo* pInfo);

void releaseList();
#endif