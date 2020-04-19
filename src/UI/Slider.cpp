#include <memory>
#include "../Audio/Mixer.h"
#include "../Event/Event.h"
#include "../Event/Mouse.h"
#include "../Game/Game.h"
#include "../UI/Image.h"
#include "../UI/Slider.h"

namespace Falltergeist
{
    namespace UI
    {
        Slider::Slider(Point pos, std::shared_ptr<Image> _imageOn, std::shared_ptr<Image> _imageOff) :
            Base{pos},
            imageOn{std::move(_imageOn)},
            imageOff{std::move(_imageOff)},
            _downSound{"sound/sfx/ib1p1xx1.acm"},
            _upSound{"sound/sfx/ib1lu1x1.acm"}
        {
            mouseDragHandler().add(std::bind(&Slider::_onDrag, this, std::placeholders::_1));
            mouseDownHandler().add(std::bind(&Slider::_onLeftButtonDown, this, std::placeholders::_1));
            mouseUpHandler().add(std::bind(&Slider::_onLeftButtonUp, this, std::placeholders::_1));
        }

        void Slider::handle(Event::Event* event)
        {
            if (auto mouseEvent = dynamic_cast<Event::Mouse*>(event)) {
                Point ofs = mouseEvent->position() - _position;

                bool opaque = imageOn->opaque(mouseEvent->position() - _offset);

                //if we are in slider coordinates and not on thumb (slider size = 218 + thumb size, thumb size = 21)
                if (ofs.x() > 0 && ofs.x() < 239 && ofs.y() > 0 && ofs.y() < imageOn->size().height() && !opaque)
                {
                    //on left button up only when not dragging thumb
                    if (mouseEvent->name() == "mouseup" && mouseEvent->leftButton() && !_drag)
                    {
                        ofs -= Point(10, 0); //~middle of thumb
                        if (ofs.x() < 0)
                        {
                            ofs.setX(0);
                        }
                        else if (ofs.y() > 218)
                        {
                            ofs.setX(218);
                        }
                        _offset.setX(ofs.x());
                        _value = ((maxValue() - minValue()) / 218.f) * (float)_offset.x();
                        emitEvent(std::make_unique<Event::Event>("change"), changeHandler());
                        Game::getInstance()->mixer()->playACMSound(_downSound);
                        Game::getInstance()->mixer()->playACMSound(_upSound);
                        return;
                    }
                }
            }
            //pass it to default handler
            Falltergeist::UI::Base::handle(event);
        }

        void Slider::_onDrag(Event::Mouse* event)
        {
            auto sender = dynamic_cast<Slider*>(event->target());
            auto newOffset = sender->_offset.x() + event->offset().x();
            if (newOffset <= 218 && newOffset >= 0)
            {
                sender->_offset.setX(newOffset);
                sender->_value = ((sender->maxValue() - sender->minValue())/218.f)*(float)sender->offset().x();
                emitEvent(std::make_unique<Event::Event>("change"), changeHandler());
            }
        }

        void Slider::_onLeftButtonDown(Event::Mouse* event)
        {
            auto sender = dynamic_cast<Slider*>(event->target());
            if (!sender->_downSound.empty())
            {
                Game::getInstance()->mixer()->playACMSound(sender->_downSound);
            }
        }

        void Slider::_onLeftButtonUp(Event::Mouse* event)
        {
            auto sender = dynamic_cast<Slider*>(event->target());
            if (!sender->_upSound.empty())
            {
                Game::getInstance()->mixer()->playACMSound(sender->_upSound);
            }
        }

        double Slider::minValue() const
        {
            return _minValue;
        }

        void Slider::setMinValue(double value)
        {
            _minValue = value;
        }

        double Slider::maxValue() const
        {
            return _maxValue;
        }

        void Slider::setMaxValue(double value)
        {
            _maxValue = value;
        }

        double Slider::value() const
        {
            return _value;
        }

        void Slider::setValue(double value)
        {
            _value = value;
            _offset.setX(static_cast<int>((218.f/(maxValue() - minValue())) * _value));
            emitEvent(std::make_unique<Event::Event>("change"), changeHandler());
        }

        Event::Handler& Slider::changeHandler()
        {
            return _changeHandler;
        }

        void Slider::render(bool eggTransparency)
        {
            imageOn->setPosition(position());
            imageOff->setPosition(position());
            if (_drag) {
                imageOn->render(eggTransparency);
            } else {
                imageOff->render(eggTransparency);
            }
        }

        bool Slider::opaque(const Point &pos)
        {
            return imageOn->opaque(pos);
        }

        Size Slider::size() const
        {
            return imageOn->size();
        }
    }
}
