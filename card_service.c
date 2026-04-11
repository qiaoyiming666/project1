#define _CRT_SECURE_NO_WARNINGS

#include"model.h"
#include"global.h"
#include"card_service.h"
#include"card_file.h"
#include"billing_service.h"

#include<string.h>
#include<stdlib.h>
#include <stdio.h>
#include <time.h>

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

//释放（释放 head 之后的所有节点，但不释放 head 本身）
void releaseCardList()
{
	if (cardList == NULL)
	{
		return;
	}
	lpCardNode cur = cardList->next;
	while (cur != NULL)
	{
		lpCardNode tmp = cur->next;
		free(cur);
		cur = tmp;
	}
	cardList->next = NULL;
}

//统计卡数量
int addCard(Card card)
{
	int nIndex = 0;
	// 使用模糊查询检查是否有同名且未被删除的卡
	Card* matches = queryCards(card.aName, &nIndex);

	// queryCards 返回 NULL 且 nIndex == 0 表示读取失败
	if (matches == NULL && nIndex == 0)
	{
		printf("添加失败：无法读取卡信息文件。\n");
		return FALSE;
	}

	// 如果有匹配项，检查是否存在未删除的完全相同卡号
	if (matches != NULL && nIndex > 0)
	{
		for (int i = 0; i < nIndex; i++)
		{
			if (strcmp(matches[i].aName, card.aName) == 0 && matches[i].nDel == 0)
			{
				printf("添加失败：卡号已存在（%s）。\n", card.aName);
				free(matches);
				return FALSE;
			}
		}
	}

	// 释放可能分配的内存
	if (matches != NULL)
	{
		free(matches);
	}

	// 不存在有效重复时写入新卡记录（允许此前已删除的同名卡重新添加）
	return saveCard(&card, CARDPATH);
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

	if (nCount < 0)
	{
		// 打开文件失败
		fprintf(stderr, "getCard: 无法读取文件或打开失败: %s\n", CARDPATH);
		return FALSE;
	}

	if (nCount == 0)
	{
		// 文件存在但没有卡记录，确保链表清空并返回成功（表示没有数据）
		if (cardList != NULL)
		{
			cardList->next = NULL;
		}
		return TRUE;
	}


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
	
	//获取卡信息到pCard
	if(FALSE == readCard(pCard, CARDPATH))
	{
		free(pCard);
		pCard = NULL;
		return FALSE;
	}

	// 添加 cardList 非空检查，确保不会对 NULL 指针 cardList 进行解引用
	if (cardList == NULL)
	{
		// 如果 cardList 还未初始化，先初始化链表头
		if (!initCardList())
		{
			free(pCard);
			return FALSE;
		}
	}

	// 将节点追加到链表尾，保持链表顺序与文件记录顺序一致（避免文件索引/链表索引不匹配）
	lpCardNode tail = cardList;
	while (tail->next != NULL)
	{
		tail = tail->next;
	}

	for(int i=0; i < nCount; i++)
	{
		lpCardNode cur = (lpCardNode)malloc(sizeof(CardNode));
		if(cur == NULL)
		{
			free(pCard);
			fprintf(stderr, "getCard: 链表节点分配失败\n");
			return FALSE;
		}
		//初始化新空间
		memset(cur, 0, sizeof(CardNode));

		//将卡信息保存在链表中（追加到尾部）
		cur->data = pCard[i];
		cur->next = NULL;

		tail->next = cur;
		tail = cur;
	}
	free(pCard);
	pCard = NULL;
	return TRUE;
}

//上机
int checkCard(const char* pName, const char* pPwd, LogonInfo* pInfo)
{
	if (pName == NULL || pPwd == NULL)
	{
		//返回登录失败状态码
		return LOGONFAILURE;
	}

	lpCardNode cardNode = NULL;
	int nIndex = 0;         //上机卡在卡信息链表的索引
	Billing billing = { 0 };//保存消费记录信息
	time_t now = time(NULL); //获取当前时间

	//获取文件中的卡信息
	if(FALSE == getCard())
	{
		// 获取卡信息失败，返回登录失败状态码
		return LOGONFAILURE;
	}
	cardNode = cardList->next;
	//遍历链表，判断能否进行上机
	while (cardNode != NULL)
	{
		if (strcmp(cardNode->data.aName, pName) == 0 && strcmp(cardNode->data.aPwd, pPwd) == 0)
		{
			//只有未在使用的卡才能进行上机操作
			if(cardNode->data.nStatus!=0)
			{
				//卡已在使用或不可用，返回登录失败状态码
				return LOGONFAILURE;
			}
			//只有余额大于0的卡才能进行上机操作
			if(cardNode->data.fBalance <= 0)
			{
				//余额不足，返回余额不足状态码
				return BALANCEINSUFFICIENT;
			}

			//备份原始数据以便返回
			Card oldCard = cardNode->data;

			//更新链表中的卡信息
			cardNode->data.nStatus = 1;
			cardNode->data.nUseCount++;
			cardNode->data.tLastUse = now;
			//如果能进行上机操作，更新卡信息
			
			// 持久化到文件
			if (!updateCard(&cardNode->data, CARDPATH, nIndex))
			{
				// 持久化失败，恢复内存
				cardNode->data = oldCard;
				return LOGONFAILURE;
			}

			// 填充并添加消费记录
			memset(&billing, 0, sizeof(billing));
			strncpy(billing.aCardName, pName, sizeof(billing.aCardName) - 1);
			billing.tLogon = now;
			billing.tLogoff = 0;
			billing.fAmount = 0.0f;
			billing.nStatus = 0;
			billing.nDel = 0;

			if (!addBilling(billing))
			{
				// 添加消费记录失败：尝试回滚卡文件与内存（将卡信息恢复到 oldCard）
				cardNode->data = oldCard;
				// 尝试写回旧记录（若失败，无法保证文件一致性，但仍尽力回滚）
				updateCard(&oldCard, CARDPATH, nIndex);
				return LOGONFAILURE;
			}

			// 成功：将上机信息写入输出参数（如果提供）
			if (pInfo != NULL)
			{
				memset(pInfo, 0, sizeof(*pInfo));
				strncpy(pInfo->aCardName, pName, sizeof(pInfo->aCardName) - 1);
				pInfo->tLogon = billing.tLogon;
				pInfo->fBalance = cardNode->data.fBalance;
			}

			return LOGONSUCCESS;
		}
		cardNode = cardNode->next;
		nIndex++;
	}
	// 未找到卡
	return CARDNOTFOUND;
} 

// 在文件中查找卡（按卡号和密码），返回堆上分配的 Card*，并通过 pIndex 返回文件记录索引。
// 成功返回非 NULL（调用者负责 free 返回指针）；失败返回 NULL。
Card* searchCard(const char* pName, const char* pPwd, int* pIndex)
{
	if (pName == NULL || pPwd == NULL || pIndex == NULL)
	{
		return NULL;
	}

	int count = getCardCount(CARDPATH);
	if (count <= 0)
	{
		return NULL;
	}

	Card* all = (Card*)malloc(sizeof(Card) * (size_t)count);
	if (all == NULL)
	{
		return NULL;
	}

	if (readCard(all, CARDPATH) == FALSE)
	{
		free(all);
		return NULL;
	}

	for (int i = 0; i < count; i++)
	{
		// 只匹配未删除记录
		if (strcmp(all[i].aName, pName) == 0 && strcmp(all[i].aPwd, pPwd) == 0 && all[i].nDel == 0)
		{
			Card* result = (Card*)malloc(sizeof(Card));
			if (result != NULL)
			{
				memcpy(result, &all[i], sizeof(Card));
				*pIndex = i;
				free(all);
				return result;
			}
			break;
		}
	}

	free(all);
	return NULL;
}

// 新增：修改卡信息实现
int modifyCard(const char* oldName, const char* oldPwd, const char* newName, const char* newPwd, int modifyName, int modifyPwd)
{
	if (oldName == NULL || oldPwd == NULL)
	{
		return FALSE;
	}

	int nIndex = -1;
	// 使用 searchCard 获取文件索引与卡记录（searchCard 返回堆上分配的 Card*）
	Card* pFound = searchCard(oldName, oldPwd, &nIndex);
	if (pFound == NULL || nIndex < 0)
	{
		if (pFound) free(pFound);
		return FALSE; // 未找到或密码错误
	}

	// 检查要修改的内容是否合法
	Card tmp = *pFound; // 工作副本，便于回滚
	// 修改账号名时检查长度和重复（允许修改为相同名字）
	if (modifyName)
	{
		if (newName == NULL || getSize(newName) < 1 || getSize(newName) > (int)sizeof(tmp.aName) - 1)
		{
			free(pFound);
			return FALSE;
		}
		// 如果新名字与旧名字不同，检查重复
		if (strcmp(newName, pFound->aName) != 0)
		{
			Card* exist = queryCard(newName);
			if (exist != NULL && exist->nDel == 0)
			{
				// 已存在有效同名账号
				return FALSE;
				free(pFound);
			}
		}
		strncpy(tmp.aName, newName, sizeof(tmp.aName) - 1);
		tmp.aName[sizeof(tmp.aName) - 1] = '\0';
	}

	// 修改密码时检查长度
	if (modifyPwd)
	{
		if (newPwd == NULL || getSize(newPwd) < 1 || getSize(newPwd) > (int)sizeof(tmp.aPwd) - 1)
		{
			free(pFound);
			return FALSE;
		}
		strncpy(tmp.aPwd, newPwd, sizeof(tmp.aPwd) - 1);
		tmp.aPwd[sizeof(tmp.aPwd) - 1] = '\0';
	}

	// 执行写回（按索引覆盖文件记录）
	if (!updateCard(&tmp, CARDPATH, nIndex))
	{
		// 写回失败
		free(pFound);
		return FALSE;
	}

	free(pFound);
	return TRUE;
}
