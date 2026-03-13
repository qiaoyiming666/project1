#include"model.h"
#include"global.h"

#include<string.h>
#include<stdlib.h>
 
Card aCard[50];   //卡信息结构体数组
int nCount = 0;   //卡信息实际个数

lpCardNode cardList = NULL;

//初始化链表
int initCardList()
{
	lpCardNode head = NULL;
	head = (lpCardNode)malloc(sizeof(CardNode));

	if (head != NULL)
	{
		head->next = NULL;
		cardList = head;
		return TRUE;
	}
	return FALSE;
}

int addCard(Card card)//统计卡数量
{
	aCard[nCount] = card;
	nCount++;
	return TRUE;
}

Card* queryCard(const char* pName)//查询卡，并调出卡的信息
{
	for (int i = 0; i < nCount;i++)
	{
		if (strcmp(pName, aCard[i].aName) == 0)
		{
			return &aCard[i];
		}
	}
}
