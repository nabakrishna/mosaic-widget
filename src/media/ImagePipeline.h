#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <functional>

namespace mosaic::media {

constexpr UINT WM_MOSAIC_PHOTO_READY = WM_APP + 1;
struct DecodedImage {
    std::vector<uint8_t> pixels; // premultiplied BGRA, tightly packed (stride == width*4)
    UINT width = 0;
    UINT height = 0;
    bool success = false;
};
class ImagePipeline {
public:
    using ResultCallback = std::function<void(DecodedImage)>;

    ImagePipeline();
    ~ImagePipeline();

    ImagePipeline(const ImagePipeline&) = delete;
    ImagePipeline& operator=(const ImagePipeline&) = delete;

    // Must be called once, after `hwnd` exists, before the first
    // RequestDecode. Starts the single worker thread.
    void Start(HWND hwnd);
    void Stop();
    void RequestDecode(std::wstring filePath, UINT targetWidth, UINT targetHeight, ResultCallback onComplete);
    void PumpResult();

private:
    void WorkerThreadMain();

    struct PendingResult {
        DecodedImage image;
        ResultCallback callback;
    };

    HWND m_hwnd = nullptr;
    std::thread m_thread;
    std::atomic<bool> m_stopping{ false };

    // --- guarded by m_mutex ---
    std::mutex m_mutex;
    std::condition_variable m_cv;
    uint64_t m_generation = 0;          // bumped on every RequestDecode
    uint64_t m_requestedGeneration = 0; // which generation the worker should be working toward
    std::wstring m_requestedPath;
    UINT m_requestedWidth = 0;
    UINT m_requestedHeight = 0;
    ResultCallback m_requestedCallback;
    std::optional<PendingResult> m_pendingResult; // set by worker, consumed by PumpResult on UI thread
};

} // namespace mosaic::media
