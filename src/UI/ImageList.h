#pragma once

#include <vector>
#include "../UI/Base.h"

namespace Falltergeist
{
    namespace UI
    {
        class Image;

        class ImageList : public Falltergeist::UI::Base
        {
            public:
                ImageList(Point pos, std::vector<std::shared_ptr<Image>> imageList);
                virtual ~ImageList() = default;

                void addImage(std::unique_ptr<Image> &image);
                void setCurrentImage(unsigned int number);
                unsigned int currentImage() const;
                const std::vector<std::shared_ptr<Image>>& images() const;

                virtual bool opaque(const Point &pos) override;

                virtual void render(bool eggTransparency) override;
                virtual void setPosition(const Point &pos) override;

            private:
                std::vector<std::shared_ptr<Image>> _images;
                unsigned int _currentImage = 0;
        };
    }
}
