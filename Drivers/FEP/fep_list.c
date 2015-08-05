#include "fep_list.h"

static void 		fep_list_init		(void);
static void 		fep_list_add			(FEP_DEV_t tmpDev); 			// return root pointer
static void		 	fep_list_remove 		(char dev_id, char dev_type); 	// return root pointer
static FEP_DEV_t * 	fep_list_find 		(char dev_id, char dev_type); 	// return root pointer
static char 		fep_list_GetFreeID 	(char dev_type);
static int 			fep_list_count 		(void);

const FEP_LIST_API_t FEP_LIST_API = 
{
	.Init = fep_list_init,
	.Add = fep_list_add,
	.Remove = fep_list_remove,
	.Find = fep_list_find,
	.GetFreeID = fep_list_GetFreeID,
	.Count = fep_list_count
};

// root of list
FEP_DEV_t * 	fep_list_root = NULL;
int 			fep_list_counter = 0;

void 		fep_list_init		(void)
{
	// dummy, not necessary
}
void 		fep_list_add			(FEP_DEV_t tmpDev) 
{
	// add to head
	FEP_DEV_t * tmp = pvPortMalloc(sizeof (FEP_DEV_t));
	tmp->dev_id			= tmpDev.dev_id;
	tmp->dev_type 		= tmpDev.dev_type;
	tmp->status 		= tmpDev.status;
	tmp->need_register 	= tmpDev.need_register;
	tmp->data 			= tmpDev.data;
	tmp->next = fep_list_root;
	fep_list_root = tmp;
	fep_list_counter++;
}
void 		fep_list_remove 		(char dev_id, char dev_type) 
{	
	FEP_DEV_t * tmp, * pre_tmp;
	tmp = fep_list_root;
	pre_tmp = tmp;
	while (tmp != NULL)
	{
		if (tmp->dev_id == dev_id && tmp->dev_type == dev_type)
		{
			if (tmp == fep_list_root)
			{
				fep_list_root = fep_list_root->next;
				vPortFree(tmp);
			}
			else
			{
				pre_tmp->next = tmp->next;
				vPortFree(tmp);
			}
			fep_list_counter--;
		}
		pre_tmp = tmp;
		tmp = tmp->next;
	}
}
FEP_DEV_t * fep_list_find 		(char dev_id, char dev_type) // return pointer of dev
{
	FEP_DEV_t * tmp = fep_list_root;
	while (tmp != NULL)
	{
		if (tmp->dev_id == dev_id && tmp->dev_type == dev_type)
			return tmp;
		tmp = tmp->next;
	}
	return tmp;
}
char 		fep_list_GetFreeID 	(char dev_type)
{
	char res = 0;
	for (res = 0; res < 8; res++)
	{
		if (fep_list_find(res, dev_type) == NULL)
			return res;
	}
	return res;
}
int 			fep_list_count 		(void)
{
	return fep_list_counter;
}
