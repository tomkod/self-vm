// Simple VM interpreter
// Copyright (C) 2019 Tomasz Dobrowolski
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>
#include <vector>
#include <string>

/*
* VM instructions
*
* All numbers are int32's.
*
* Program starts in memory at addr = -1 and goes to negative side.
*
* Registers:
*   Instruction address (it's better to hide it, not accessible).
*
* Instructions:
*   nop
*   hlt (halt)
*   jr rel_index (relative jump to instruction)
*   ja addr (absolute jump to instruction)
*   j[nz/z/g/ge/l/le] rel_index, addr (conditional jump up/down)
*   lia addr rel_index offset (load instruction address to [addr] of this instruction, offset is 0..2)
*   ld addr [src_addr] (load value to addr from indirect address pointed by [src_addr])
*   st[v] [addr] src_addr/value (store value from src_addr/value to indirect address pointed by [addr])
*   mov[v] addr addr/value
*   add[v] addr addr/value
*   sub[v] addr addr/value
*   mul[v] addr addr/value
*   div[v] addr addr/value
*   dbg addr (print value at addr)
*   dbgext (debug various internal machine stats; now just prints base cycles)
*
* Special commands
*   % (comment)
*   def symbol <integer constant>
*   enum symbol (defines constant with value = last integer constant + 1)
*   $opcode (default symbols for opcode encoding)
*   include <path> (include code)
*/

enum class OpCode : int32_t
{
    Nop = 0,
    Hlt,
    Jr,
    Ja,
    Jnz,
    Jz,
    Jg,
    Jge,
    Jl,
    Jle,
    Lia,
    Ld,
    St,
    Stv,
    Mov,
    Add,
    Sub,
    Mul,
    Div,
    Movv,
    Addv,
    Subv,
    Mulv,
    Divv,
    Dbg,
    Dbgext
};

const static std::vector<std::pair<std::string, OpCode>> opcode_def{
    { "nop", OpCode::Nop },
    { "hlt", OpCode::Hlt },
    { "jr", OpCode::Jr },
    { "ja", OpCode::Ja },
    { "jnz", OpCode::Jnz },
    { "jz", OpCode::Jz },
    { "jg", OpCode::Jg },
    { "jge", OpCode::Jge },
    { "jl", OpCode::Jl },
    { "jle", OpCode::Jle },
    { "lia", OpCode::Lia },
    { "ld", OpCode::Ld },
    { "st", OpCode::St },
    { "stv", OpCode::Stv },
    { "mov", OpCode::Mov },
    { "add", OpCode::Add },
    { "sub", OpCode::Sub },
    { "mul", OpCode::Mul },
    { "div", OpCode::Div },
    { "movv", OpCode::Movv },
    { "addv", OpCode::Addv },
    { "subv", OpCode::Subv },
    { "mulv", OpCode::Mulv },
    { "divv", OpCode::Divv },
    { "dbg", OpCode::Dbg },
    { "dbgext", OpCode::Dbgext },
};
