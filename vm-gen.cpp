// Generator of VM self-interpreter
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

#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <math.h>
#include <assert.h>

#include "vm.h"

void printTab(std::ostream& os, int32_t lev)
{
    for(int32_t i = 0; i < lev; ++i)
        os << " ";
}

void printLine(std::ostream& os, int32_t lev, const char* s)
{
    printTab(os, lev);
    os << s << std::endl;
}

const static char* codegen_error = "!error";

void genVerifyAddr(uint32_t arg_idx, const char** lines, uint32_t& line_count)
{
    // rb = arg1
    // rc = arg2
         if (arg_idx == 1) lines[line_count++] = "mov rd rb";
    else if (arg_idx == 2) lines[line_count++] = "mov rd rc";
    else {
        lines[line_count++] = codegen_error;
        assert(false);
    }
    lines[line_count++] = "sub rd m_base_offs";
    lines[line_count++] = "jl @execute_error_bounds rd";
    lines[line_count++] = "sub rd m_mem_size";
    lines[line_count++] = "jge @execute_error_bounds rd";
}

void genGetAddr(uint32_t arg_idx, const char** lines, uint32_t& line_count)
{
    // rb = arg1
    // rc = arg2
         if (arg_idx == 1) lines[line_count++] = "add rb m_data_offs";
    else if (arg_idx == 2) lines[line_count++] = "add rc m_data_offs";
    else {
        lines[line_count++] = codegen_error;
        assert(false);
    }
    genVerifyAddr(arg_idx, lines, line_count);
}

void genBinaryOp(OpCode opcode, const char** lines, uint32_t& line_count)
{
    // re = intermediate register = arg1 value
    // rc = arg2
    switch(opcode)
    {
    case OpCode::Add:
    case OpCode::Addv:
        lines[line_count++] = "add re rc";
        break;
    case OpCode::Sub:
    case OpCode::Subv:
        lines[line_count++] = "sub re rc";
        break;
    case OpCode::Mul:
    case OpCode::Mulv:
        lines[line_count++] = "mul re rc";
        break;
    case OpCode::Div:
    case OpCode::Divv:
        lines[line_count++] = "jz @execute_error_divzero rc";
        lines[line_count++] = "div re rc";
        break;
    default:
        lines[line_count++] = codegen_error;
        assert(false);
    }
}

void genDoJump(bool relative, const char** lines, uint32_t& line_count)
{
    // rb = relative or absolute position
    lines[line_count++] = "mov rd rb";
    lines[line_count++] = "divv rd 3";
    lines[line_count++] = "mulv rd 3";
    lines[line_count++] = "sub rd rb";
    lines[line_count++] = "jnz @execute_error_jump rd";
    if (relative) {
        lines[line_count++] = "add rb m_inst_addr";
    } else {
        lines[line_count++] = "add rb m_data_offs";
        lines[line_count++] = "subv rb 3";
    }
    genVerifyAddr(1, lines, line_count);
    lines[line_count++] = "addv rb 3";
    lines[line_count++] = "mov m_inst_addr rb";
}

void genCondJump(OpCode opcode, const char** lines, uint32_t& line_count)
{
    // rb = relative position
    // rc = condition value
    const char* label = codegen_error;
    switch(opcode) {
    case OpCode::Jz:
        lines[line_count++] = "jnz @execute_skip_jz rc";
        label = "@execute_skip_jz:";
        break;
    case OpCode::Jnz:
        lines[line_count++] = "jz @execute_skip_jnz rc";
        label = "@execute_skip_jnz:";
        break;
    case OpCode::Jg:
        lines[line_count++] = "jle @execute_skip_jg rc";
        label = "@execute_skip_jg:";
        break;
    case OpCode::Jl:
        lines[line_count++] = "jge @execute_skip_jl rc";
        label = "@execute_skip_jl:";
        break;
    case OpCode::Jge:
        lines[line_count++] = "jl @execute_skip_jge rc";
        label = "@execute_skip_jge:";
        break;
    case OpCode::Jle:
        lines[line_count++] = "jg @execute_skip_jle rc";
        label = "@execute_skip_jle:";
        break;
    default:
        assert(false);
    }
    genDoJump(true, lines, line_count);
    lines[line_count++] = label;
}

void genExecuteOp(std::ostream& os, OpCode opcode, int32_t lev)
{
    constexpr uint32_t max_lines = 64;
    const char* lines[max_lines];
    uint32_t line_count = 0;
    switch(opcode) {
    case OpCode::Nop:
        break;
    case OpCode::Hlt:
        lines[line_count++] = "jr @execute_loopend";
        break;
    case OpCode::Ja:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "ld rb rb";
        lines[line_count++] = "addv rb 1";
        genDoJump(false, lines, line_count);
        break;
    case OpCode::Jr:
        genDoJump(true, lines, line_count);
        break;
    case OpCode::Jz:
    case OpCode::Jnz:
    case OpCode::Jg:
    case OpCode::Jl:
    case OpCode::Jge:
    case OpCode::Jle:
        genGetAddr(2, lines, line_count);
        lines[line_count++] = "ld rc rc";
        genCondJump(opcode, lines, line_count);
        break;
    case OpCode::Lia:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "mov rd rc";
        lines[line_count++] = "add rd m_inst_addr";
        lines[line_count++] = "addv rd 2";
        lines[line_count++] = "sub rd m_data_offs";
        lines[line_count++] = "st rb rd";
        break;
    case OpCode::Ld:
        genGetAddr(1, lines, line_count);
        genGetAddr(2, lines, line_count);
        lines[line_count++] = "ld rc rc";
        genGetAddr(2, lines, line_count);
        lines[line_count++] = "ld rc rc";
        lines[line_count++] = "st rb rc";
        break;
    case OpCode::St:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "ld rb rb";
        genGetAddr(1, lines, line_count);
        genGetAddr(2, lines, line_count);
        lines[line_count++] = "ld rc rc";
        lines[line_count++] = "st rb rc";
        break;
    case OpCode::Stv:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "ld rb rb";
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "st rb rc";
        break;
    case OpCode::Mov:
        genGetAddr(1, lines, line_count);
        genGetAddr(2, lines, line_count);
        lines[line_count++] = "ld rc rc";
        lines[line_count++] = "st rb rc";
        break;
    case OpCode::Add:
    case OpCode::Sub:
    case OpCode::Mul:
    case OpCode::Div:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "ld re rb";
        genGetAddr(2, lines, line_count);
        lines[line_count++] = "ld rc rc";
        genBinaryOp(opcode, lines, line_count);
        lines[line_count++] = "st rb re";
        break;
    case OpCode::Movv:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "st rb rc";
        break;
    case OpCode::Addv:
    case OpCode::Subv:
    case OpCode::Mulv:
    case OpCode::Divv:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "ld re rb";
        genBinaryOp(opcode, lines, line_count);
        lines[line_count++] = "st rb re";
        break;
    case OpCode::Dbg:
        genGetAddr(1, lines, line_count);
        lines[line_count++] = "ld rb rb";
        lines[line_count++] = "dbg rb";
        break;
    case OpCode::Dbgext:
        lines[line_count++] = "dbgext";
        break;
    default:
        lines[line_count++] = codegen_error;
        assert(false);
    }
    assert(line_count <= max_lines);
    for(uint32_t i = 0; i < line_count; ++i)
        printLine(os, lev, lines[i]);
}

void genBinaryOpSwitchRec(std::ostream& os, int32_t s, int32_t e, int32_t lev)
{
    // if op > n/2 goto op > n/2
    // op <= n/2:
    //   ...
    // op > n/2:
    //   ...
    if (s == e) {
        os << std::endl;
        printTab(os, lev);
        const auto& d = opcode_def[static_cast<uint32_t>(s)];
        os << "% " << d.first << std::endl;
        genExecuteOp(os, d.second, lev);
        printTab(os, lev);
        os << "jr @execute_continue" << std::endl;
        os << std::endl;
        return;
    }
    int32_t m = (s + e) >> 1;
    const auto& d = opcode_def[static_cast<uint32_t>(m)];
    printTab(os, lev);
    os << "mov rd ra" << std::endl;
    printTab(os, lev);
    os << "subv rd $" << d.first << std::endl;
    printTab(os, lev);
    os << "jg @execute_after_" << d.first << " rd" << std::endl;
    genBinaryOpSwitchRec(os, s, m, lev + 1);
    printTab(os, lev);
    os << "@execute_after_" << d.first << ":" << std::endl;
    genBinaryOpSwitchRec(os, m + 1, e, lev + 1);
}

void genExecuteProgram(std::ostream& os)
{
    const char* autogen_begin = "%%% auto-generated begin: do not edit %%%\n\n";
    const char* autogen_end = "\n%%% auto-generated end %%%\n";

    os << autogen_begin;
    os << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
    os << "%%% Self-interpreting VM, Tomasz Dobrowolski (C) 2019 %%%\n";
    os << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

    const std::vector<std::string> regs = {
      "top",
      "ret_val",
      "param",
      "ra",
      "rb",
      "rc",
      "rd",
      "re",
      "rcnt",
      "m_inst_addr",
      "m_base_offs",
      "m_data_offs",
      "m_mem_size",
    };
    for(size_t i = 0; i < regs.size(); ++i) {
        os << "def " << regs[i] << " " << i << std::endl;
    }
    os << std::endl;

    os << "jr @main\n\n";

    const char* execute_begin =
        "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
        "%% Execute self-interpreting machine code   %%\n"
        "%% with safety checks.                      %%\n"
        "%% Arguments:                               %%\n"
        "%%  m_base_offs = machine memory start      %%\n"
        "%%  m_data_offs = machine data start        %%\n"
        "%%                (and code size limit)     %%\n"
        "%%  m_mem_size = total code+data size limit %%\n"
        "%% Returns:                                 %%\n"
        "%%  ret_val = error code or program ret_val %%\n"
        "%%  -11111112 = invalid jump location       %%\n"
        "%%  -11111113 = out-of-bounds memory access %%\n"
        "%%  -11111114 = division by zero            %%\n"
        "%%  -11111115 = infinite loop               %%\n"
        "%%  -11111116 = unknown operation code      %%\n"
        "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
        "@execute_program:\n"
        " mov m_inst_addr m_data_offs\n"
        "\n"
        " movv rcnt 10000000 % execution limit\n"
        " @execute_loop:\n"
        "  subv m_inst_addr 1\n"
        "  ld ra m_inst_addr\n"
        "  subv m_inst_addr 1\n"
        "  ld rb m_inst_addr\n"
        "  subv m_inst_addr 1\n"
        "  ld rc m_inst_addr\n"
        "\n"
        "  % ra = opcode, rb = arg1, rc = arg2\n"
        "\n"
        "  mov rd ra\n"
        "  jl @execute_error_opcode rd  % invalid opcode\n"
        "  subv rd $dbgext\n"
        "  jg @execute_error_opcode rd  % invalid opcode\n"
        "\n"
        "  % switch as binary search tree\n"
        "\n";
    os << execute_begin;

    genBinaryOpSwitchRec(os, 0, static_cast<int32_t>(opcode_def.size()) - 1, 2);

    const char* execute_end =
       "  @execute_continue:\n"
       "\n"
       "  subv rcnt 1\n"
       "  jg @execute_loop rcnt\n"
       "  jr @execute_error_infloop\n"
       " @execute_loopend:\n"
       "\n"
       " mov ra m_data_offs\n"
       " addv ra ret_val\n"
       " ld ret_val ra\n"
       "\n"
       " jr @execute_errorend\n"
       " @execute_error_jump:\n"
       "  dbg rb\n"
       "  movv ret_val -11111112\n"
       "  jr @execute_errorend\n"
       " @execute_error_bounds:\n"
       "  dbg rd\n"
       "  movv ret_val -11111113\n"
       "  jr @execute_errorend\n"
       " @execute_error_divzero:\n"
       "  movv ret_val -11111114\n"
       "  jr @execute_errorend\n"
       " @execute_error_infloop:\n"
       "  movv ret_val -11111115\n"
       "  jr @execute_errorend\n"
       " @execute_error_opcode:\n"
       "  movv ret_val -11111116\n"
       " @execute_errorend:\n"
       "\n"
       " addv top 1\n"
       " ld ra top\n"
       " ja ra\n";
    os << execute_end;
    os << autogen_end;
}

bool genExecuteProgram(const char* file_path)
{
    std::ofstream fp;
    fp.open(file_path);
    if (!fp)
        return false;
    genExecuteProgram(fp);
    return true;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "usage: vm-gen <output text file>" << std::endl;
        return -1;
    }
    if (!genExecuteProgram(argv[1])) {
        return -1;
    }
    return 0;
}
