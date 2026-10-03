#ifndef __COM_PID_H__
#define __COM_PID_H__

#include "Com_Debug.h"
#include "Com_Types.h"

void Common_PID_Cumpute(PID_Struct *pid);
void Common_PID_Casecade(PID_Struct *outer, PID_Struct *inner);

#endif /* __COM_PID_H__ */
