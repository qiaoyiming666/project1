#define _CRT_SECURE_NO_WARNINGS


#include"card_service.h"
#include"card_file.h"
#include"billing_service.h"
#include"billing_file.h"
#include "global.h"
#include "model.h"
#include "money_file.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include<stdlib.h>

double calculateAmount(time_t tlogon, time_t tlogoff);

// 添加卡
int addCardInfo(Card card)
{
	return addCard(card);
}

// 模糊查询
Card* queryCardsInfo(const char* pName, int* pIndex)
{
	return queryCards(pName, pIndex);
}
// 上机
// 处理 checkCard 的新返回值并把上机信息返回给调用方
int doLogonInfo(const char* pName, const char* pPwd, LogonInfo* pInfo)
{
	if (pName == NULL || pPwd == NULL)
	{
		return LOGONFAILURE;
	}
	LogonInfo localInfo;
	// checkCard 会在成功时填充 localInfo 并写入消费记录
	int rc = checkCard(pName, pPwd, &localInfo);
	switch (rc)
	{
	case LOGONSUCCESS:
		if (pInfo != NULL)
		{
			*pInfo = localInfo; // 结构体直接拷贝
		}
		return LOGONSUCCESS;

	case CARDNOTFOUND:
		// 卡不存在
		return CARDNOTFOUND;

	case BALANCEINSUFFICIENT:
		// 余额不足
		return BALANCEINSUFFICIENT;

	default:
		// 其他错误（写文件失败等）
		return LOGONFAILURE;
	}
	//判断卡号密码是否正确
	//checkCard(pName, pPwd);
	//判断能否进行上机操作

	//更新卡信息(链表和文件)

	//添加消费记录
	//saveBilling(&billing, BILLINGPATH);
}

// 下机
int doLogoffInfo(const char* pName,const char* pPwd,logoffInfo* pInfo)
{
	Card* pCard = NULL;
	Billing* pBilling = NULL;
	int nIndex = 0;//保存查询到的卡信息的索引
	int nPosition = 0;//保存查询到的消费记录信息的索引
	double dbAmount = 0.0;//消费金额
	float fBalance = 0.0;//余额

	//查询上机信息，只有上机的卡才能进行下机操作
	pCard = searchCard(pName, pPwd, &nIndex);

	if(pCard == NULL)
	{
		//未找到上机信息，返回下机失败状态码
		return LOGOFFFAILURE;
	}
	if(pCard->nStatus != 1)
	{
		//卡未在使用，返回下机失败状态码
		free(pCard);
		return UNUSED;	
	}
	//获取计费信息
	pBilling = queryBilling(pName, &nPosition);
	if(pBilling == NULL || pBilling->nStatus != 0)
	{
		//未找到计费信息或计费信息状态不为未结算，返回下机失败状态码
		free(pCard);
		return LOGOFFFAILURE;
	}
	//计算消费金额
	dbAmount = calculateAmount(pBilling->tLogon, time(NULL));
	// 判断余额是否足够支付消费金额，如果不足，返回余额不足状态码
	fBalance = pCard->fBalance - dbAmount;
	if (fBalance < 0)
	{
		free(pCard);
		return BALANCEINSUFFICIENT;
	}
	//更新卡信息(链表和文件)
	if (pInfo != NULL)
	{
		strcpy(pInfo->aCardName, pName);//保存卡号
		pInfo->fAmount = dbAmount;//保存消费金额
		pInfo->fBalance = fBalance;//保存余额
		pInfo->tLogoff = time(NULL);//保存下机时间
	}

	pCard->fBalance = fBalance;//更新余额
	pCard->nStatus = 0;//更新卡状态为未使用
	pCard->tLastUse = time(NULL);//更新最后使用时间

	updateCard(pCard, CARDPATH, nIndex);

	//更新消费记录（结算）
	pBilling->fAmount = (float)dbAmount; // 显式转换
	pBilling->tLogoff = time(NULL);//更新下机时间
	pBilling->nStatus = 1;//更新状态为已结算

	updateBilling(pBilling, BILLINGPATH, nPosition);

	free(pCard);
	return TRUE;
}
// 计算消费金额的函数，单位时间起算，每单位时间单位价格，不足单位时间按单位时间计算
double calculateAmount(time_t tlogon, time_t tlogoff)
{
	// 计算消费金额的函数，单位时间起算，每单位时间单位价格，不足单位时间按单位时间计算
	float ratePerUnitTime = UNITPRICE; // 每单位时间的费用
	double durationInSeconds = difftime(tlogoff, tlogon);// 计算上机持续时间（秒）
	int units = (int)(durationInSeconds / UNITTIME);// 计算完整的单位时间数量
	if (durationInSeconds > 0 && (durationInSeconds - units * UNITTIME) > 0)
	{
		units++; // 不足一个单位时间按一个单位时间计算
	}
	return units * ratePerUnitTime;
}
// 充值
int doAddBalance(const char* pName,const char* pPwd, float fAmount)
{
	Card* pCard = NULL;
	int nIndex = 0;//保存查询到的卡信息的索引
	//查询卡信息，只有未删除的卡才能进行充值操作

	// 使用 searchCard 获取卡信息和文件索引
	pCard = searchCard(pName, pPwd, &nIndex);

	if(pCard == NULL || pCard->nDel != 0)
	{
		//未找到卡信息或卡已删除，返回充值失败状态码
		if (pCard) free(pCard);
		return FALSE;
	}
	if(pCard->nStatus == 1)
	{
		//卡正在使用，返回充值失败状态码
		free(pCard);
		return FALSE;	
	}
	//更新卡信息(链表和文件)
	pCard->fBalance += fAmount;//更新余额
	updateCard(pCard, CARDPATH, nIndex);
	free(pCard);
	return TRUE;
}

// 充值（业务层，记录充值日志并更新卡余额）
// pMoneyInfo 用于传入充值金额（pMoneyInfo->fAmount）并返回更新后的余额（pMoneyInfo->fBalance）
int doAddMoney(const char* pName, const char* pPwd, MoneyInfo* pMoneyInfo)
{
	if (pName == NULL || pPwd == NULL || pMoneyInfo == NULL)
	{
		return FALSE;
	}

	if (pMoneyInfo->fAmount <= 0.0f)
	{
		return FALSE;
	}

	Card* pCard = NULL;
	int nIndex = 0;

	pCard = searchCard(pName, pPwd, &nIndex);

	if (pCard == NULL || pCard->nDel != 0)
	{
		if (pCard) free(pCard);
		return FALSE;
	}
	if (pCard->nStatus == 1)
	{
		// 卡正在使用，不允许充值
		free(pCard);
		return FALSE;
	}
	// 更新卡余额
	pCard->fBalance += pMoneyInfo->fAmount;
	if (!updateCard(pCard, CARDPATH, nIndex))
	{
		free(pCard);
		return FALSE;
	}
	// 写充值记录
	Money money = { 0 };
	strncpy(money.aCardName, pName, sizeof(money.aCardName) - 1);
	money.tTime = time(NULL);
	money.nStatus = 0; // 0-充值
	money.fAmount = pMoneyInfo->fAmount;
	money.nDel = 0;

	if (!saveMoney(&money, MONEYPATH))
	{
		// 写日志失败，尝试回滚卡余额（回滚写失败时仍尽力保持一致性）
		pCard->fBalance -= pMoneyInfo->fAmount;
		updateCard(pCard, CARDPATH, nIndex);
		free(pCard);
		return FALSE;
	}

	// 返回最新余额
	pMoneyInfo->fBalance = pCard->fBalance;
	strncpy(pMoneyInfo->aCardName, pName, sizeof(pMoneyInfo->aCardName) - 1);

	free(pCard);
	return TRUE;
}

//退款
int doRefund(const char* pName,const char* pwd, float fAmount)
{
	Card* pCard = NULL;
	int nIndex = 0;//保存查询到的卡信息的索引
	//查询卡信息，只有未删除的卡才能进行退款操作

	pCard = searchCard(pName, pwd, &nIndex);
	if(pCard == NULL || pCard->nDel != 0)
	{
		//未找到卡信息或卡已删除，返回退款失败状态码
		if (pCard) free(pCard);
		return FALSE;
	}
	if(pCard->nStatus == 1)
	{
		//卡正在使用，返回退款失败状态码
		free(pCard);
		return FALSE;	
	}
	//更新卡信息(链表和文件)
	pCard->fBalance -= fAmount;//更新余额
	updateCard(pCard, CARDPATH, nIndex);
	free(pCard);
	return TRUE;
}

//退款（业务层，记录退费日志并更新卡余额）
// pMoneyInfo 用于传入退费金额（pMoneyInfo->fAmount）并返回更新后的余额（pMoneyInfo->fBalance）
int doRefundMoney(const char* pName, const char* pPwd, MoneyInfo* pMoneyInfo)
{
	if (pName == NULL || pPwd == NULL || pMoneyInfo == NULL)
	{
		return FALSE;
	}

	if (pMoneyInfo->fAmount <= 0.0f)
	{
		return FALSE;
	}

	Card* pCard = NULL;
	int nIndex = 0;

	pCard = searchCard(pName, pPwd, &nIndex);

	if (pCard == NULL || pCard->nDel != 0)
	{
		if (pCard) free(pCard);
		return FALSE;
	}
	if (pCard->nStatus == 1)
	{
		// 卡正在使用，不允许退费
		free(pCard);
		return FALSE;
	}

	// 确保余额足够退费
	if (pCard->fBalance < pMoneyInfo->fAmount)
	{
		free(pCard);
		return FALSE;
	}

	// 更新卡余额
	pCard->fBalance -= pMoneyInfo->fAmount;
	if (!updateCard(pCard, CARDPATH, nIndex))
	{
		free(pCard);
		return FALSE;
	}

	// 写退费记录
	Money money = { 0 };
	strncpy(money.aCardName, pName, sizeof(money.aCardName) - 1);
	money.tTime = time(NULL);
	money.nStatus = 1; // 1-退费
	money.fAmount = pMoneyInfo->fAmount;
	money.nDel = 0;

	if (!saveMoney(&money, MONEYPATH))
	{
		// 写日志失败，尝试回滚余额变更
		pCard->fBalance += pMoneyInfo->fAmount;
		updateCard(pCard, CARDPATH, nIndex);
		free(pCard);
		return FALSE;
	}

	// 返回最新余额
	pMoneyInfo->fBalance = pCard->fBalance;
	strncpy(pMoneyInfo->aCardName, pName, sizeof(pMoneyInfo->aCardName) - 1);

	free(pCard);
	return TRUE;
}

// 注销卡：输入参数 pCard 必须包含 aName 和 aPwd，函数将尝试按文件中第一条匹配记录注销
// 注销成功返回 TRUE，失败返回 FALSE。成功时会将 pCard->fBalance 置为退款金额并填充 aName。

int annulCard(Card* pCard)
{
	if (pCard == NULL || pCard->aName[0] == '\0' || pCard->aPwd[0] == '\0')
	{
		return FALSE;
	}

	int nIndex = -1;

	// 在文件中查找卡（返回堆上分配的 Card*)
	Card* pFound = searchCard(pCard->aName, pCard->aPwd, &nIndex);
	if (pFound == NULL || nIndex < 0)
	{
		if (pFound) free(pFound);
		return FALSE; // 未找到
	}

	// 只有未上机（nStatus == 0）才能注销
	if (pFound->nStatus != 0)
	{
		free(pFound);
		return FALSE;
	}

	// 记录要退款的金额（原余额）
	float fRefund = pFound->fBalance;

	// 修改记录：状态置为 已注销（2），余额置为 0
	Card tmp = *pFound;
	tmp.nStatus = 2;     // 已注销
	tmp.fBalance = 0.0f;

	// 更新卡文件（按索引覆盖）
	if (!updateCard(&tmp, CARDPATH, nIndex))
	{
		free(pFound);
		return FALSE;
	}

	// 写退款日志（money 文件），nStatus = 1 表示退费
	Money money = { 0 };
	strncpy(money.aCardName, pFound->aName, sizeof(money.aCardName) - 1);
	money.tTime = time(NULL);
	money.nStatus = 1; // 1-退费
	money.fAmount = fRefund;
	money.nDel = 0;

	if (!saveMoney(&money, MONEYPATH))
	{
		// 如果日志写入失败，尝试回滚卡记录（尽最大努力）
		updateCard(pFound, CARDPATH, nIndex); // 恢复原记录（忽略返回值）
		free(pFound);
		return FALSE;
	}

	// 将结果返回给调用者：退款金额和卡号
	pCard->fBalance = fRefund;
	strncpy(pCard->aName, tmp.aName, sizeof(pCard->aName) - 1);

	free(pFound);
	return TRUE;
}

void releaseList()
{
	releaseCardList();
	releaseBillingList();
}