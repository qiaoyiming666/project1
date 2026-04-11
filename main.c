#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>

#include"menu.h"
#include"card_service.h"
#include"global.h"

int main(void)
{
	printf("欢迎进入计费管理系统\n");
	printf("\n");

	// 初始化卡链表
	if (initCardList() == FALSE)
	{
		printf("初始化失败，程序退出。\n");
		return 1;
	}

	int nSelection = -1;//储存用户输入数字

	do 
	{ 
		//输出菜单
		outputMenu();
		
		//接收用户输入的菜单项编号
		/*scanf("%d", &nSelection);*/

		// 接收用户输入的菜单项编号，检查返回值以处理非数字输入
		if (scanf("%d", &nSelection) != 1)
		{
			// 读取失败（非数字），清空输入并提示
			int ch;
			while ((ch = getchar()) != '\n' && ch != EOF) {}
			printf("输入的菜单序列号错误！请输入 0~8 之间的数字。\n");
			// 设置为非法值，继续循环提示菜单
			nSelection = -1;
			continue;
		}

		//清除输入流缓存
		//fflush(stdin);  
		
		// 清除输入流中残留的换行或其它字符
		int ch;
		while ((ch = getchar()) != '\n' && ch != EOF) {}

		//根据用户输入数字，输出选择的菜单信息
		switch (nSelection)
		{
		case 1:
		{
			add();
			break;
		}
		case 2:
		{
			query();
			break;
		}
		case 3:
		{
			logon();
			break;
		}
		case 4:
		{
			logoff();
			break;
		}
		case 5:
		{
			addMoney();
			break;
		}
		case 6:
		{
			refundMoney();
			break;
		}
		case 7:
		{
			printf("查询系统\n");
			break;
		}
		case 8:
		{
			annul();
			break;
		}
		case 9:
		{
			modifyAccount();
			break;
		}
		case 0:
		{
			exitApp();
			printf("退出\n");
			break;
		}
		default:
		{
			printf("输入的菜单序列号错误！\n");
			break;
		}
		}
	} while (nSelection != 0);
	return 0;
}
