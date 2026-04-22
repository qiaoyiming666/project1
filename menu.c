#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

#include"model.h"
#include"card_service.h"
#include"billing_service.h"
#include"service.h"
#include"tool.h"
#include"global.h"
#include"menu.h"


//输出菜单
void outputMenu() 
{
	printf("---------------菜单---------------\n");
	printf("1.添加卡\n");
	printf("2.查询卡\n");
	printf("3.上机\n");
	printf("4.下机\n");
	printf("5.充值\n");
	printf("6.退费\n");
	printf("7.查询系统\n");
	printf("8.注销卡\n");
	printf("9.修改账号名或密码\n");
	printf("0.退出\n");
	//提示选择菜单编号
	printf("请选择菜单编号（0~9）：");
}
// 将数值状态转换为可读文本
static const char* statusToString(int status)
{
    switch (status)
    {
    case 0: return "未使用";
    case 1: return "正在使用";
    case 2: return "已注销";
    case 3: return "失效";
    default: return "未知";
    }
}
//添加卡
void add()
{
	Card card;
    char aName[18] = { '\0' };
    char aPwd[8] = { '\0' };

	printf("---------------添加卡---------------\n");
    /* 读取卡号：使用 fgets 捕获整行，检测长度，超长时提供重试或返回选项 */
    {
        char tmp[512] = {0};
        while (1)
        {
            printf("请输入卡号（长度1~18）：");
            if (fgets(tmp, sizeof(tmp), stdin) == NULL)
            {
                printf("输入无效。\n");
                return;
            }
            /* 去掉末尾换行 */
            size_t ln = strlen(tmp);
            if (ln > 0 && (tmp[ln-1] == '\n' || tmp[ln-1] == '\r')) tmp[--ln] = '\0';

            /* 去掉两端空白 */
            while (ln > 0 && (tmp[0] == ' ' || tmp[0] == '\t'))
            {
                memmove(tmp, tmp+1, --ln + 1);
            }
            while (ln > 0 && (tmp[ln-1] == ' ' || tmp[ln-1] == '\t')) tmp[--ln] = '\0';

            if (ln == 0)
            {
                printf("卡号不能为空，请重新输入。\n");
                continue;
            }
            if (ln > 17)
            {
                int choice = 0;
                while (1)
                {
                    printf("卡号长度(%zu)超过最大允许长度17！\n", ln);
                    printf("是否重新输入？1. 继续  2. 返回\n");
                    printf("请选择(1/2): ");
                    char optbuf[16] = {0};
                    if (fgets(optbuf, sizeof(optbuf), stdin) == NULL) return;
                    if (sscanf(optbuf, "%d", &choice) != 1) { printf("输入无效，请输入1或2。\n"); continue; }
                    if (choice == 1) break; /* 重新输入卡号 */
                    if (choice == 2) return; /* 放弃添加 */
                    printf("选择无效，请输入1或2。\n");
                }
                continue; /* 重新输入卡号 */
            }
            /* 合法长度，拷贝并退出循环 */
            strncpy(aName, tmp, sizeof(aName) - 1);
            aName[sizeof(aName) - 1] = '\0';
            break;
        }
    }
    /* 读取密码，使用 fgets 以避免 stdin 中残留数据影响后续读取 */
    {
        char tmpPwd[64] = {0};
        printf("请输入密码（长度1~8）：");
        if (fgets(tmpPwd, sizeof(tmpPwd), stdin) == NULL)
        {
            printf("输入无效。\n");
            return;
        }
        size_t ln = strlen(tmpPwd);
        if (ln > 0 && (tmpPwd[ln-1] == '\n' || tmpPwd[ln-1] == '\r')) tmpPwd[--ln] = '\0';
        /* 去掉首尾空白 */
        while (ln > 0 && (tmpPwd[0] == ' ' || tmpPwd[0] == '\t')) { memmove(tmpPwd, tmpPwd+1, --ln + 1); }
        while (ln > 0 && (tmpPwd[ln-1] == ' ' || tmpPwd[ln-1] == '\t')) tmpPwd[--ln] = '\0';
        if (ln == 0)
        {
            printf("密码不能为空。\n");
            return;
        }
        strncpy(aPwd, tmpPwd, sizeof(aPwd) - 1);
        aPwd[sizeof(aPwd) - 1] = '\0';
    }
	
	//判断卡号和密码是否合法
	int nNameSize = getSize(aName);
	int nPwdSize = getSize(aPwd);

    // card.aName 和 card.aPwd 在 model.h 中分别为 18 / 8 字节
    // 实际可存放的最大字符数分别为 17 / 7（保留 1 字节给 '\0'）
    if (nNameSize > 17 || nPwdSize > 7)
	{
		printf("卡号或者密码超过规定长度！\n");
		return;
	}
	else
	{
        // 使用 strncpy 并确保终止符，防止潜在越界
        strncpy(card.aName, aName, sizeof(card.aName) - 1);
        card.aName[sizeof(card.aName) - 1] = '\0';
        strncpy(card.aPwd, aPwd, sizeof(card.aPwd) - 1);
        card.aPwd[sizeof(card.aPwd) - 1] = '\0';
	}

	printf("请输入开卡金额（RMB）：");
	/* 读取开卡金额并验证输入有效性和非负 */
	if (scanf("%f", &card.fBalance) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("开卡金额输入无效。\n");
		return;
	}
	if (card.fBalance < 0.0f)
	{
		printf("开卡金额不能为负！\n");
		return;
	}
	
	card.fTotalUse = card.fBalance;  //初始化累计使用金额
	card.nUseCount = 0;           //初始化使用次数
	card.nStatus = 0;             //初始化卡状态
	card.nDel = 0;                //初始化删除标记

	/*初始化开卡时间、截止时间、最后使用时间*/
	card.tStart = time(NULL); 
	card.tEnd = time(NULL);
	card.tLastUse = time(NULL);

	struct tm* startTime;
	struct tm* endTime;

	startTime = localtime(&card.tStart);       //保存开卡时间
	endTime = localtime(&card.tEnd);          //转化截止时间类型，便于截止时间计算
	endTime->tm_year = startTime->tm_year + 1; //卡的有效期为一年
	card.tEnd = mktime(endTime);               //重新转化截止时间类型

	//统计卡数量
	if (!addCardInfo(card))
	{
		printf("添加卡信息失败！\n");
	}
	else
	{
		printf("\n");
		printf("----------添加的卡信息如下----------\n");
		printf("卡号\t密码\t卡状态\t余额\n");
		printf("%s\t%s\t%d\t%0.1f\n",aName,aPwd,card.nDel,card.fBalance);

	}
}
//获取字符串长度
int getSize(const char* pInfo)
{
	int nSize = 0;
	while (*(pInfo + nSize) != '\0')
	{
		nSize++;
	}
	return nSize;
}
//查询卡信息
void query()
{
    char aName[18] = { 0 };//卡号
    char aTime[TIMELENGTH] = { 0 };//时间字符串
    Card* pCard = NULL;//保存卡信息

    int nIndex = 0;
    int i;
    int opt = 0;

    printf("请选择查询方式：\n1-精确查询  2-模糊查询 ：");
    if (scanf("%d", &opt) != 1)
    {
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        opt = 2;
    }
    // 清理残留输入
    {
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    }

    if (opt == 1)
    {
        // 精确查询
        printf("请输入要精确查询的卡号：");
        if (scanf("%17s", aName) != 1)
        {
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf("输入无效。\n");
            return;
        }
        // 调用精确查询
        pCard = queryCard(aName);
        if (pCard == NULL)
        {
            printf("未找到卡号：'%s'。\n", aName);
            return;
        }
        // 输出单条记录
        timeToString(pCard->tLastUse, aTime);
        printf("查询到的卡信息如下：\n");
        printf("卡号\t状态\t余额\t累计使用\t使用次数\t上次使用时间\n");
        printf("%s\t%s\t%0.1f\t%0.1f\t\t%d\t\t%s\t\n",
            pCard->aName,
            statusToString(pCard->nStatus),
            pCard->fBalance,
            pCard->fTotalUse,
            pCard->nUseCount,
            aTime);
        return;
    }

    // 默认为模糊查询
    printf("请输入查询的卡号（支持部分匹配）：");
    fflush(stdout);
    if (scanf("%17s", aName) != 1)
    {
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("输入无效。\n");
        return;
    }

    //查询卡
    pCard = queryCardsInfo(aName, &nIndex);

    // 处理查询失败、未找到或其它异常的情况，给出明确反馈并保证内存安全
    if (pCard == NULL && nIndex == 0)
    {
        // 完全未获取到数据（可能是读取文件错误或query内部失败）
        printf("查询失败：无法读取卡信息（可能是数据文件损坏或读取失败）。\n");
        return;
    }
    if (nIndex == 0)
    {
        if (pCard != NULL)
        {
            free(pCard);
            pCard = NULL;
        }
        printf("未找到匹配的卡号：'%s'。\n", aName);
        printf("提示：\n - 请检查卡号是否输入正确（大小写/字符）\n - 支持部分匹配查询，例如输入一部分名称\n");
        return;
    }

    //显示
    printf("查询到的卡信息如下：\n");
    printf("卡号\t状态\t余额\t累计使用\t使用次数\t上次使用时间\n");
    for (i = 0; i < nIndex; i++)
    {
        if (pCard == NULL)
        {
            printf("内部错误\n");
            break;
        }
        timeToString(pCard[i].tLastUse, aTime);
        printf("%s\t%s\t%0.1f\t%0.1f\t\t%d\t\t%s\t\n",
            pCard[i].aName,
            statusToString(pCard[i].nStatus),
            pCard[i].fBalance,
            pCard[i].fTotalUse,
            pCard[i].nUseCount,
            aTime);
    }

    // 用完释放内存
    if (pCard != NULL)
    {
        free(pCard);
        pCard = NULL;
    }
}

//上机
void logon()
{
	char aName[20] = { 0 };//卡号
	char aPwd[8] = { 0 };//密码
	Card* pCard = NULL;
	LogonInfo info = { 0 };
	int rc;
	
	printf("---------------上机---------------\n");
	printf("请输入卡号：");
	// 接收用户输入的卡号和密码
	if (scanf("%19s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	//进行上机
	//pCard = doLogonInfo(aName, aPwd);
	// 调用新的 doLogonInfo（返回状态码，并通过 info 输出上机信息）
	rc = doLogonInfo(aName, aPwd, &info);
	
	//显示上机信息
	/*printf("上机信息如下：\n");
	if (pCard == NULL)
	{
		printf("上机失败！\n");
	}
	else
	{
		printf("卡号\t上机时间\t余额\n");
		//上机时间默认为最后使用时间，后面添加计费信息后，使用计费信息中的上机时间
		char logonTime[TIMELENGTH] = { 0 };
		timeToString(pCard->tLastUse, logonTime);
		printf("%s\t%s\t%0.1f\n", pCard->aName, logonTime, pCard->fBalance);
	}*/
	// 根据返回码处理
	if (rc == LOGONSUCCESS)
	{
		// 使用卡号查询并获得 Card*（保持原来通过 pCard 使用的方式）
		pCard = queryCard(aName);
		printf("上机信息如下：\n");
		if (pCard == NULL)
		{
			// 理论上不应发生，但做容错处理
			printf("上机成功，但无法读取卡详情。\n");
		}
		else
		{
			printf("卡号\t余额\t上机时间\n");
			char logonTime[TIMELENGTH] = { 0 };
			// 优先使用 billing/logonInfo 的时间（info.tLogon）
			timeToString(info.tLogon != 0 ? info.tLogon : pCard->tLastUse, logonTime);
			printf("%s\t%0.1f\t%s\n", pCard->aName,info.fBalance , logonTime);
		}
	}
	else if (rc == CARDNOTFOUND)
	{
		printf("上机失败：未找到该卡。\n");
	}
	else if (rc == BALANCEINSUFFICIENT)
	{
		printf("上机失败：余额不足。\n");
	}
	else
	{
		printf("上机失败：系统错误或文件写入失败。\n");
	}
}

//下机
void logoff()
{
	char aName[20] = { 0 };//卡号
	char aPwd[8] = { 0 };//密码
	logoffInfo info = { 0 };//保存下机信息
	int rc = 0;//调用新的 doLogoffInfo（返回状态码，并通过 info 输出下机信息）

	printf("---------------下机---------------\n");
	printf("请输入卡号：");
	//提示用户输入下机的卡号和密码
	if (scanf("%19s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}

	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}
	//根据输入的卡号和密码进行验证，判断能否进行下机操作

	//调用下机函数进行下机操作
	rc = doLogoffInfo(aName, aPwd, &info); // 目前不处理下机信息输出
	//显示下机信息（下机时间、消费金额、余额等）
	if (rc == LOGOFFSUCCESS)
	{
		char timeBuf[TIMELENGTH] = { 0 };
		timeToString(info.tLogoff, timeBuf);
		printf("下机成功！\n");
		printf("卡号：%s\n", info.aCardName);
		printf("下机时间：%s\n", timeBuf);
		printf("本次消费：%.1f\n", info.fAmount);
		printf("当前余额：%.1f\n", info.fBalance);
	}
	else if (rc == UNUSED)
	{
		printf("下机失败：该卡当前未上机。\n");
	}
	else if (rc == BALANCEINSUFFICIENT)
	{
		printf("下机失败：余额不足，无法完成结算。\n");
	}
	else if (rc == LOGOFFFAILURE)
	{
		printf("下机失败：系统错误或未找到有效计费信息。\n");
	}
	else
	{
		printf("下机失败：未知错误（%d）。\n", rc);
	}
}

//充值
void addMoney()
{
	// 充值界面
	char aName[18] = { 0 };//卡号最大17字符
	char aPwd[8] = { 0 };//密码最大7字符
	float fAmount = 0.0f;
	MoneyInfo moneyInfo = { 0 };

	//提示用户输入充值的卡号、密码和金额
	printf("---------------充值---------------\n");
	printf("请输入卡号：");
	if (scanf("%17s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入充值金额：");
	if (scanf("%f", &fAmount) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("金额输入无效。\n");
		return;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	if (fAmount <= 0.0f)
	{
		printf("充值金额必须大于0。\n");
		return;
	}

	moneyInfo.fAmount = fAmount;
	//充值
	//显示充值结果（余额等） 
	
	// 调用业务层处理充值（写日志并更新余额）
	if (doAddMoney(aName, aPwd, &moneyInfo) == TRUE)
	{
		printf("充值成功！\n");
		printf("卡号：%s\n充值金额：%.1f\n当前余额：%.1f\n", moneyInfo.aCardName, moneyInfo.fAmount, moneyInfo.fBalance);
	}
	else
	{
		printf("充值失败！请检查卡号/密码是否正确，卡是否被删除或正在使用，或系统错误。\n");
	}
}

//退费
void refundMoney()
{
	// 退费界面
	char aName[18] = { 0 };//卡号最大17字符
	char aPwd[8] = { 0 };//密码最大7字符
	float fAmount = 0.0f;
	MoneyInfo moneyInfo = { 0 };
	//提示用户输入退费的卡号、密码和金额
	printf("---------------退费---------------\n");
	printf("请输入卡号：");
	if (scanf("%17s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入退费金额：");
	if (scanf("%f", &fAmount) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("金额输入无效。\n");
		return;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	if (fAmount <= 0.0f)
	{
		printf("退费金额必须大于0。\n");
		return;
	}

	moneyInfo.fAmount = fAmount;

	//退费
	//显示退费结果（余额等）

	// 调用业务层处理退费（写日志并更新余额）
	if (doRefundMoney(aName, aPwd, &moneyInfo) == TRUE)
	{
		printf("退费成功！\n");
		printf("卡号：%s\n退费金额：%.1f\n当前余额：%.1f\n", moneyInfo.aCardName, moneyInfo.fAmount, moneyInfo.fBalance);
	}
	else
	{
		printf("退费失败！请检查卡号/密码是否正确、卡是否被删除或正在使用，或余额是否足够，或系统错误。\n");
	}
}

// 追加查询结果到 STATPATH，格式：header##timestamp##body（body 中换行会被替换为空格）
static void appendReport(const char* header, const char* body)
{
	FILE* fp = fopen(STATPATH, "a");
	if (fp != NULL)
	{
		time_t now = time(NULL);
		char timebuf[TIMELENGTH] = { 0 };
		timeToString(now, timebuf);

		// 复制 body 并替换换行/回车为单空格，防止多行破坏解析格式
		char clean[4096] = { 0 };
		if (body != NULL)
		{
			strncpy(clean, body, sizeof(clean) - 1);
			for (size_t i = 0; i < strlen(clean); i++)
			{
				if (clean[i] == '\n' || clean[i] == '\r') clean[i] = ' ';
			}
		}

		// 写入单行：header##time##content
		fprintf(fp, "%s##%s##%s\n", header ? header : "", timebuf, clean);
		fclose(fp);
	}
}

// Helper: 读取年月日三部分，返回 1 表示成功并写入 outStr ("YYYY-MM-DD")；
// forStart==1 表示开始日期（不能为空），forStart==0 表示结束日期（可全部留空->返回 2 表示使用当前时间）。
static int readYMDToDate(char* outStr, size_t outSize, int forStart)
{
	char yearBuf[16] = {0};
	char monBuf[8] = {0};
	char dayBuf[8] = {0};
	int year = 0, mon = 0, day = 0;

	while (1)
	{
        printf("请输入年份(YYYY)%s: ", forStart ? "(必须输入)" : "(可留空，留空表示当前时间)");
        if (!readLineTrim(yearBuf, sizeof(yearBuf))) { printf("输入失败，请重试。\n"); continue; }

        // 如果用户在年份输入中直接输入了完整的日期格式 YYYY-MM-DD，允许这样快捷输入
        if (strchr(yearBuf, '-') != NULL)
        {
            // 解析可能的完整日期（忽略时间部分）
            if (sscanf(yearBuf, "%d-%d-%d", &year, &mon, &day) == 3)
            {
                if (year < 1900 || mon < 1 || mon > 12 || day < 1 || day > 31)
                {
                    printf("日期数值不合法，请重新输入。\n");
                    continue;
                }
                snprintf(outStr, outSize, "%04d-%02d-%02d", year, mon, day);
                return 1;
            }
            // 如果不能解析为完整日期，则继续按分段输入处理
        }

        // 若为结束日期且用户直接回车（空串），返回 2 表示使用当前时间
        if (!forStart && strlen(yearBuf) == 0) return 2;

        printf("请输入月份(MM)%s: ", forStart ? "(必须输入)" : "(可留空)");
        if (!readLineTrim(monBuf, sizeof(monBuf))) { printf("输入失败，请重试。\n"); continue; }
        if (!forStart && strlen(yearBuf) == 0 && strlen(monBuf) == 0) return 2;

        printf("请输入日期(DD)%s: ", forStart ? "(必须输入)" : "(可留空)");
        if (!readLineTrim(dayBuf, sizeof(dayBuf))) { printf("输入失败，请重试。\n"); continue; }
        if (!forStart && strlen(yearBuf) == 0 && strlen(monBuf) == 0 && strlen(dayBuf) == 0) return 2;

        // 对于开始日期，三项都必须非空；对于结束日期，要求要么全部为空（已处理），要么三项都提供
        if (forStart)
        {
            if (strlen(yearBuf) == 0 || strlen(monBuf) == 0 || strlen(dayBuf) == 0)
            {
                printf("开始日期的年、月、日必须全部输入，请重新输入。\n");
                continue;
            }
        }
        else
        {
            // 如果部分填写但不完整，提示重试
            if ((strlen(yearBuf) == 0 || strlen(monBuf) == 0 || strlen(dayBuf) == 0))
            {
                printf("结束日期要么全部留空（表示当前时间），要么年/月/日都填写，请重新输入。\n");
                continue;
            }
        }

        // 解析数字并简单校验
        if (sscanf(yearBuf, "%d", &year) != 1 || sscanf(monBuf, "%d", &mon) != 1 || sscanf(dayBuf, "%d", &day) != 1)
        {
            printf("日期格式不正确，请输入数字。\n");
            continue;
        }
        if (year < 1900 || mon < 1 || mon > 12 || day < 1 || day > 31)
        {
            printf("日期数值不合法，请重新输入。\n");
            continue;
        }

        // 组织为 YYYY-MM-DD 格式
        snprintf(outStr, outSize, "%04d-%02d-%02d", year, mon, day);
        return 1;
	}
}

// 查询统计菜单
void queryStatistics()
{
	while (1)
	{
		printf("---------------查询统计---------------\n");
		printf("1. 消费记录查询(按卡号、时间段)\n");
		printf("2. 时间段总营业额\n");
		printf("3. 年度每月营业额\n");
		printf("0. 返回上级菜单\n");
		printf("请选择: ");

		char optStr[16] = { 0 };
		if (!readLineTrim(optStr, sizeof(optStr)))
		{
			printf("输入无效。\n");
			continue;
		}
		int opt = -1;
		if (sscanf(optStr, "%d", &opt) != 1)
		{
			printf("输入无效。\n");
			continue;
		}
		if (opt == 0) break;

		char startStr[TIMELENGTH] = { 0 };//时间字符串输入缓冲区 ("YYYY-MM-DD")
		char endStr[TIMELENGTH] = { 0 };//时间字符串输入缓冲区 ("YYYY-MM-DD")
		time_t tStart = 0, tEnd = 0;

		if (opt == 1)
		{
			char cardName[18] = { 0 };
			printf("请输入卡号(输入\"all\"表示查询所有): ");
			if (!readLineTrim(cardName, sizeof(cardName))) { printf("输入无效。\n"); continue; }

			// 开始日期（必须输入年/月/日）
			int rc = readYMDToDate(startStr, sizeof(startStr), 1);
			if (rc != 1) { printf("开始日期输入失败。\n"); continue; }
			// 结束日期（可全部留空表示当前时间）
			rc = readYMDToDate(endStr, sizeof(endStr), 0);
			if (rc == 2)
			{
				tEnd = time(NULL);
			}
			else if (rc == 1)
			{
				tEnd = stringToTime(endStr);
				// 扩展结束时间到当日 23:59:59
				if (tEnd != 0) tEnd += (time_t)(24*3600 - 1);
			}
			// 解析开始时间（已经确保填写）
			tStart = stringToTime(startStr);
			if (tStart == 0)
			{
				printf("开始日期解析失败。\n");
				continue;
			}

			const char* pNameFilter = NULL;
			if (strcmp(cardName, "all") != 0) pNameFilter = cardName;

            // 直接逐行扫描文件，避免依赖可能有问题的中间缓冲区
            FILE* fb = fopen(BILLINGPATH, "r");
            if (fb == NULL)
            {
                printf("无法打开账单文件：%s\n", BILLINGPATH);
                continue;
            }

            // 准备报表缓冲区（基于匹配数量上限估算）
            size_t bufSize = (size_t)128 * 8 + 1024; // 初始
            char* buf = (char*)malloc(bufSize);
            if (buf == NULL)
            {
                printf("内存不足，无法生成报表。\n");
                fclose(fb);
                continue;
            }
            buf[0] = '\0';
            int off = 0;
            off += snprintf(buf + off, bufSize - off, "消费记录 查询 %s 至 %s\n", startStr, endStr[0] ? endStr : "(now)");
            off += snprintf(buf + off, bufSize - off, "卡号\t消费时间\t\t金额\n");

            printf("卡号\t消费时间\t\t金额\n"); fflush(stdout);

            char line[2048];
            int found = 0;
            double total = 0.0;
            while (fgets(line, sizeof(line), fb) != NULL)
            {
                // trim newline
                size_t ln = strlen(line);
                while (ln > 0 && (line[ln-1] == '\n' || line[ln-1] == '\r')) { line[--ln] = '\0'; }
                if (ln == 0) continue;

                // parse fields separated by "##"
                char* fields[6] = {0};
                char* cur = line;
                for (int f = 0; f < 5; f++)
                {
                    char* pos = strstr(cur, "##");
                    if (!pos) { cur = NULL; break; }
                    *pos = '\0'; fields[f] = cur; cur = pos + 2;
                }
                if (cur == NULL) continue;
                fields[5] = cur;

                if (fields[0] == NULL) continue;
                // status and nDel
                int status = fields[4] ? atoi(fields[4]) : 0;
                int ndel = fields[5] ? atoi(fields[5]) : 0; // note: in file last field is nDel
                // In file format it's card##logon##logoff##amount##status##ndel
                // But our split placed last field in fields[5]

                if (status != 1 || ndel != 0) continue;

                // time fields
                time_t tlogon = fields[1] ? stringToTime(fields[1]) : 0;
                time_t tlogoff = fields[2] ? stringToTime(fields[2]) : 0;
                time_t t = tlogoff != 0 ? tlogoff : tlogon;
                if (tStart != 0 && t < tStart) continue;
                if (tEnd != 0 && t > tEnd) continue;

                // name match
                if (pNameFilter != NULL && pNameFilter[0] != '\0')
                {
                    if (strcmp(fields[0], pNameFilter) != 0) continue;
                }

                // amount
                double amt = fields[3] ? atof(fields[3]) : 0.0;

                // ensure buffer capacity
                size_t need = 128;
                if ((size_t)off + need > bufSize)
                {
                    size_t newSize = bufSize * 2 + need;
                    char* tmp = (char*)realloc(buf, newSize);
                    if (!tmp) break;
                    buf = tmp; bufSize = newSize;
                }

                char timestr[TIMELENGTH] = {0};
                timeToDateString(t, timestr);
                printf("%s\t%s\t%.1f\n", fields[0], timestr, (float)amt);
                off += snprintf(buf + off, bufSize - off, "%s\t%s\t%.1f\n", fields[0], timestr, (float)amt);
                total += amt; found++;
            }
            fclose(fb);

            if (found == 0)
            {
                printf("没有找到匹配的记录。\n");
                appendReport("消费记录查询", "没有匹配记录。");
                free(buf);
                continue;
            }

            off += snprintf(buf + off, bufSize - off, "合计金额: %.2f\n", total);
            printf("合计金额: %.2f\n", total);
            appendReport("消费记录查询", buf);

            // 详细文件
            {
                time_t now = time(NULL);
                char tmStr[TIMELENGTH] = {0};
                timeToDateString(now, tmStr);
                for (size_t i = 0; i < strlen(tmStr); i++) if (tmStr[i] == ' '||tmStr[i]==':'||tmStr[i]=='-') tmStr[i] = '_';
                char detailName[256] = {0};
                snprintf(detailName, sizeof(detailName), "stat_detail_consume_%s.txt", tmStr);
                FILE* fp = fopen(detailName, "w");
                if (fp)
                {
                    fprintf(fp, "查询时间: %s\n", tmStr);
                    fprintf(fp, "%s\n", buf);
                    fclose(fp);
                }
            }

            free(buf);
		}
		else if (opt == 2)
		{
			// 开始日期必须输入
            int rc = readYMDToDate(startStr, sizeof(startStr), 1);
			if (rc != 1) { printf("开始日期输入失败。\n"); continue; }
			// 结束日期可留空（空则视为当前时间）
			rc = readYMDToDate(endStr, sizeof(endStr), 0);
			if (rc == 2)
			{
				tEnd = time(NULL);
			}
			else if (rc == 1)
			{
				tEnd = stringToTime(endStr);
				if (tEnd != 0) tEnd += (time_t)(24*3600 - 1);
			}
			tStart = stringToTime(startStr);
			if (tStart == 0)
			{
				printf("开始日期解析失败。\n");
				continue;
			}

            // 直接从账单中按时间段汇总，避免底层统计函数可能遗漏未正确解析的记录
            int bCount = 0;
            Billing* bills = queryBillingByCardAndRange(NULL, tStart, tEnd, &bCount);
            double total = 0.0;
            if (bills != NULL && bCount > 0)
            {
                for (int i = 0; i < bCount; i++)
                {
                    total += (double)bills[i].fAmount;
                }
                free(bills);
            }
            // 同时统计现金流（充值/退费）——直接扫描 money 文件以保证解析一致性
            double inSum = 0.0, outSum = 0.0;
            int cashRc = FALSE;
            {
                FILE* fm = fopen(MONEYPATH, "r");
                if (fm != NULL)
                {
                    char line[512];
                    while (fgets(line, sizeof(line), fm) != NULL)
                    {
                        size_t ln = strlen(line);
                        while (ln > 0 && (line[ln-1] == '\n' || line[ln-1] == '\r')) { line[--ln] = '\0'; }
                        if (ln == 0) continue;

                        char tmp[512];
                        strncpy(tmp, line, sizeof(tmp) - 1);
                        tmp[sizeof(tmp) - 1] = '\0';

                        char* fields[5] = {0};
                        char* cur = tmp;
                        for (int f = 0; f < 4; f++)
                        {
                            char* pos = strstr(cur, "##");
                            if (!pos) { cur = NULL; break; }
                            *pos = '\0'; fields[f] = cur; cur = pos + 2;
                        }
                        if (cur == NULL) continue;
                        fields[4] = cur;

                        if (fields[0] == NULL) continue;
                        int ndel = fields[4] ? atoi(fields[4]) : 0;
                        if (ndel != 0) continue;
                        time_t t = fields[1] ? stringToTime(fields[1]) : 0;
                        if (tStart != 0 && t < tStart) continue;
                        if (tEnd != 0 && t > tEnd) continue;
                        int status = fields[2] ? atoi(fields[2]) : 0;
                        double amt = fields[3] ? atof(fields[3]) : 0.0;
                        if (status == 0) inSum += amt;
                        else if (status == 1) outSum += amt;
                        cashRc = TRUE;
                    }
                    fclose(fm);
                }
            }

			char out[512] = { 0 };
			if (cashRc == TRUE)
			{
				snprintf(out, sizeof(out),
					"时间段总营业额: %.2f\n时间段充值总额: %.2f\n时间段退费总额: %.2f\n净现金流: %.2f",
					total, inSum, outSum, inSum - outSum);
			}
			else
			{
				snprintf(out, sizeof(out), "时间段总营业额: %.2f\n现金流统计失败或无记录。", total);
			}

			printf("%s\n", out);
			appendReport("时间段总营业额与现金流", out);

			// 写详细文件
			{
                time_t now = time(NULL);
                char tmStr[TIMELENGTH] = { 0 };
                // 使用日期精度生成文件名时间戳
                timeToDateString(now, tmStr);
				for (size_t i = 0; i < strlen(tmStr); i++)
				{
					if (tmStr[i] == ' ' || tmStr[i] == ':' || tmStr[i] == '-') tmStr[i] = '_';
				}
				char detailName[256] = { 0 };
				snprintf(detailName, sizeof(detailName), "stat_detail_turnover_%s.txt", tmStr);
				FILE* fp = fopen(detailName, "w");
				if (fp)
				{
					fprintf(fp, "查询时间: %s\n", tmStr);
					fprintf(fp, "%s\n", out);
					fclose(fp);
				}
			}
		}
		else if (opt == 3)
		{
			int year = 0;
			printf("请输入年份(YYYY): ");
			char yearStr[16] = { 0 };
			if (!readLineTrim(yearStr, sizeof(yearStr)) || sscanf(yearStr, "%d", &year) != 1)
			{
				printf("输入无效。\n");
				continue;
			}
			double months[12] = { 0 };
			if (!getMonthlyTurnover(year, months))
			{
				printf("无法获取信息\n");
				continue;
			}
			char buf[2048] = { 0 };
			int off = 0;
			off += snprintf(buf + off, sizeof(buf) - off, "年份 %d 每月营业额\n", year);
			printf("月份\t营业额\n");
			double sum = 0.0;
			for (int i = 0; i < 12; i++)
			{
				printf("%2d\t%.2f\n", i + 1, months[i]);
				off += snprintf(buf + off, sizeof(buf) - off, "%02d\t%.2f\n", i + 1, months[i]);
				sum += months[i];
			}
			off += snprintf(buf + off, sizeof(buf) - off, "年度合计: %.2f\n", sum);
			printf("年度合计: %.2f\n", sum);
			appendReport("年度月度营业额", buf);

			// 写详细文件
			{
                time_t now = time(NULL);
                char tmStr[TIMELENGTH] = { 0 };
                // 使用日期精度生成文件名时间戳
                timeToDateString(now, tmStr);
				for (size_t i = 0; i < strlen(tmStr); i++)
				{
					if (tmStr[i] == ' ' || tmStr[i] == ':' || tmStr[i] == '-') tmStr[i] = '_';
				}
				char detailName[256] = { 0 };
				snprintf(detailName, sizeof(detailName), "stat_detail_monthly_%d_%s.txt", year, tmStr);
				FILE* fp = fopen(detailName, "w");
				if (fp)
				{
					fprintf(fp, "查询时间: %s\n", tmStr);
					fprintf(fp, "%s\n", buf);
					fclose(fp);
				}
			}
		}
		else
		{
			printf("输入无效。\n");
		}
	}
}

//注销卡
void annul()
{
	char aName[18] = { 0 };//卡号
	char aPwd[8] = { 0 };//密码
	Card cardParam = { 0 };

	printf("---------------注销卡---------------\n");
	printf("请输入卡号：");
	//提示用户输入退费的卡号、密码和金额
	if (scanf("%17s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}

	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	// 准备参数并调用业务层注销函数
	strncpy(cardParam.aName, aName, sizeof(cardParam.aName) - 1);
	strncpy(cardParam.aPwd, aPwd, sizeof(cardParam.aPwd) - 1);
	cardParam.fBalance = 0.0f;

	if (annulCard(&cardParam) == TRUE)
	{
		// 注销成功，显示卡号和退款金额（两列）
		printf("注销成功！\n");
		printf("卡号\t退款金额\n");
		printf("%s\t%.1f\n", cardParam.aName, cardParam.fBalance);
	}
	else
	{
		printf("注销卡失败！\n");
	}
}

// 修改：menu 层仅负责交互，调用 service.doModifyAccount
//修改账号或密码
void modifyAccount()
{
	char aName[18] = { 0 };// 当前卡号
	char aPwd[8] = { 0 };// 当前密码
	char newName[18] = { 0 };
	char newPwd[8] = { 0 };
	int modifyNameFlag = 0;//是否修改账号名
	int modifyPwdFlag = 0;//是否修改密码

	printf("---------------修改账号名或密码---------------\n");
	printf("请输入当前卡号：");
	if (scanf("%17s", aName) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	printf("请输入当前密码：");
	if (scanf("%7s", aPwd) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	printf("请选择要修改的项目：\n");
	printf("1. 修改账号名\n");
	printf("2. 修改密码\n");
	printf("3. 同时修改\n");
	printf("0. 取消\n");
	printf("请选择：");
	int opt = -1;
	if (scanf("%d", &opt) != 1)
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
		printf("输入无效。\n");
		return;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	if (opt == 0)
	{
		printf("已取消。\n");
		return;
	}
	if (opt == 1 || opt == 3)
	{
		printf("请输入新账号名（长度1~17）：");
		if (scanf("%17s", newName) != 1)
		{
			int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
			printf("输入无效。\n");
			return;
		}
		modifyNameFlag = 1;
	}
	if (opt == 2 || opt == 3)
	{
		printf("请输入新密码（长度1~7）：");
		if (scanf("%7s", newPwd) != 1)
		{
			int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
			printf("输入无效。\n");
			return;
		}
		modifyPwdFlag = 1;
	}
	// 清理残留输入
	{
		int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
	}

	// 调用业务层
	if (doModifyAccount(aName, aPwd, newName, newPwd, modifyNameFlag, modifyPwdFlag) == TRUE)
	{
		printf("修改成功！\n");
	}
	else
	{
		printf("修改失败！请检查卡号/密码或新账号名是否已存在，或系统错误。\n");
	}
}

//退出
void exitApp()
{
	releaseList(); 
}
