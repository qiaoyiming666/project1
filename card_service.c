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

//释放
void releaseCardList()
{
	lpCardNode cur;
	if (cardList != NULL)
	{
		if (cardList->next != NULL)
		{
			cur = cardList->next;
			free(cur);
			cur = NULL;
		}
	}
}

//统计卡数量
int addCard(Card card)
{
	lpCardNode cur = NULL;

	if (cardList == NULL)
	{
		initCardList();
	}

	//将数据保存到节点中
	cur = (lpCardNode)malloc(sizeof(CardNode));
	if (cur != NULL)
	{
		cur->data = card;
		cur->next = NULL;
		//遍历找到最后一个节点
		while (cardList->next != NULL)
		{
			cardList = cardList->next;
		}
		cardList->next = cur;
		return TRUE;
	}

	return FALSE;
}

//查询卡，并调出卡的信息
Card* queryCard(const char* pName)
{
	lpCardNode cur = NULL;
	if (cardList != NULL)
	{
		cur = cardList->next;
		while (cur != NULL)
		{
			if (strcmp(cur->data.aName, pName) == 0)
			{
				return &cur->data;
			}
			cur = cur->next;
		}
	}
	return NULL;
}
//模糊查询
Card* queryCards(const char* pName, int* pIndex)
{
	lpCardNode cur = NULL;
	Card* pCard = (Card*)malloc(sizeof(Card));
	if (pCard == NULL)
	{
		return NULL;
	}
	if (cardList != NULL)
	{
		cur = cardList->next;
		while (cur != NULL)
		{
			if (strstr(cur->data.aName, pName) != NULL)
			{
				pCard[*pIndex] = cur->data;
				(*pIndex)++;

				pCard = (Card*)realloc(pCard, sizeof(Card) * ((*pIndex) + 1));
			}
			cur = cur->next;
		}
	}
	return pCard;
}