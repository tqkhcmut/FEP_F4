#ifndef _fep_list_h_
#define _fep_list_h_

#include "fep_define.h"

typedef struct FEP_LIST_API
{
	void 		(*Init)		(void);
	void 		(*Add)		(FEP_DEV_t tmpDev); 			// return root pointer
	void 		(*Remove)	(char dev_id, char dev_type); 	// return root pointer
	FEP_DEV_t * (*Find)		(char dev_id, char dev_type); 	// return root pointer
	char 		(*GetFreeID)(char dev_type);
	int 		(*Count)	(void);
} FEP_LIST_API_t;

extern const FEP_LIST_API_t FEP_LIST_API;

#endif
