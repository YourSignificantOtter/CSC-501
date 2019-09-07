#include <stdio.h>

extern int etext;
extern int edata;
extern int end;

void printsegaddress()
{
	kprintf("void printsegaddress()\n\n");

	int *etextPtr = &etext;
	int *edataPtr = &edata;
	int *endPtr = &end;

	kprintf("Current:   etext[0x%08X]=0x%08X, edata[0x%08X]=0x%08X, ebss[0x%08X]=0x%08X\n", &etext, etext, &edata, edata, &end, end);
	kprintf("Preceding: etext[0x%08X]=0x%08X, edata[0x%08X]=0x%08X, ebss[0x%08X]=0x%08X\n", etextPtr - 1, *(etextPtr - 1), edataPtr - 1, *(edataPtr - 1), endPtr - 1, *(endPtr - 1));	
	kprintf("After:     etext[0x%08X]=0x%08X, edata[0x%08X]=0x%08X, ebss[0x%08X]=0x%08X\n", etextPtr + 1, *(etextPtr + 1), edataPtr + 1, *(edataPtr + 1), endPtr + 1, *(endPtr + 1));	
	
	kprintf("\n");
}
