#include "../Graphics/Sprite.h"
#include "../UI/Image.h"
#include "../UI/ResourceManager.h"

namespace Falltergeist
{
    namespace UI
    {
        Image* ResourceManager::getImage(const std::string &filename)
        {
            return new Image(std::make_unique<Graphics::Sprite>(filename));
        }

        Image* ResourceManager::getImage(const std::string &filename, Graphics::Point p)
        {
            auto img = this->getImage(filename);
            img->setPosition(p);
            return img;
        }

        std::shared_ptr<Graphics::Sprite> ResourceManager::getSprite(const std::string &filename)
        {
            return std::make_shared<Graphics::Sprite>(filename);
        }
    }
}
