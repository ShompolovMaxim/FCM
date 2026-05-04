#pragma once

#include "../fcm.h"

#include <memory>

std::shared_ptr<FCM> cloneFCMForRuntime(const std::shared_ptr<FCM>& fcm);
