#pragma once

#include "../UI/IResourceManager.h"

namespace Falltergeist
{
    namespace UI
    {
        class ResourceManager final : public IResourceManager
        {
            public:
                ResourceManager() = default;
                virtual ~ResourceManager() = default;

                Image* getImage(const std::string &filename) override;
                Image* getImage(const std::string &filename, Graphics::Point p) override;
                std::shared_ptr<Graphics::Sprite> getSprite(const std::string &filename) override;
        };
    }
}
