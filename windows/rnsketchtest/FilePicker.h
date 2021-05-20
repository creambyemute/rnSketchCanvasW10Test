#pragma once

#include "pch.h"
#include "NativeModules.h"
#include <string>

namespace RN = winrt::Microsoft::ReactNative;

namespace rnsketchtest::FilePicker {
    REACT_MODULE(FilePicker, L"FilePicker");
    struct FilePicker final {
        RN::ReactContext m_reactContext;

        REACT_INIT(Initialize)
            void Initialize(RN::ReactContext const& reactContext) noexcept;

        REACT_METHOD(pickFile)
        winrt::fire_and_forget pickFile(RN::ReactPromise<std::string> promise) noexcept;
    };
}