#ifndef CARD_SERVICE_H
#define CARD_SERVICE_H

int addCard(Card card);     //统计卡数量
Card* queryCard(const char* pName);//查询卡，并调出卡的信息

#endif