#pragma once

#include "pch.h"
#include "NativeModules.h"
#include <string>
#include <filesystem>
#include <windows.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.Imaging.h>

namespace RN = winrt::Microsoft::ReactNative;

namespace rnsketchtest::ImageResizer {
    REACT_MODULE(ImageResizer, L"ImageResizer");
    struct ImageResizer final {
        RN::ReactContext m_reactContext;

        REACT_INIT(Initialize)
            void Initialize(RN::ReactContext const& reactContext) noexcept;

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> getFileCopy(std::filesystem::path imgPath) noexcept;
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IRandomAccessStream> getStreamFromFile(winrt::Windows::Storage::StorageFile file) noexcept;
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::InMemoryRandomAccessStream> rotateStreamBy(winrt::Windows::Storage::Streams::IRandomAccessStream stream, int desiredRotation) noexcept;
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::InMemoryRandomAccessStream> scaleStream(winrt::Windows::Storage::Streams::IRandomAccessStream stream, int newMaxSide, bool onlyScaleDown) noexcept;
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IRandomAccessStream> convertToJPEG(winrt::Windows::Storage::Streams::IRandomAccessStream inputStream, winrt::Windows::Storage::Streams::IRandomAccessStream outputStream, int newQuality) noexcept;
        
        REACT_METHOD(resizeImage)
            winrt::fire_and_forget resizeImage(std::string imgPath, int newMaxSide, int quality, int rotation, bool onlyScaleDown, RN::ReactPromise<std::string> promise) noexcept;
        
    };
}