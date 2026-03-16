#include"model.h"
#include"global.h"
#include"card_service.h"
#include"card_file.h"

#include<string.h>
#include<stdlib.h>
 

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
	return saveCard(&card, CARDPATH);
	return FALSE;
}

//查询卡，并调出卡的信息
Card* queryCard(const char* pName)
{
	lpCardNode cur = NULL;

	//调用getCard函数将文件中的卡信息保存在链表中
	if (FALSE == getCard())
	{
		return FALSE;
	}

	if (cardList != NULL)
	{
		//从链表头开始遍历
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

	//调用getCard函数将文件中的卡信息保存在链表中
	if (FALSE == getCard())
	{
		return FALSE;
	}
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
				if (pCard == NULL)
				{
					return NULL;
				}
			}
			cur = cur->next;
		}
	}
	return pCard;
}

//将文件中的卡信息保存在链表中
int getCard()
{
	//获取卡信息数据
	int nCount = getCardCount(CARDPATH);

	//释放链表内存
	if (cardList != NULL)
	{
		releaseCardList();
	}

	//动态分配内存保存卡信息
	Card* pCard = (Card*)malloc(sizeof(Card) * nCount);
	if(pCard == NULL)
	{
		return FALSE;
	}
	
	//获取卡信息
	if(FALSE == readCard(pCard, CARDPATH))
	{
		free(pCard);
		pCard = NULL;
		return FALSE;
	}

	for(int i=0; i < nCount; i++)
	{
		lpCardNode cur = (lpCardNode)malloc(sizeof(CardNode));
		if(cur == NULL)
		{
			return FALSE;
		}
		//初始化新空间
		memset(cur, 0, sizeof(CardNode));

		//将卡信息保存在链表中
		cur->data = pCard[i];
		cur->next = cardList->next;

		cardList->next = cur;
		cardList = cur;
	}
	free(pCard);
	pCard = NULL;
	return TRUE;

}
