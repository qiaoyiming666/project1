#ifndef CARD_SERVICE_H
#define CARD_SERVICE_H

int addCard(Card card);     //统计卡数量
Card* queryCard(const char* pName);//查询卡，并调出卡的信息
Card* queryCards(const char* pName, int* pIndex);//模糊查询
void releaseCardList();//释放内存

#endif