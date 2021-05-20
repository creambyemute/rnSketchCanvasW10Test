#include "pch.h"

#include "FilePicker.h"

#include <filesystem>
#include <windows.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Foundation;
using namespace Windows::Storage::Pickers;
using namespace Windows::System;
using namespace Windows::UI::Xaml;

namespace rnsketchtest::FilePicker {
    void FilePicker::Initialize(RN::ReactContext const& reactContext) noexcept {
        m_reactContext = reactContext;
    }

    winrt::fire_and_forget FilePicker::pickFile(RN::ReactPromise<std::string> promise) noexcept
    try {
        FileOpenPicker filePicker;
        filePicker.ViewMode(PickerViewMode::Thumbnail);
        filePicker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
        filePicker.FileTypeFilter().ReplaceAll({ L".jpg", L".jpeg", L".png" });

        try {
            StorageFile file{ co_await filePicker.PickSingleFileAsync() };
            try {
                if (file) {
                    StorageFile tempFile{ co_await file.CopyAsync(winrt::Windows::Storage::ApplicationData::Current().TemporaryFolder(), file.Name(), NameCollisionOption::ReplaceExisting) };
                    if (tempFile) {
                        promise.Resolve(winrt::to_string(tempFile.Path()));
                    } else {
                        promise.Reject("Couldn't copy file to TemporaryFolder");
                    }
                } else {
                    promise.Reject("No file was picked");
                }
            } catch (const hresult_error& ex) {
                promise.Reject(RN::ReactError{ "Couldn't copy file to TemporaryFolder", winrt::to_string(ex.message()).c_str()});
            }

        } catch(const hresult_error& ex) {
            promise.Reject(RN::ReactError{ "Unable to FilePicker.PickSingleFileAsync", winrt::to_string(ex.message()).c_str() });
        }

    } catch(const hresult_error& ex) {
        promise.Reject(RN::ReactError{ "Unable to set FilePicker options", winrt::to_string(ex.message()).c_str() });
    }
}