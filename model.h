#ifndef MODEL_H
#define MODEL_H


#include<time.h> 

//卡信息结构体
typedef struct Card
{
	char aName[18];  //卡号
	char aPwd[8];    //密码 

	int nStatus;     //卡状态（0-未使用；1-正在使用；2-已注销；3-失效）
	int nUseCount;   //使用次数
	float fBalance;  //余额
	float fTotalUse;    //累计使用金额
	int nDel;        //删除标记（0-未删除；1-删除）

	time_t tStart;   //开卡时间
	time_t tEnd;     //截止时间
	time_t tLastUse; //最后使用时间 
}Card; 

//链表节点结构体
typedef struct CardNode
{
	Card data;
	struct CardNode* next;
}CardNode, * lpCardNode;

//消费记录结构体
typedef struct Billing
{
	char aCardName[18]; //卡号
	time_t tLogon;    //上机时间
	time_t tLogoff;   //下机时间
	float fAmount;     //消费金额
	int nStatus;       //状态（0-未结算；1-已结算）
	int nDel;         //删除标记（0-未删除；1-删除）
}Billing;
//上机信息结构体
typedef struct logonInfo
{
	char aCardName[18]; //卡号
	time_t tLogon;    //上机时间
	float fBalance;  //余额
}LogonInfo;
//下机信息结构体
typedef struct logoffInfo
{
	char aCardName[18]; //卡号
	time_t tLogoff;   //下机时间
	float fAmount;     //消费金额
	float fBalance;  //余额
}logoffInfo;
//消费记录链表节点结构体
typedef struct BillingNode
{
	Billing data;
	struct BillingNode* next;
}BillingNode, * lpBillingNode;
//充值退费记录结构体
typedef struct Money
{
	char aCardName[18]; //卡号
	time_t tTime;    //时间
	int nStatus;       //状态（0-充值；1-退费）
	float fAmount;     //金额
	int nDel;         //删除标记（0-未删除；1-删除）
}Money;

typedef struct MoneyInfo
{
	char aCardName[18]; //卡号

	float fAmount;     //金额
	float fBalance;  //余额
}MoneyInfo;

#endif