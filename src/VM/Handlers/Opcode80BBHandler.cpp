/*
 * Copyright 2012-2014 Falltergeist Developers.
 *
 * This file is part of Falltergeist.
 *
 * Falltergeist is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Falltergeist is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Falltergeist.  If not, see <http://www.gnu.org/licenses/>.
 */

// C++ standard includes

// Falltergeist includes
#include "../../Logger.h"
#include "../../VM/Handlers/Opcode80BBHandler.h"
#include "../../Exception.h"
#include "../../Game.h"
#include "../../ResourceManager.h"
#include "../../Game/GameObject.h"
#include "../../Game/GameObjectFactory.h"
#include "../../State/Location.h"
#include "../../PathFinding/Hexagon.h"
#include "../../PathFinding/HexagonGrid.h"
#include "../../VM/VM.h"



// Third party includes

namespace Falltergeist
{

Opcode80BBHandler::Opcode80BBHandler(VM* vm) : OpcodeHandler(vm)
{
}

void Opcode80BBHandler::_run()
{
    Logger::debug("SCRIPT") << "[80BB] [+] int tile_contains_obj_pid(int position, int elevation, int PID)" << std::endl;
    auto PID = _vm->popDataInteger();
    auto elevation = _vm->popDataInteger();
    auto position = _vm->popDataInteger();
    auto game = Game::getInstance();
    int found = 0;
    for (auto object : *game->Location()->hexagonGrid()->at(position)->objects())
    {
        if (object->PID() == PID && object->elevation() == elevation)
        {
            found = 1;
        }
    }
    _vm->pushDataInteger(found);
}

}


