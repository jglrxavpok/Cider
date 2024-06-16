//
// Created by jglrxavpok on 23/09/2023.
//

/**
 * C++ helpers for context.h
 */

#pragma once

#include <cider/context.h>
#include <functional>

void swapContextOnTop(Context* pCurrent, Context* pToSwitchTo, std::function<void(Context*)> onTop);