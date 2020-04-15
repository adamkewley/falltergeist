#pragma once

#include <memory>

#include "Graphics/Point.h"
#include "UI/Image.h"



namespace Falltergeist
{
    namespace Graphics
    {
        class Sprite;
    }

    namespace UI
    {
        class Image;

        class IResourceManager
        {
            public:
                virtual ~IResourceManager() = default;

                // TODO replace with smart pointer
                virtual Image* getImage(const std::string &filename) = 0;
                Image* getImage(const std::string &filename, Graphics::Point p) {
                    auto img = this->getImage(filename);
                    img->setPosition(p);
                    return img;
                }
                virtual std::shared_ptr<Graphics::Sprite> getSprite(const std::string &filename) = 0;
        };
    }
}
