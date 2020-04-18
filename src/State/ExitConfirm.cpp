#include "../State/ExitConfirm.h"
#include "../functions.h"
#include "../Game/Game.h"
#include "../Graphics/Renderer.h"
#include "../Input/Mouse.h"
#include "../ResourceManager.h"
#include "../State/Location.h"
#include "../State/MainMenu.h"
#include "../UI/Factory/ImageButtonFactory.h"
#include "../UI/Image.h"
#include "../UI/ImageButton.h"
#include "../UI/PlayerPanel.h"
#include "../UI/TextArea.h"

namespace Falltergeist
{
    using ImageButtonType = UI::Factory::ImageButtonFactory::Type;

    namespace State
    {
        ExitConfirm::ExitConfirm(std::shared_ptr<UI::IResourceManager> resourceManager) : State()
        {
            this->resourceManager = resourceManager;
            imageButtonFactory = std::make_unique<UI::Factory::ImageButtonFactory>(resourceManager);
        }

        void ExitConfirm::init()
        {
            if (_initialized) {
                return;
            }

            State::init();

            setModal(true);
            setFullscreen(false);

            auto background = resourceManager->getImage("art/intrface/lgdialog.frm");
            auto panelHeight = Game::getInstance()->locationState()->playerPanel()->size().height();
            auto backgroundPos = (Game::getInstance()->renderer()->size() - background->size() - Point(0, panelHeight)) / 2;
            background->setPosition(backgroundPos);
            addUI(std::move(background));

            auto& box1 = *addUI(resourceManager->getImage("art/intrface/donebox.frm"));
            box1.setPosition(backgroundPos + Point(38, 98));

            auto& box2 = *addUI(resourceManager->getImage("art/intrface/donebox.frm"));
            box2.setPosition(backgroundPos + Point(170, 98));

            auto& yesButton = *addUI(imageButtonFactory->getByType(ImageButtonType::SMALL_RED_CIRCLE, backgroundPos + Point(50, 102)));
            yesButton.mouseClickHandler().add([this](Event::Event* event){
                this->doYes();
            });

            auto& noButton = *addUI(imageButtonFactory->getByType(ImageButtonType::SMALL_RED_CIRCLE, backgroundPos + Point(183, 102)));
            noButton.mouseClickHandler().add( [this](Event::Event* event){
                this->doNo();
            });

            // label: Are you sure you want to quit?
            auto& quitLabel = *makeUI<UI::TextArea>(_t(MSG_MISC, 0), backgroundPos + Point(30, 52));
            quitLabel.setFont("font1.aaf", {0xb8,0x9c, 0x28, 0xff});
            quitLabel.setSize({244, 0});
            quitLabel.setHorizontalAlign(UI::TextArea::HorizontalAlign::CENTER);

            // label: yes & no
            // label: yes 101
            auto& yesButtonLabel = *makeUI<UI::TextArea>(_t(MSG_DIALOG_BOX, 101), backgroundPos + Point(74, 101));
            yesButtonLabel.setFont("font3.aaf", {0xb8,0x9c,0x28,0xff});

            // label: no 102
            auto& noButtonLabel = *makeUI<UI::TextArea>(_t(MSG_DIALOG_BOX, 102), backgroundPos + Point(204, 101));
            noButtonLabel.setFont("font3.aaf", {0xb8,0x9c,0x28,0xff});
        }

        void ExitConfirm::doYes()
        {
            Game::getInstance()->setState(new MainMenu(resourceManager));
        }

        void ExitConfirm::doNo()
        {
            Game::getInstance()->popState();
        }

        void ExitConfirm::onKeyDown(Event::Keyboard* event)
        {
            switch (event->keyCode())
            {
                case SDLK_ESCAPE:
                case SDLK_n:
                    doNo();
                    break;
                case SDLK_RETURN:
                case SDLK_y:
                    doYes();
                    break;
            }
        }

        void ExitConfirm::onStateActivate(Event::State* event)
        {
            Game::getInstance()->mouse()->pushState(Input::Mouse::Cursor::BIG_ARROW);
        }

        void ExitConfirm::onStateDeactivate(Event::State* event)
        {
            Game::getInstance()->mouse()->popState();
        }
    }
}
