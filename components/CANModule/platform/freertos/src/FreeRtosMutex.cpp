#include "FreeRtosMutex.hpp"

FreeRtosMutex::FreeRtosMutex() {
    handle_ = xSemaphoreCreateMutex();
    configASSERT(handle_ != nullptr);
}

FreeRtosMutex::~FreeRtosMutex() {
    if (handle_ != nullptr) {
        vSemaphoreDelete(handle_);
        handle_ = nullptr;
    }
}

void FreeRtosMutex::lock() {
    xSemaphoreTake(handle_, portMAX_DELAY);
}

void FreeRtosMutex::unlock() {
    xSemaphoreGive(handle_);
}