//
// Copyright (c) 2000  Microsoft Corporation
//

#include "../../pinball.h"

#ifndef PINBALL_TBLOCKER_H
#define PINBALL_TBLOCKER_H

/* 130 */
struct TBlocker;

void TBlocker::TimerExpired(int, struct TPinballComponent*); // idb
int __thiscall TBlocker::Message(TBlocker* this, int, float); // idb
TBlocker* __thiscall TBlocker::TBlocker(TBlocker* this, struct TPinballTable* a2, int a3);

void* TBlocker::vftable = &TBlocker::Message; // weak

#endif //PINBALL_TBLOCKER_H
