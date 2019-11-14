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
	pageReplaceDebug = TRUE;

	if(policy == SC || policy == AGING)
	{
		page_replace_policy = policy;
	}
	else
	{
		kprintf("srpolicy(%d) invalid policy input, defaulting to SC!\n");
		page_replace_policy = SC;
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
