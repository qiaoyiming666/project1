#include "model.h"
#ifndef CARD_FILE_H
#define CARD_FILE_H

int saveCard(const Card* pcard, const char* pPath);//存数据
int readCard(Card* pCard, const char* pPath);//读数据
Card praseCard(const char* pBuf);//解析函数
int getCardCount(const char* pPath);//读取卡数量
#endif