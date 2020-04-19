#include <algorithm>
#include <memory>
#include "../State/State.h"
#include "../Event/State.h"
#include "../Game/Game.h"
#include "../Graphics/Renderer.h"
#include "../UI/ImageList.h"
#include "../UI/SmallCounter.h"
#include "../UI/TextArea.h"

namespace Falltergeist
{
    namespace State
    {
        State::State() : Event::EventTarget(Game::getInstance()->eventDispatcher())
        {
            activateHandler().add([this](Event::State* event) {
                this->onStateActivate(event);
            });
            deactivateHandler().add([this](Event::State* event) {
                this->onStateDeactivate(event);
            });
            keyDownHandler().add([this](Event::Keyboard* event) {
                this->onKeyDown(event);
            });
        }

        void State::init()
        {
            _initialized = true;
        }

        void State::think(const float &deltaTime)
        {
            for (auto& ui : _ui) {
                ui->think(deltaTime);
            }
        }

        int State::x() const
        {
            return _position.x();
        }

        void State::setX(int x)
        {
            _position.setX(x);
        }

        int State::y() const
        {
            return _position.y();
        }

        void State::setY(int y)
        {
            _position.setY(y);
        }

        const Point& State::position() const
        {
            return _position;
        }

        void State::setPosition(const Point& pos)
        {
            _position = pos;
        }

        bool State::initialized()
        {
            return _initialized;
        }

        bool State::fullscreen() const
        {
            return _fullscreen;
        }

        void State::setFullscreen(bool value)
        {
            _fullscreen = value;
        }

        bool State::modal() const
        {
            return _modal;
        }

        void State::setModal(bool value)
        {
            _modal = value;
        }

        std::shared_ptr<UI::Base> State::getUIInternal(const std::string& name) const
        {
            auto it = _labeledUI.find(name);
            return it != _labeledUI.end() ? it->second : std::shared_ptr<UI::Base>{nullptr};
        }

        void State::handle(Event::Event* event)
        {
            if (event->handled()) {
                return;
            }
            // TODO: maybe make handle() a template function to get rid of dynamic_casts?
            if (auto keyboardEvent = dynamic_cast<Event::Keyboard*>(event)) {
                if (keyboardEvent->originalType() == Event::Keyboard::Type::KEY_UP) {
                    emitEvent(std::make_unique<Event::Keyboard>(*keyboardEvent), keyUpHandler());
                }
                if (keyboardEvent->originalType() == Event::Keyboard::Type::KEY_DOWN) {
                    emitEvent(std::make_unique<Event::Keyboard>(*keyboardEvent), keyDownHandler());
                }
            }

            for (auto it = _ui.rbegin(); it != _ui.rend(); ++it) {
                if (event->handled()) {
                    return;
                }
                (*it)->handle(event);
            }
        }

        void State::render()
        {
            for (auto& ui : _ui) {
                if (ui->visible()) {
                    ui->render(false);
                }
            }
            _uiToDelete.clear();
        }

        void State::popUI()
        {
            if (not _ui.empty()) {
                _uiToDelete.emplace_back(std::move(_ui.back()));
                _ui.pop_back();
            }
        }

        void State::onStateActivate(Event::State* event)
        {
        }

        void State::onStateDeactivate(Event::State* event)
        {
        }

        void State::onKeyDown(Event::Keyboard* event)
        {
        }

        bool State::active()
        {
            return _active;
        }

        void State::setActive(bool value)
        {
            _active = value;
        }

        Event::StateHandler& State::pushHandler()
        {
            return _pushHandler;
        }

        Event::StateHandler& State::popHandler()
        {
            return _popHandler;
        }

        Event::StateHandler& State::activateHandler()
        {
            return _activateHandler;
        }

        Event::StateHandler& State::deactivateHandler()
        {
            return _deactivateHandler;
        }

        Event::StateHandler& State::fadeDoneHandler()
        {
            return _fadeDoneHandler;
        }

        Event::KeyboardHandler& State::keyDownHandler()
        {
            return _keyDownHandler;
        }

        Event::KeyboardHandler& State::keyUpHandler()
        {
            return _keyUpHandler;
        }

        void State::scriptFade(VM::Script *script, bool in)
        {
            fadeDoneHandler().clear();
            fadeDoneHandler().add([this, script](Event::Event* event) {
                fadeDoneHandler().clear();
                script->run();
            });

            if (in) {
                Game::getInstance()->renderer()->fadeIn(0, 0, 0, 1000);
            } else {
                Game::getInstance()->renderer()->fadeOut(0, 0, 0, 1000);
            }
        }
    }
}
