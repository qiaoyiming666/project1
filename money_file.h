#ifndef MONEY_FILE_H
#define MONEY_FILE_H

#include "model.h"

// 将充值/退费记录保存到文本文件（追加）
// 返回 TRUE 成功，FALSE 失败
int saveMoney(const Money* pMoney, const char* pPath);

// 从文件读取所有 money 记录到 pMoney（数组），按文件行序填充
// 返回 TRUE 成功，FALSE 失败（注意：调用前应分配足够大内存，通常配合 getMoneyCount 使用）
int readMoney(Money* pMoney, const char* pPath);

// 获取 money 文件中有效记录数（去空行），失败返回 -1
int getMoneyCount(const char* pPath);

#endif // !1
