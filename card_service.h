#include "model.h"
#ifndef CARD_SERVICE_H
#define CARD_SERVICE_H

int addCard(Card card);     //统计卡数量
Card* queryCard(const char* pName);//查询卡，并调出卡的信息
Card* queryCards(const char* pName, int* pIndex);//模糊查询
int initCardList();//初始化链表
void releaseCardList();//释放内存
int getCard();//将文件中的卡信息保存在链表中

//Card* checkCard(const char* pName, const char* pPwd);//上机
double calculateAmount(time_t tlogon, time_t tlogoff);
// 将 checkCard 改为返回状态码，并通过输出参数返回上机信息（LogonInfo）
int checkCard(const char* pName, const char* pPwd, LogonInfo * pInfo);
//
Card* searchCard(const char* pName, const char* pPwd, int* pIndex);
#endif
