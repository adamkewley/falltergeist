#include <algorithm>
#include <vector>
#include "../functions.h"
#include "../State/CritterDialog.h"
#include "../Exception.h"
#include "../Format/Int/File.h"
#include "../Format/Int/Procedure.h"
#include "../Game/CritterObject.h"
#include "../Game/Game.h"
#include "../Graphics/Renderer.h"
#include "../LocationCamera.h"
#include "../PathFinding/Hexagon.h"
#include "../ResourceManager.h"
#include "../State/CritterInteract.h"
#include "../State/CritterDialogReview.h"
#include "../State/Location.h"
#include "../UI/AnimationQueue.h"
#include "../UI/Factory/ImageButtonFactory.h"
#include "../UI/Image.h"
#include "../UI/TextArea.h"
#include "../VM/Script.h"

namespace Falltergeist
{
    namespace State
    {
        using ImageButtonType = UI::Factory::ImageButtonFactory::Type;

        CritterDialog::CritterDialog(std::shared_ptr<UI::IResourceManager> _resourceManager) :
            State{},
            resourceManager{std::move(_resourceManager)},
            imageButtonFactory{std::make_unique<UI::Factory::ImageButtonFactory>(resourceManager)}
        {
        }

        void CritterDialog::init()
        {
            if (_initialized) {
                return;
            }

            State::init();

            setFullscreen(false);
            setModal(false);

            setPosition((Game::getInstance()->renderer()->size() - Point(640, 480)) / 2 + Point(0, 291));

            addUI("background", resourceManager->getImage("art/intrface/di_talk.frm"));

            auto question = makeNamedUI<UI::TextArea>("question", 140, -62);
            //auto question = new UI::TextArea("question", 140, -62);
            question->setSize({375, 53});
            // TODO: maybe padding properties should be removed from TextArea to simplify it. Use invisible panel for mouse interactions.
            question->setPadding({0, 5}, {0, 5});
            question->setWordWrap(true);

            // TODO: maybe move text scrolling into separate UI? Though it is only in two places and works slightly differently...
            question->mouseClickHandler().add([question](Event::Mouse* event) {
                Point relPos = event->position() - question->position();
                if (relPos.y() < (question->size().height() / 2))
                {
                    if (question->lineOffset() > 0)
                    {
                        question->setLineOffset(question->lineOffset() - 4);
                    }
                }
                else if (question->lineOffset() < question->numLines() - 4)
                {
                    question->setLineOffset(question->lineOffset() + 4);
                }
            });

            question->mouseMoveHandler().add([question](Event::Mouse* event) {
                if (question->numLines() > 4)
                {
                    auto mouse = Game::getInstance()->mouse();
                    Point relPos = event->position() - question->position();
                    auto state = relPos.y() < (question->size().height() / 2)
                        ? Input::Mouse::Cursor::SMALL_UP_ARROW
                        : Input::Mouse::Cursor::SMALL_DOWN_ARROW;

                    if (mouse->state() != state)
                    {
                        mouse->setState(state);
                    }
                }
            });

            question->mouseOutHandler().add([](Event::Mouse* event) {
                Game::getInstance()->mouse()->setState(Input::Mouse::Cursor::BIG_ARROW);
            });

            // Interface buttons
            auto reviewButton = addUI(imageButtonFactory->getByType(ImageButtonType::DIALOG_REVIEW_BUTTON, {13, 154}));
            reviewButton->mouseClickHandler().add(std::bind(&CritterDialog::onReviewButtonClick, this, std::placeholders::_1));

            auto barterButton = addUI(imageButtonFactory->getByType(ImageButtonType::DIALOG_RED_BUTTON, {593, 40}));
            barterButton->mouseClickHandler().add(std::bind(&CritterDialog::onBarterButtonClick, this, std::placeholders::_1));
        }

        // TODO: add auto-text scrolling after 10 seconds (when it's longer than 4 lines)
        void CritterDialog::setQuestion(const std::string& value)
        {
            auto game = Game::getInstance();
            auto dialog = dynamic_cast<CritterInteract*>(game->topState(1));

            dialog->dialogReview()->addQuestion(std::string("  ") + value);

            auto question = getUI<UI::TextArea>("question");
            question->setText(std::string("  ") + value);
            question->setLineOffset(0);
        }

        void CritterDialog::onAnswerIn(Event::Mouse* event)
        {
            auto sender = dynamic_cast<UI::TextArea*>(event->target());
            sender->setFont("font1.aaf", {0xff, 0xff, 0x7f, 0xff});
        }

        void CritterDialog::onAnswerOut(Event::Mouse* event)
        {
            auto sender = dynamic_cast<UI::TextArea*>(event->target());
            sender->setFont("font1.aaf", {0x3f,0xf8, 0x00, 0xff});
        }

        std::vector<int>* CritterDialog::functions()
        {
            return &_functions;
        }

        std::vector<int>* CritterDialog::reactions()
        {
            return &_reactions;
        }

        void CritterDialog::deleteAnswers()
        {
            while(!_answers.empty())
            {
                _answers.pop_back();
                popUI();
            }
            _functions.clear();
            _reactions.clear();
        }

        void CritterDialog::onReviewButtonClick(Event::Mouse* event)
        {
            if (auto interact = dynamic_cast<CritterInteract*>(Game::getInstance()->topState(1)))
            {
                interact->switchSubState(CritterInteract::SubState::REVIEW);
            }
        }

        void CritterDialog::onBarterButtonClick(Event::Mouse* event)
        {
            if (auto interact = dynamic_cast<CritterInteract*>(Game::getInstance()->topState(1)))
            {
                if (interact->critter()->canTrade()) {
                    interact->switchSubState(CritterInteract::SubState::BARTER);
                } else {
                    getUI<UI::TextArea>("question")->setText(_t(MSG_PROTO, 903));
                }
            }
        }

        void CritterDialog::onKeyDown(Event::Keyboard* event)
        {
            static std::vector<uint32_t> numkeys = {
                    SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9,
                    SDLK_KP_1, SDLK_KP_2, SDLK_KP_3, SDLK_KP_4, SDLK_KP_5, SDLK_KP_6, SDLK_KP_7, SDLK_KP_8, SDLK_KP_9,
            };

            auto key = event->keyCode();

            if (key == SDLK_0 || key == SDLK_KP_0)
            {
                // Todo: end dialog
                return;
            }

            auto keyIt = std::find(numkeys.begin(), numkeys.end(), key);
            // Number key pressed
            if (keyIt != numkeys.end())
            {
                size_t keyOffset = keyIt - numkeys.begin();

                // If numpad key
                if (keyOffset > 8) keyOffset -= 9;

                if (keyOffset < _answers.size()) _selectAnswer(keyOffset);
                return;
            }

            auto question = getUI<UI::TextArea>("question");
            if (key == SDLK_UP && question->lineOffset() > 0)
            {
                question->setLineOffset(question->lineOffset() - 4);
            }
            else if (key == SDLK_DOWN && question->lineOffset() < question->numLines() - 4)
            {
                question->setLineOffset(question->lineOffset() + 4);
            }
        }

        void CritterDialog::_selectAnswer(size_t i)
        {
            if (i >= _answers.size()) throw Exception("No answer with number " + std::to_string(i));

            auto game = Game::getInstance();
            auto dialog = dynamic_cast<CritterInteract*>(game->topState(1));

            dialog->dialogReview()->addAnswer(_answers.at(i)->text().substr(1));

            // @todo optimize
            auto script = dialog->script();
            int newOffset = script->script()->procedures().at(_functions.at(i)).bodyOffset();
            int oldOffset = script->programCounter() - 2;
            int reaction = 50;
            if (i < _reactions.size())
            {
                reaction = _reactions.at(i);
            }
            deleteAnswers();
            script->dataStack()->push(0); // arguments counter;
            script->returnStack()->push(oldOffset); // return adrress
            script->setProgramCounter(newOffset);
            // play transition, if needed, then run script.
            dialog->transition(static_cast<CritterInteract::Reaction>(reaction));
            //dialog->script()->run();
        }


        void CritterDialog::addAnswer(const std::string& text)
        {
            std::string line = "";
            line += '\x95';
            line += " ";
            line += text;

            int y = 50;
            for (auto answer : _answers)
            {
                y += answer->textSize().height() + 5;
            }

            auto answer = makeUI<UI::TextArea>(line, 140, y);
            answer->setWordWrap(true);
            answer->setSize({370, 0});

            answer->mouseInHandler().add(std::bind(&CritterDialog::onAnswerIn, this, std::placeholders::_1));
            answer->mouseOutHandler().add(std::bind(&CritterDialog::onAnswerOut, this, std::placeholders::_1));
            answer->mouseClickHandler().add(std::bind(&CritterDialog::onAnswerClick, this, std::placeholders::_1));
            _answers.push_back(answer);
        }

        bool CritterDialog::hasAnswers()
        {
            return _answers.size() > 0;
        }

        void CritterDialog::onAnswerClick(Event::Mouse* event)
        {
            auto sender = dynamic_cast<UI::TextArea*>(event->target());

            size_t i = 0;
            for (const auto& answer : _answers)
            {
                if (answer.get() == sender)
                {
                    _selectAnswer(i);
                    return;
                }
                ++i;
            }
        }
    }
}
