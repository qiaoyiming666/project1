#ifndef GLOBAL_H
#define GLOBAL_H

#define FALSE 0
#define TRUE 1
#define TIMELENGTH 20 //时间字符长度
#define CARDCHARNUM 256 //卡信息字符串长度

#define CARDPATH "data\\card.ams" //卡信息文件路径
#define BILLINGPATH "data\\billing.ams" //消费记录文件路径
#define MONEYPATH "data\\money.ams" //充值记录文件路径

#define LOGONFAILURE 0 //上机失败
#define LOGONSUCCESS 1 //上机成功
#define CARDNOTFOUND 2 //未找到卡
#define BALANCEINSUFFICIENT 3 //余额不足
#define PWDERROR 4 // 密码错误

#define LOGOFFFAILURE 0 //下机失败
#define LOGOFFSUCCESS 1 //下机成功
#define UNUSED 2 //未使用

//单位时间：15分钟，每单位时间的费用：0.5元
#define UNITPRICE 0.5f //每单位时间的费用
#define UNITTIME 15*60 //单位时间对应的秒数

#endif