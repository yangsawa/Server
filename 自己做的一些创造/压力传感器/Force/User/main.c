#include "stm32f10x.h"                  // Device header
#include "system.h"
int main(void)
{
	SysInit();
	SysFunc();
  SysLoop();
}
