#include "pch.h"

#include "ImageResizer.h"

#include <filesystem>
#include <windows.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.Imaging.h>

using namespace winrt;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Imaging;
using namespace Windows::System;

namespace rnsketchtest::ImageResizer {
    void ImageResizer::Initialize(RN::ReactContext const& reactContext) noexcept {
        m_reactContext = reactContext;
    } 

    IAsyncOperation<StorageFile> ImageResizer::getFileCopy(std::filesystem::path imgPath) noexcept
    {
        StorageFile file{ co_await StorageFile::GetFileFromPathAsync(winrt::to_hstring(imgPath.c_str())) };
        std::string displayName = winrt::to_string(file.DisplayName());
        std::string fileType = winrt::to_string(file.FileType());
        std::string newFileName = displayName.append("ResizerCopy.").append(fileType);
        StorageFile fileCopy{ co_await file.CopyAsync(ApplicationData::Current().TemporaryFolder(), winrt::to_hstring(newFileName), NameCollisionOption::ReplaceExisting) };
        return fileCopy;
    }

    IAsyncOperation<IRandomAccessStream> ImageResizer::getStreamFromFile(StorageFile file) noexcept
    {
        IRandomAccessStream stream{ co_await file.OpenAsync(FileAccessMode::ReadWrite) };
        return stream;
    }

    IAsyncOperation<InMemoryRandomAccessStream> ImageResizer::rotateStreamBy(IRandomAccessStream stream, int desiredRotation) noexcept
    {
        BitmapRotation newRotation{ BitmapRotation::None };
        switch (desiredRotation)
        {
        case 90:
            newRotation = BitmapRotation::Clockwise90Degrees;
            break;
        case 180:
            newRotation = BitmapRotation::Clockwise180Degrees;
            break;
        case 270:
            newRotation = BitmapRotation::Clockwise270Degrees;
            break;
        default:
            break;
        }

        BitmapDecoder decoder{ co_await BitmapDecoder::CreateAsync(stream) };
        InMemoryRandomAccessStream inMemoryStream;
        BitmapEncoder encoder{ co_await BitmapEncoder::CreateForTranscodingAsync(inMemoryStream, decoder) };

        BitmapTransform transform = encoder.BitmapTransform();
        transform.Rotation(newRotation);
        transform.InterpolationMode(BitmapInterpolationMode::Fant);

        co_await encoder.FlushAsync();
        return inMemoryStream;
    }

    IAsyncOperation<InMemoryRandomAccessStream> ImageResizer::scaleStream(IRandomAccessStream stream, int newMaxSide, bool onlyScaleDown) noexcept
    {
        BitmapDecoder decoder{ co_await BitmapDecoder::CreateAsync(stream) };
        uint32_t originalWidth = decoder.PixelWidth();
        uint32_t originalHeight = decoder.PixelHeight();
        uint32_t originalOrientedWidth = decoder.OrientedPixelWidth();
        uint32_t originalOrientedHeight = decoder.OrientedPixelHeight();
        
        InMemoryRandomAccessStream inMemoryStream;
        BitmapEncoder encoder{ co_await BitmapEncoder::CreateForTranscodingAsync(inMemoryStream, decoder) };
        BitmapTransform transform = encoder.BitmapTransform();
        transform.InterpolationMode(BitmapInterpolationMode::Fant);

        if (onlyScaleDown && originalHeight < newMaxSide && originalWidth < newMaxSide) {
            co_await encoder.FlushAsync();
            return inMemoryStream;
        }
        else {
            double widthRatio = (double)newMaxSide / originalOrientedWidth;
            double heightRatio = (double)newMaxSide / originalOrientedHeight;
            uint32_t aspectWidth = (uint32_t)newMaxSide;
            uint32_t aspectHeight = (uint32_t)newMaxSide;

            if (originalOrientedWidth < originalOrientedHeight) {
                aspectWidth = (uint32_t)(heightRatio * originalOrientedWidth);
            }
            else {
                aspectHeight = (uint32_t)(widthRatio * originalOrientedHeight);
            }

            transform.ScaledWidth(aspectWidth);
            transform.ScaledHeight(aspectHeight);
            if (originalOrientedHeight != originalHeight && originalOrientedWidth != originalWidth) {
                transform.Rotation(BitmapRotation::Clockwise270Degrees);
            }

            co_await encoder.FlushAsync();
            return inMemoryStream;
        }
    }

    IAsyncOperation<IRandomAccessStream> ImageResizer::convertToJPEG(IRandomAccessStream inputStream, IRandomAccessStream outputStream, int newQuality) noexcept
    {
        BitmapDecoder decoder{ co_await BitmapDecoder::CreateAsync(inputStream) };
        PixelDataProvider pixelData{ co_await decoder.GetPixelDataAsync() };
        
        // According to docs it should be std::Array <byte>, that didn't work.
        // Using auto like in react-native-fs or react-native-sketch-canvas
        auto detachedPixelData = pixelData.DetachPixelData();
        pixelData = nullptr;

        double imgQuality = newQuality > 0.0 ? newQuality / 100.0 : 1.0;
        BitmapPropertySet propertySet;
        propertySet.Insert(winrt::to_hstring("ImageQuality"), BitmapTypedValue(box_value(imgQuality), PropertyType::Single));

        BitmapEncoder encoder{ co_await BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId(), outputStream, propertySet) };
        encoder.SetPixelData(decoder.BitmapPixelFormat(), decoder.BitmapAlphaMode(), decoder.OrientedPixelWidth(), decoder.OrientedPixelHeight(), decoder.DpiX(), decoder.DpiY(), detachedPixelData);

        co_await encoder.FlushAsync();
        co_await outputStream.FlushAsync();
        return outputStream;
    }

    // Implementation
    winrt::fire_and_forget ImageResizer::resizeImage(std::string imgPath, int newMaxSide, int newQuality, int newRotation, bool onlyScaleDown, RN::ReactPromise<std::string> promise) noexcept
    try {
        // Make a path
        std::filesystem::path fPath{ imgPath };
        fPath.make_preferred();
        try {
            StorageFile imgFileCopy{ co_await getFileCopy(fPath) };
            if (imgFileCopy) {
                try {
                    IRandomAccessStream imgStream{ co_await getStreamFromFile(imgFileCopy) };
                    if (imgStream) {
                        try
                        {
                            // According to docs, 1. scale then rotate (could be combined)
                            InMemoryRandomAccessStream scaledIMStream{ co_await scaleStream(imgStream, newMaxSide, onlyScaleDown) };
                            if (scaledIMStream) {
                                scaledIMStream.Seek(0);
                                imgStream.Seek(0);
                                imgStream.Size(0);
                                co_await RandomAccessStream::CopyAsync(scaledIMStream, imgStream);
                            }
                            else {
                                promise.Reject(RN::ReactError{ "scaleStream for imgStream returned nullptr", winrt::to_string(fPath.c_str()) });
                            }
                            scaledIMStream = nullptr;

                            // 2. Rotate
                            InMemoryRandomAccessStream rotatedIMStream{ co_await rotateStreamBy(imgStream, newRotation) };
                            if (rotatedIMStream) {
                                rotatedIMStream.Seek(0);
                                imgStream.Seek(0);
                                imgStream.Size(0);
                                co_await RandomAccessStream::CopyAsync(rotatedIMStream, imgStream);
                            }
                            else {
                                promise.Reject(RN::ReactError{ "rotateStreamBy for imgStream returned nullptr", winrt::to_string(fPath.c_str()) });
                            }
                            rotatedIMStream = nullptr;


                            // Finally convert to JPG
                            StorageFolder outputFolder{ ApplicationData::Current().TemporaryFolder() };
                            StorageFile outputFile{ co_await outputFolder.CreateFileAsync(winrt::to_hstring("tempResizedImage.jpg"), CreationCollisionOption::ReplaceExisting) };
                            IRandomAccessStream outputImgStream{ co_await getStreamFromFile(outputFile) };
                            outputImgStream = co_await convertToJPEG(imgStream, outputImgStream, newQuality);
                            outputImgStream = nullptr;
                            imgStream = nullptr;
                            co_await imgFileCopy.DeleteAsync();
                            
                            promise.Resolve(winrt::to_string(outputFile.Path()));
                        }
                        catch (const hresult_error& ex)
                        {
                            promise.Reject(RN::ReactError{ "rotateStreamBy for imgStream failed", winrt::to_string(fPath.c_str()) });
                        }
                    } else {
                        promise.Reject(RN::ReactError{ "getStreamFromFile for imgFileCopy returned nullptr", winrt::to_string(fPath.c_str()) });
                    }
                } catch(const hresult_error& ex) {
                    promise.Reject(RN::ReactError{ "Unable to get getStreamFromFile for imgFileCopy", winrt::to_string(fPath.c_str()) });
                }
            } else {
                promise.Reject(RN::ReactError{ "getFileCopy for path returned nullptr", winrt::to_string(fPath.c_str()) });
            }
        } catch(const hresult_error& ex) {
            promise.Reject(RN::ReactError{ "Unable to getFileCopy for path " + imgPath, winrt::to_string(ex.message()).c_str() });
        }
    } catch(const hresult_error& ex) {
        promise.Reject(RN::ReactError{ "Unable to make path or make_preferred for path " + imgPath, winrt::to_string(ex.message()).c_str() });
    }
}