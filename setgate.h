#ifndef _SETGATE_H
#define _SETGATE_H

#include "x86_desc.h"

void set_intr_gate(int n, void* addr);
void set_system_gate(int n, void* addr);
void set_system_intr_gate(int n, void* addr);
void set_trap_gate(int n, void* addr);
void set_task_gate(int n, void* addr);

#endif

