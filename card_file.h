#include "model.h"
#ifndef CARD_FILE_H
#define CARD_FILE_H

int saveCard(const Card* pcard, const char* pPath);//存数据
int readCard(Card* pCard, const char* pPath);//读数据
Card praseCard(const char* pBuf);//解析函数
int getCardCount(const char* pPath);//读取卡数量

// 更新文件中对应卡号的记录（根据 Card.aName 匹配），成功返回 TRUE
int updateCard(const Card* pCard, const char* pPath, int nIndex);

// 保存一条消费记录到 data\billing.ams（追加），返回 TRUE 表示成功
//int saveBilling(const char* pCardNo, time_t tLogon, time_t tLogoff, float amount, int status, int nDel);

#endif