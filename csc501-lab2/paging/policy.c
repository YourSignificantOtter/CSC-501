/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
Bool pageReplaceDebug = FALSE;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
	if(policy != SC || policy != AGING)
	{
		kprintf("srpolicy(%d) invalid policy input, defaulting to SC!\n");
		page_replace_policy = SC;
	}
	else
	{
		page_replace_policy = policy;
		pageReplaceDebug = TRUE;
	}

	return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
	return page_replace_policy;
}
