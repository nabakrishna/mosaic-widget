#pragma once
#include <vector>
#include <string>
#include "media/IPhotoProvider.h"

namespace mosaic::media {

class LocalFolderPhotoProvider : public IPhotoProvider {
public:
    void SetFolder(const std::wstring& folderPath);

    bool HasPhotos() const override { return !m_files.empty(); }
    std::optional<std::wstring> NextPhotoPath() override;

private:
    void Reshuffle();

    std::vector<std::wstring> m_files;
    size_t m_cursor = 0;
};

} // namespace mosaic::media
