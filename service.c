
#include"card_service.h"
#include"billing_service.h"
#include"billing_file.h"
#include "global.h"
#include "model.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

//添加卡
int addCardInfo(Card card)
{
	return addCard(card);
}

//模糊查询
Card* queryCardsInfo(const char* pName, int* pIndex)
{
	return queryCards(pName, pIndex);
}
//上机
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

//下机
int doLogoffInfo(const char* pName,const char* pwd,logoffInfo* pInfo)
{
	Card* pCard = NULL;
	Billing* pBilling = NULL;
	int nIndex = 0;//保存查询到的卡信息的索引
	int nPosition = 0;//保存查询到的消费记录信息的索引
	double dbAmount = 0.0;//消费金额
	float fBalance = 0.0;//余额

	//查询上机信息，只有上机的卡才能进行下机操作
	pCard = 

	//获取计费信息
	//计算消费金额
	// 判断余额是否足够支付消费金额，如果不足，返回余额不足状态码
	//更新卡信息(链表和文件)
	//更新消费记录（结算）
	return 0;
}

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


void releaseList()
{
	releaseCardList();
	releaseBillingList();
}