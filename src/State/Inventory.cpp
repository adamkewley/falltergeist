#include <sstream>
#include "../State/Inventory.h"
#include "../Event/Event.h"
#include "../Event/Mouse.h"
#include "../functions.h"
#include "../Game/ArmorItemObject.h"
#include "../Game/CritterObject.h"
#include "../Game/DudeObject.h"
#include "../Game/Game.h"
#include "../Game/Object.h"
#include "../Game/ObjectFactory.h"
#include "../Game/WeaponItemObject.h"
#include "../Graphics/CritterAnimationFactory.h"
#include "../Graphics/Size.h"
#include "../Helpers/CritterHelper.h"
#include "../Input/Mouse.h"
#include "../ResourceManager.h"
#include "../State/State.h"
#include "../State/GameMenu.h"
#include "../State/InventoryDragItem.h"
#include "../State/Location.h"
#include "../UI/Animation.h"
#include "../UI/Factory/ImageButtonFactory.h"
#include "../UI/Image.h"
#include "../UI/ImageButton.h"
#include "../UI/ImageList.h"
#include "../UI/InventoryItem.h"
#include "../UI/ItemsList.h"
#include "../UI/PlayerPanel.h"
#include "../UI/Rectangle.h"
#include "../UI/TextArea.h"

namespace Falltergeist
{
    using ImageButtonType = UI::Factory::ImageButtonFactory::Type;

    namespace State
    {
        Inventory::Inventory(std::shared_ptr<UI::IResourceManager> _resourceManager):
            State{},
            resourceManager{std::move(_resourceManager)},
            imageButtonFactory{std::make_unique<UI::Factory::ImageButtonFactory>(resourceManager)}
        {
            pushHandler().add([](Event::State* ev) {
                Game::getInstance()->mouse()->pushState(Input::Mouse::Cursor::ACTION);
            });

            popHandler().add([](Event::State* ev) {
                // If hand cursor now
                if (Game::getInstance()->mouse()->state() == Input::Mouse::Cursor::HAND) {
                    Game::getInstance()->mouse()->popState();
                }
                Game::getInstance()->mouse()->popState();
            });
        }

        void Inventory::init()
        {
            constexpr SDL_Color brightRed{0xff, 0x00, 0x00, 0xff};
            constexpr SDL_Color neonGreen{0x3f, 0xf8, 0x00, 0xff};

            if (_initialized) {
                return;
            }

            State::init();

            setModal(true);
            setFullscreen(false);

            auto game = Game::getInstance();

            {
                auto panelHeight = Game::getInstance()->locationState()->playerPanel()->size().height();
                auto inventoryPanelPos = (game->renderer()->size() - Point(499, 377 + panelHeight)) / 2;
                setPosition(inventoryPanelPos);
            }

            addUI("background", resourceManager->getImage("art/intrface/invbox.frm"));
            getUI("background")->mouseClickHandler().add([this](Event::Mouse* event) {
                this->backgroundRightClick(event);
            });
            {
                auto scrollItemListUpBtn = addUI("button_up", imageButtonFactory->getByType(ImageButtonType::INVENTORY_UP_ARROW, {128, 40}));
                scrollItemListUpBtn->mouseClickHandler().add([this](Event::Mouse* event) {
                    this->onScrollUpButtonClick(event);
                });
            }
            addUI("button_up_disabled", resourceManager->getImage("art/intrface/invupds.frm", {128, 40}));
            {
                auto scrollItemListDownBtn = addUI("button_down", imageButtonFactory->getByType(ImageButtonType::INVENTORY_DOWN_ARROW, {128, 65}));
                scrollItemListDownBtn->mouseClickHandler().add([this](Event::Mouse* event) {
                    this->onScrollDownButtonClick(event);
                });
            }
            addUI("button_down_disabled", resourceManager->getImage("art/intrface/invdnds.frm", {128, 65}));
            {
                auto doneBtn = addUI("button_done", imageButtonFactory->getByType(ImageButtonType::SMALL_RED_CIRCLE, {438, 328}));
                doneBtn->mouseClickHandler().add([this](Event::Mouse* event) {
                    this->onDoneButtonClick(event);
                });
            }

            const Point screenPos{300, 47};
            auto player = Game::getInstance()->player();

            addUI("player_name", new UI::TextArea(player->name(), screenPos));

            {
                auto line1 = new UI::Rectangle(screenPos + Point{0, 16}, Graphics::Size(142, 1), neonGreen);
                addUI(line1);
            }

            {
                std::string statsLabels;
                for (auto i = (unsigned)STAT::STRENGTH; i <= (unsigned)STAT::LUCK; i++)
                {
                    statsLabels += _t(MSG_INVENTORY, i);
                    statsLabels += "\n";
                }
                addUI("label_stats", new UI::TextArea(std::move(statsLabels), screenPos + Point{0, 20}));
            }

            {
                std::string statsValues;
                for (auto i = (unsigned)STAT::STRENGTH; i <= (unsigned)STAT::LUCK; i++)
                {
                    statsValues += std::to_string(player->stat((STAT)i)) + "\n";
                }
                addUI("label_stats_values", new UI::TextArea(std::move(statsValues), screenPos + Point{22, 20}));
            }

            {
                std::string label;
                for (unsigned int i=7; i<14; i++)
                {
                    label += _t(MSG_INVENTORY, i) + "\n";
                }
                auto textLabel = new UI::TextArea(std::move(label), screenPos + Point{40, 20});
                addUI("textLabel", textLabel);
            }

            {
                std::string label;
                label += std::to_string(player->hitPoints());
                label += "/";
                label += std::to_string(player->hitPointsMax());
                auto hitPointsLabel = new UI::TextArea(std::move(label), screenPos + Point{94, 20});
                hitPointsLabel->setWidth(46);
                hitPointsLabel->setHorizontalAlign(UI::TextArea::HorizontalAlign::RIGHT);
                addUI("hitPointsLabel", hitPointsLabel);
            }

            {
                auto armorClassLabel = new UI::TextArea(std::to_string(player->armorClass()), screenPos + Point{94, 30});
                armorClassLabel->setWidth(46);
                armorClassLabel->setHorizontalAlign(UI::TextArea::HorizontalAlign::RIGHT);
                addUI("armorClassLabel", armorClassLabel);
            }

            Game::ArmorItemObject* playerArmor = player->armorSlot();
            {
                std::stringstream ss;
                if (playerArmor != nullptr)
                {
                    ss << player->damageThreshold(DAMAGE::NORMAL) + playerArmor->damageThreshold(DAMAGE::NORMAL) << "/\n";
                    ss << player->damageThreshold(DAMAGE::LASER) + playerArmor->damageThreshold(DAMAGE::LASER) << "/\n";
                    ss << player->damageThreshold(DAMAGE::FIRE) + playerArmor->damageThreshold(DAMAGE::FIRE) << "/\n";
                    ss << player->damageThreshold(DAMAGE::PLASMA) + playerArmor->damageThreshold(DAMAGE::PLASMA) << "/\n";
                    ss << player->damageThreshold(DAMAGE::EXPLOSIVE) + playerArmor->damageThreshold(DAMAGE::NORMAL) << "/";
                }
                else
                {
                    ss << player->damageThreshold(DAMAGE::NORMAL) <<"/\n";
                    ss << player->damageThreshold(DAMAGE::LASER) <<"/\n";
                    ss << player->damageThreshold(DAMAGE::FIRE) <<"/\n";
                    ss << player->damageThreshold(DAMAGE::PLASMA) <<"/\n";
                    ss << player->damageThreshold(DAMAGE::EXPLOSIVE) <<"/";
                }
                auto damageThresholdLabel = new UI::TextArea(ss.str(), screenPos + Point{94, 40});
                damageThresholdLabel->setWidth(26);
                damageThresholdLabel->setHorizontalAlign(UI::TextArea::HorizontalAlign::RIGHT);
                addUI("damageThresholdLabel", damageThresholdLabel);
            }

            {
                std::stringstream ss;
                if (playerArmor != nullptr)
                {
                    ss << player->damageResist(DAMAGE::NORMAL) + playerArmor->damageResist(DAMAGE::NORMAL) << "%\n";
                    ss << player->damageResist(DAMAGE::LASER) + playerArmor->damageResist(DAMAGE::LASER) << "%\n";
                    ss << player->damageResist(DAMAGE::FIRE) + playerArmor->damageResist(DAMAGE::FIRE) << "%\n";
                    ss << player->damageResist(DAMAGE::PLASMA) + playerArmor->damageResist(DAMAGE::PLASMA) << "%\n";
                    ss << player->damageResist(DAMAGE::EXPLOSIVE) + playerArmor->damageResist(DAMAGE::NORMAL) << "%";
                }
                else
                {
                    ss << player->damageResist(DAMAGE::NORMAL) <<"%\n";
                    ss << player->damageResist(DAMAGE::LASER) <<"%\n";
                    ss << player->damageResist(DAMAGE::FIRE) <<"%\n";
                    ss << player->damageResist(DAMAGE::PLASMA) <<"%\n";
                    ss << player->damageResist(DAMAGE::EXPLOSIVE) <<"%";
                }
                auto damageResistanceLabel = new UI::TextArea(ss.str(), screenPos + Point{120, 40});
                addUI("damageResistanceLabel", damageResistanceLabel);
            }

            {
                auto line2 = new UI::Rectangle(screenPos + Point{0, 94}, Graphics::Size(142, 1), neonGreen);
                addUI("line2", line2);
            }

            {
                auto line3 = new UI::Rectangle(screenPos + Point{0, 134}, Graphics::Size(142, 1), neonGreen);
                addUI("line3", line3);
            }

            {
                auto totalWtLabel = new UI::TextArea(_t(MSG_INVENTORY, 20), screenPos + Point{14, 180});
                addUI("totalWtLabel", totalWtLabel);
            }

            auto weightMax = player->carryWeightMax();

            {
                auto weight = player->carryWeight();
                auto weightLabel = new UI::TextArea(std::to_string(weight), screenPos + Point{70, 180});
                weightLabel->setWidth(24);
                weightLabel->setHorizontalAlign(UI::TextArea::HorizontalAlign::RIGHT);
                if (weight>weightMax)
                {
                    weightLabel->setFont("font1.aaf", brightRed);
                }
                addUI("weightLabel", weightLabel);
            }

            {
                std::stringstream ss;
                ss << "/" << weightMax;
                auto weightMaxLabel = new UI::TextArea(ss.str(), screenPos + Point{94, 180});
                addUI("weightMaxLabel", weightMaxLabel);
            }

            Game::ItemObject* playerLhWeapon = player->leftHandSlot();

            {
                std::string label = playerLhWeapon ? _handItemSummary(playerLhWeapon) : "";
                auto leftHandLabel = new UI::TextArea(std::move(label), screenPos + Point{0, 100});
                addUI("leftHandLabel", leftHandLabel);
            }

            Game::ItemObject* playerRhWeapon = player->rightHandSlot();

            {
                std::string label = playerRhWeapon ? _handItemSummary(playerRhWeapon) : "";
                auto rightHandLabel = new UI::TextArea(std::move(label), screenPos + Point{0, 140});
                addUI("rightHandLabel", rightHandLabel);
            }

            {
                auto screenLabel = new UI::TextArea("", screenPos + Point{0, 20});
                screenLabel->setSize({140, 168}); // screen size
                screenLabel->setVisible(false);
                screenLabel->setWordWrap(true);
                addUI("screenLabel", screenLabel);
            }

            // Player model
            {
                // TODO: this is a rotating animation in vanilla FO2
                auto dude = Game::getInstance()->player();
                Helpers::CritterHelper critterHelper;
                Graphics::CritterAnimationFactory animationFactory;

                auto dudeCritter = animationFactory.buildStandingAnimation(
                        critterHelper.armorFID(dude.get()),
                        critterHelper.weaponId(dude.get()),
                        Game::Orientation::SC
                );

                dudeCritter->setPosition({188, 52});
                addUI(dudeCritter.release());
            }

            // List of items in inventory.
            // Players can drag items from here to the ARMOR, ITEM1, or ITEM2 slots, or rearrange
            // items within the items list.
            auto inventoryList = new UI::ItemsList(Point(40, 40));
            inventoryList->setItems(game->player()->inventory());
            addUI("inventory_list", inventoryList);

            // ARMOR slot
            {
                auto inventoryItem = new UI::InventoryItem(playerArmor, {154, 183});
                inventoryItem->setType(UI::InventoryItem::Type::SLOT);
                inventoryItem->itemDragStopHandler().add([inventoryList](Event::Mouse* event){
                    inventoryList->onItemDragStop(event);
                });
                inventoryList->itemDragStopHandler().add([inventoryItem](Event::Mouse* event){
                    inventoryItem->onArmorDragStop(event);
                });
                addUI(inventoryItem);
            }

            // ITEM1 (left hand) slot
            {
                auto inventoryItem = new UI::InventoryItem(playerLhWeapon, {154, 286});
                inventoryItem->setType(UI::InventoryItem::Type::SLOT);
                inventoryItem->itemDragStopHandler().add([inventoryList](Event::Mouse* event){
                    inventoryList->onItemDragStop(event, HAND::LEFT);
                });
                inventoryList->itemDragStopHandler().add([inventoryItem](Event::Mouse* event){
                    inventoryItem->onHandDragStop(event, HAND::LEFT);
                });
                addUI(inventoryItem);
            }

            // ITEM2 (right hand) slot
            {
                auto inventoryItem = new UI::InventoryItem(playerRhWeapon, {247, 286});
                inventoryItem->setType(UI::InventoryItem::Type::SLOT);
                inventoryItem->itemDragStopHandler().add([inventoryList](Event::Mouse* event){
                    inventoryList->onItemDragStop(event, HAND::RIGHT);
                });
                inventoryList->itemDragStopHandler().add([inventoryItem](Event::Mouse* event){
                    inventoryItem->onHandDragStop(event, HAND::RIGHT);
                });
                addUI(inventoryItem);
            }

            inventoryList->itemsListModifiedHandler().add([this](Event::Event* event){
                this->onInventoryModified();
            });

            // initialize inventory scroll buttons
            enableScrollUpButton(false);
            enableScrollDownButton(inventoryList->canScrollDown());
        }

        void Inventory::onDoneButtonClick(Event::Mouse* event)
        {
            Game::getInstance()->popState();
        }

        void Inventory::onScrollUpButtonClick(Event::Mouse* event)
        {
            auto inventory = dynamic_cast<UI::ItemsList*>(getUI("inventory_list"));
            if(inventory->canScrollUp())
            {
                inventory->scrollUp();
                //enable/disable scroll buttons on upward scroll
                if(!inventory->canScrollUp())
                {
                    enableScrollUpButton(false);
                }
                if(inventory->canScrollDown())
                {
                    enableScrollDownButton(true);
                }
            }
        }

        void Inventory::onScrollDownButtonClick(Event::Mouse* event)
        {
            auto inventory = dynamic_cast<UI::ItemsList*>(getUI("inventory_list"));
            if(inventory->canScrollDown())
            {
                inventory->scrollDown();
                //enable/disable scroll buttons on downward scroll
                if(!inventory->canScrollDown())
                {
                    enableScrollDownButton(false);
                }
                if(inventory->canScrollUp())
                {
                    enableScrollUpButton(true);
                }
            }
        }

        void Inventory::onInventoryModified()
        {
            auto inventory = dynamic_cast<UI::ItemsList*>(getUI("inventory_list"));
            /*
            this would scroll up when an item is removed and you are at the bottom
            of the list to fix the gap, but a bug is causing slotOffset to be crazy number
            if(inventory->items()->size() - inventory->slotOffset() <  inventory->slotsNumber())
            {
                inventory->scrollUp();
            }
            */
            enableScrollDownButton(inventory->canScrollDown());
            enableScrollUpButton(inventory->canScrollUp());
        }

        void Inventory::enableScrollUpButton(bool enable)
        {
            auto scrollUpButton = dynamic_cast<UI::ImageButton*>(getUI("button_up"));
            auto scrollUpButtonDisabled = dynamic_cast<UI::Image*>(getUI("button_up_disabled"));
            scrollUpButtonDisabled->setVisible(!enable);
            scrollUpButton->setEnabled(enable);
        }

        void Inventory::enableScrollDownButton(bool enable)
        {
            auto scrollDownButton = dynamic_cast<UI::ImageButton*>(getUI("button_down"));
            auto scrollDownButtonDisabled = dynamic_cast<UI::Image*>(getUI("button_down_disabled"));
            scrollDownButtonDisabled->setVisible(!enable);
            scrollDownButton->setEnabled(enable);
        }

        std::string Inventory::_handItemSummary (Game::ItemObject* hand)
        {
            if (hand)
            {
                std::stringstream ss;
                ss << hand->name() << "\n";
                // is it weapon
                if (hand->subtype() == Game::ItemObject::Subtype::WEAPON)
                {
                    auto weapon = dynamic_cast<Game::WeaponItemObject*>(hand);
                    ss << "Dmg: " << weapon->damageMin() << "-" << weapon->damageMax() << " ";
                    ss << "Rng: " << weapon->rangePrimary();
                    // has it ammo?
                    if (weapon->ammoType() != 0)
                    {
                        ss << "\nAmmo: /" << weapon->ammoCapacity() << " ";
                        auto ammo = Game::ObjectFactory::getInstance()->createObject(weapon->ammoPID());
                        ss << ammo->name();
                    }
                }
                return ss.str();
            }
            else
            {
                return "No item\nUnarmed dmg:";
            }
        }

        void Inventory::backgroundRightClick(Event::Mouse* event)
        {
            auto mouse = Game::getInstance()->mouse();
            if (mouse->state() == Input::Mouse::Cursor::ACTION)
            {
                mouse->pushState(Input::Mouse::Cursor::HAND);
            }
            else
            {
                mouse->popState();
                //state->_screenShow(1);
            }
            _screenShow(0);
        }

        void Inventory::_screenShow (unsigned int PID)
        {
            {
                auto playerNameLabel = getTextArea("player_name");
                auto screenLabel = getTextArea("screenLabel");
                if (PID == 0) {
                    auto player = Game::getInstance()->player();
                    playerNameLabel->setText(player->name());
                } else {
                    playerNameLabel->setText(_t(MSG_PROTO_ITEMS, PID * 100)); // item name
                    screenLabel->setText(_t(MSG_PROTO_ITEMS, PID * 100 + 1)); // item dedcription
                }
                screenLabel->setVisible(PID != 0);
            }

            static const char* textAreas[] = {
                "label_stats",
                "label_stats_values",
                "textLabel",
                "hitPointsLabel",
                "armorClassLabel",
                "damageThresholdLabel",
                "damageResistanceLabel",
                "totalWtLabel",
                "weightLabel",
                "weightMaxLabel",
                "leftHandLabel",
                "rightHandLabel",
            };

            for (const char* textArea : textAreas) {
                getTextArea(textArea)->setVisible(PID == 0);
            }

            getUI("line2")->setVisible(PID == 0);
            getUI("line3")->setVisible(PID == 0);
        }

        void Inventory::onKeyDown(Event::Keyboard* event)
        {
            if (event->keyCode() == SDLK_ESCAPE) {
                Game::getInstance()->popState();
            }
        }
    }
}
