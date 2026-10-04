#pragma once
#include <string>
#include <optional>

namespace mosaic::media {

class IPhotoProvider {
public:
    virtual ~IPhotoProvider() = default;
    virtual bool HasPhotos() const = 0;
    virtual std::optional<std::wstring> NextPhotoPath() = 0;
};

} // namespace mosaic::media
