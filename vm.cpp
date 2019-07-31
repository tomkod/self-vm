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

enum class Result : int32_t
{
    Continue = 0,
    Halt,
    InfiniteLoop,
    InvalidInstAddr,
    InvalidDataAddr,
    InvalidJumpAddr,
    InvalidOpCode,
    DivByZero
};

constexpr int32_t InstSize = 3; // every instruction is 3x int32

struct Machine
{
    int32_t inst_addr; // should be initialized to data_offset at start
    int32_t data_offset;
    int32_t mem_size;
    int32_t cycles;
    int32_t max_cycles;
    int32_t last_dbgext_cycles;
    std::vector<int32_t> mem;
};

Result execute(Machine& m)
{
    #define GetAddr(ret, arg) \
        const uint32_t ret = static_cast<uint32_t>(arg + m.data_offset); \
        if (ret >= static_cast<uint32_t>(m.mem_size)) \
            return Result::InvalidDataAddr;
    #define DoJump(base_addr, rel_addr) { \
        if ((rel_addr % InstSize) != 0) \
            return Result::InvalidJumpAddr; \
        const int32_t inst_addr2 = base_addr + rel_addr; \
        if (inst_addr2 < 0 || inst_addr2 >= m.mem_size) \
            return Result::InvalidJumpAddr; \
        m.inst_addr = inst_addr2 + InstSize; \
    }

    const int32_t inst_addr = m.inst_addr - InstSize;
    m.inst_addr = inst_addr;
    if (inst_addr < 0 || inst_addr >= m.mem_size)
        return Result::InvalidInstAddr;
    const OpCode opcode = static_cast<OpCode>(m.mem[static_cast<uint32_t>(inst_addr + 2)]);
    const int32_t arg1 = m.mem[static_cast<uint32_t>(inst_addr + 1)];
    const int32_t arg2 = m.mem[static_cast<uint32_t>(inst_addr)];
    switch(opcode) {
    case OpCode::Nop:
        break;
    case OpCode::Hlt:
        return Result::Halt;
    case OpCode::Ja:
    {
        GetAddr(addr1, arg1)
        int32_t rel_addr = m.mem[addr1] + 1;
        DoJump(m.data_offset - InstSize, rel_addr)
        break;
    }
    case OpCode::Jr:
    {
        DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Jnz:
    {
        GetAddr(addr2, arg2)
        if (m.mem[addr2] != 0)
            DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Jz:
    {
        GetAddr(addr2, arg2)
        if (m.mem[addr2] == 0)
            DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Jg:
    {
        GetAddr(addr2, arg2)
        if (m.mem[addr2] > 0)
            DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Jge:
    {
        GetAddr(addr2, arg2)
        if (m.mem[addr2] >= 0)
            DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Jl:
    {
        GetAddr(addr2, arg2)
        if (m.mem[addr2] < 0)
            DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Jle:
    {
        GetAddr(addr2, arg2)
        if (m.mem[addr2] <= 0)
            DoJump(inst_addr, arg1)
        break;
    }
    case OpCode::Lia:
    {
        GetAddr(addr1, arg1)
        int32_t abs_addr = inst_addr + InstSize - 1 + arg2;
        m.mem[addr1] = abs_addr - m.data_offset;
        break;
    }
    case OpCode::Ld:
    {
        GetAddr(addr1, arg1)
        GetAddr(paddr2, arg2)
        GetAddr(addr2, m.mem[paddr2])
        m.mem[addr1] = m.mem[addr2];
        break;
    }
    case OpCode::St:
    {
        GetAddr(paddr1, arg1)
        GetAddr(addr1, m.mem[paddr1])
        GetAddr(addr2, arg2)
        m.mem[addr1] = m.mem[addr2];
        break;
    }
    case OpCode::Stv:
    {
        GetAddr(paddr1, arg1)
        GetAddr(addr1, m.mem[paddr1])
        m.mem[addr1] = arg2;
        break;
    }
    case OpCode::Mov:
    {
        GetAddr(addr1, arg1)
        GetAddr(addr2, arg2)
        m.mem[addr1] = m.mem[addr2];
        break;
    }
    case OpCode::Add:
    {
        GetAddr(addr1, arg1)
        GetAddr(addr2, arg2)
        m.mem[addr1] += m.mem[addr2];
        break;
    }
    case OpCode::Sub:
    {
        GetAddr(addr1, arg1)
        GetAddr(addr2, arg2)
        m.mem[addr1] -= m.mem[addr2];
        break;
    }
    case OpCode::Mul:
    {
        GetAddr(addr1, arg1)
        GetAddr(addr2, arg2)
        m.mem[addr1] *= m.mem[addr2];
        break;
    }
    case OpCode::Div:
    {
        GetAddr(addr1, arg1)
        GetAddr(addr2, arg2)
        const int32_t d = m.mem[addr2];
        if (!d)
            return Result::DivByZero;
        m.mem[addr1] /= d;
        break;
    }
    case OpCode::Movv:
    {
        GetAddr(addr1, arg1)
        m.mem[addr1] = arg2;
        break;
    }
    case OpCode::Addv:
    {
        GetAddr(addr1, arg1)
        m.mem[addr1] += arg2;
        break;
    }
    case OpCode::Subv:
    {
        GetAddr(addr1, arg1)
        m.mem[addr1] -= arg2;
        break;
    }
    case OpCode::Mulv:
    {
        GetAddr(addr1, arg1)
        m.mem[addr1] *= arg2;
        break;
    }
    case OpCode::Divv:
    {
        GetAddr(addr1, arg1)
        if (!arg2)
            return Result::DivByZero;
        m.mem[addr1] /= arg2;
        break;
    }
    case OpCode::Dbg:
    {
        GetAddr(addr1, arg1)
        std::cout << "dbg " << addr1 << " [" << arg1 << "]: " << m.mem[addr1] << std::endl;
        break;
    }
    case OpCode::Dbgext:
    {
        std::cout << "base cycles = " << m.cycles
                  << ", diff = " << (m.cycles - m.last_dbgext_cycles) <<  std::endl;
        m.last_dbgext_cycles = m.cycles;
        break;
    }
    default:
        return Result::InvalidOpCode;
    }
    if (++m.cycles >= m.max_cycles)
        return Result::InfiniteLoop;

    #undef DoJump
    #undef GetAddr
    return Result::Continue;
}

struct ParsePos
{
    const char* code_text;
    uint32_t code_size;
    uint32_t index;
    uint32_t line;
};

void skipWhiteSpace(ParsePos& pos)
{
    const uint32_t code_size = pos.code_size;
    for(;pos.index < code_size; ++pos.index) {
        char ch = pos.code_text[pos.index];
        if (static_cast<unsigned char>(ch) > 32)
            break;
        if (ch == '\n')
            ++pos.line;
    }
}

bool parseString(std::string& ret, ParsePos& pos)
{
    skipWhiteSpace(pos);
    ret.resize(0);
    const uint32_t code_size = pos.code_size;
    for(;pos.index < code_size; ++pos.index) {
        char ch = pos.code_text[pos.index];
        if (static_cast<unsigned char>(ch) <= 32)
            break;
        ret.push_back(ch);
    }
    return ret.size() > 0;
}

void skipLine(ParsePos& pos)
{
    const uint32_t code_size = pos.code_size;
    for(;pos.index < code_size; ++pos.index) {
        char ch = pos.code_text[pos.index];
        if (ch == '\n' || ch == '\r')
            break;
    }
    for(;pos.index < code_size; ++pos.index) {
        char ch = pos.code_text[pos.index];
        if (ch != '\n' && ch != '\r')
            break;
        if (ch == '\n')
            ++pos.line;
    }
}

struct Op
{
    OpCode code;
    int32_t arg1;
    int32_t arg2;
};

bool isInteger(const std::string& arg)
{
    if (arg.empty())
        return false;
    uint32_t index = (arg[0] == '-') ? 1 : 0;
    const uint32_t size = static_cast<uint32_t>(arg.size());
    for(; index < size; ++index) {
        char ch = arg[index];
        if (ch < '0' || ch > '9')
            return false;
    }
    return true;
}

bool getValue(const std::unordered_map<std::string, int32_t>& consts, const std::string& arg, int32_t& ret_val)
{
    auto it = consts.find(arg);
    if (it != consts.end()) {
        ret_val = it->second;
        return true;
    }
    if (!isInteger(arg))
        return false;
    ret_val = std::atoi(arg.c_str());
    return true;
}

bool getRelIndex(const std::unordered_map<std::string, int32_t>& labels, const std::string& arg, int32_t inst_offs, int32_t& ret_val)
{
    auto it = labels.find(arg);
    if (it != labels.end()) {
        ret_val = inst_offs - it->second;
        return true;
    }
    if (!isInteger(arg))
        return false;
    ret_val = std::atoi(arg.c_str());
    return true;
}

struct Symbols
{
    std::unordered_map<std::string, int32_t> labels;
    std::unordered_map<std::string, OpCode> opcodes;
    std::unordered_map<std::string, int32_t> consts;
    int32_t last_const;
};

void initSymbols(Symbols& sym)
{
    int32_t prev = -1;
    for(const auto& p : opcode_def) {
        assert(static_cast<int32_t>(p.second) > prev);
        prev = static_cast<int32_t>(p.second);
        sym.opcodes[p.first] = p.second;
        sym.consts["$" + p.first] = static_cast<int32_t>(p.second);
    }
    sym.last_const = -1;
}

bool compile(std::vector<Op>& ret_ops, Symbols& sym,
    const char* code_text, uint32_t code_size, uint32_t& error_line)
{
    std::string cmd, arg1, arg2, subarg2;
    ParsePos pos;
    pos.code_text = code_text;
    pos.code_size = code_size;

    #define RetError { \
        error_line = pos.line; \
        return false; \
    }

    for(uint32_t pass = 0; pass < 2; ++pass) {
        int32_t inst_offs = 0;
        pos.index = 0;
        pos.line = 1;
        while(parseString(cmd, pos)) {
            if (cmd.front() == '%') {
                skipLine(pos);
                continue;
            }
            if (cmd.back() == ':') {
                if (pass == 1)
                    continue;
                cmd.pop_back();
                if (sym.labels.find(cmd) != sym.labels.end())
                    RetError
                sym.labels[cmd] = inst_offs;
                continue;
            }
            if (cmd == "enum") {
                if (!parseString(arg1, pos))
                    RetError
                if (pass == 1)
                    continue;
                if (sym.consts.find(cmd) != sym.consts.end())
                    RetError
                sym.consts[arg1] = ++sym.last_const;
                continue;
            }
            if (cmd == "def") {
                if (!parseString(arg1, pos))
                    RetError
                if (!parseString(arg2, pos))
                    RetError
                if (pass == 1)
                    continue;
                if (sym.consts.find(cmd) != sym.consts.end())
                    RetError
                sym.consts[arg1] = sym.last_const = static_cast<int32_t>(std::atoi(arg2.c_str()));
                continue;
            }
            OpCode opcode;
            {
                auto it = sym.opcodes.find(cmd);
                if (it == sym.opcodes.end())
                    RetError
                opcode = it->second;
            }
            Op op = {opcode, 0, 0};
            switch(opcode) {
            case OpCode::Nop:
            case OpCode::Hlt:
            case OpCode::Dbgext:
                break;
            case OpCode::Ja:
            case OpCode::Dbg:
            {
                if (!parseString(arg1, pos))
                    RetError
                if (pass == 1) {
                    if (!getValue(sym.consts, arg1, op.arg1))
                        RetError
                }
                break;
            }
            case OpCode::Jr:
            case OpCode::Jnz:
            case OpCode::Jz:
            case OpCode::Jg:
            case OpCode::Jge:
            case OpCode::Jl:
            case OpCode::Jle:
            {
                if (!parseString(arg1, pos))
                    RetError
                if (pass == 1) {
                    if (!getRelIndex(sym.labels, arg1, inst_offs, op.arg1))
                        RetError
                }
                if (opcode != OpCode::Jr) {
                    if (!parseString(arg2, pos))
                        RetError
                    if (pass == 1 && !getValue(sym.consts, arg2, op.arg2))
                        RetError
                }
                break;
            }
            case OpCode::Lia:
                if (!parseString(arg1, pos))
                    RetError
                if (!parseString(arg2, pos))
                    RetError
                if (!parseString(subarg2, pos))
                    RetError
                if (pass == 1) {
                    if (!getValue(sym.consts, arg1, op.arg1))
                        RetError
                    if (!getRelIndex(sym.labels, arg2, inst_offs, op.arg2))
                        RetError
                    int32_t darg2;
                    if (!getValue(sym.consts, subarg2, darg2))
                        RetError
                    op.arg2 += darg2;
                }
                break;
            default:
                if (!parseString(arg1, pos))
                    RetError
                if (!parseString(arg2, pos))
                    RetError
                if (pass == 1) {
                    if (!getValue(sym.consts, arg1, op.arg1))
                        RetError
                    if (!getValue(sym.consts, arg2, op.arg2))
                        RetError
                }
                break;
            }
            if (pass == 1) {
                ret_ops.push_back(op);
            }
            inst_offs += InstSize;
        }
    }

    #undef RetError
    return true;
}

void reverseString(std::string& s)
{
    for(uint32_t cnt = static_cast<uint32_t>(s.size()), i = ((cnt + 1) >> 1); i > 0; --i)
        std::swap(s[i - 1], s[cnt - i]);
}

void stripFile(std::string& s, std::string& file)
{
    file.clear();
    while(!s.empty()) {
        const auto ch = s.back();
        if (ch == '\\' || ch == '/')
            break;
        file.push_back(ch);
        s.pop_back();
    }
    reverseString(file);
}

bool readFile(const std::string& path, std::vector<char>& ret)
{
    std::ifstream fp;
    fp.open(path.c_str());
    if (!fp)
        return false;
    fp.seekg(0, fp.end);
    const auto length = fp.tellg();
    fp.seekg(0, fp.beg);
    ret.resize(static_cast<size_t>(length));
    fp.read(ret.data(), length);
    return true;
}

struct LineMap
{
    std::string file;
    uint32_t local_line;
    uint32_t merged_line;
};

void decodeErrorFileAndLine(const std::vector<LineMap>& line_map,
    std::string& error_file, uint32_t& error_line)
{
    for(size_t i = line_map.size(); i > 0; --i) {
        const auto& p = line_map[i - 1];
        if (error_line >= p.merged_line) {
            error_file = p.file;
            error_line -= p.merged_line;
            error_line += p.local_line;
            break;
        }
    }
}

bool readFileWithInclude(std::vector<char>& ret, const char* file_path,
    std::vector<LineMap>& line_map, uint32_t& error_line)
{
    uint32_t local_line = 1;

    std::string file;
    std::string dir = file_path;
    stripFile(dir, file);
    line_map.push_back({file, local_line, error_line});

    std::ifstream fp;
    fp.open(file_path);
    if (!fp) {
        std::cout << "file " << file_path << " not found";
        return false;
    }
    std::string line, cmd, arg;
    while(std::getline(fp, line)) {
        ParsePos pos = {line.data(), static_cast<uint32_t>(line.size()), 0u, 0u};
        if (parseString(cmd, pos) && cmd == "include") {
            if (!parseString(arg, pos))
                return false;
            arg = dir + arg;
            if (!readFileWithInclude(ret, arg.c_str(), line_map, error_line))
                return false;
            ++local_line;
            line_map.push_back({file, local_line, error_line});
            continue;
        }
        for(const auto ch : line)
            ret.push_back(ch);
        ret.push_back('\n');
        ++local_line;
        ++error_line;
    }
    return true;
}

bool readAndCompile(std::vector<Op>& ret_ops, const char* code_file_path,
    std::string& error_file, uint32_t& error_line)
{
    error_line = 1;
    std::vector<LineMap> line_map;
    std::vector<char> code;
    if (!readFileWithInclude(code, code_file_path, line_map, error_line))
        return false;
    Symbols sym;
    initSymbols(sym);
    if (!compile(ret_ops, sym, code.data(), code.size(), error_line)) {
        decodeErrorFileAndLine(line_map, error_file, error_line);
        return false;
    }
    return true;
}

void resetMachine(Machine& m, const std::vector<Op>& ops)
{
    m.data_offset = static_cast<int32_t>(ops.size()) + 100000;
    m.mem_size = m.data_offset + 1000000;
    m.cycles = 0;
    m.max_cycles = 500000000;
    m.last_dbgext_cycles = m.cycles;
    m.inst_addr = m.data_offset;
    m.mem.clear();
    m.mem.resize(static_cast<size_t>(m.mem_size), 0);
    uint32_t ofs = static_cast<uint32_t>(m.data_offset);
    for(const auto& op : ops) {
        ofs -= InstSize;
        m.mem[ofs + 2] = static_cast<int32_t>(op.code);
        m.mem[ofs + 1] = op.arg1;
        m.mem[ofs] = op.arg2;
    }
}

void dumpMachine(const Machine& m, int32_t inst_count, int32_t data_count)
{
    std::unordered_map<int32_t, std::string> opcodes;
    for(const auto& p : opcode_def)
        opcodes[static_cast<int32_t>(p.second)] = p.first;
    std::cout << "------------" << std::endl;
    std::cout << "memory dump:" << std::endl;
    for(int32_t i = std::max(0, m.data_offset - inst_count*InstSize); i < m.data_offset; i += 3) {
        auto it = opcodes.find(m.mem[static_cast<uint32_t>(i + 2)]);
        std::cout << i << " [" << (i - m.data_offset) << "]: " << m.mem[static_cast<uint32_t>(i)] << " "
                  << m.mem[static_cast<uint32_t>(i + 1)] << " "
                  << (it == opcodes.end() ? "invalid" : it->second) << std::endl;

    }
    std::cout << "------------" << std::endl;
    for(int32_t i = m.data_offset, cnt = std::min(i + data_count, m.mem_size); i < cnt; ++i)
        std::cout << i << " [" << (i - m.data_offset) << "]: " << m.mem[static_cast<uint32_t>(i)] << std::endl;
}

const char* getResult(Result res)
{
    switch(res) {
    case Result::Continue: return "continue";
    case Result::Halt: return "halt";
    case Result::InfiniteLoop: return "infinite loop";
    case Result::DivByZero: return "division by zero";
    case Result::InvalidOpCode: return "invalid opcode";
    case Result::InvalidDataAddr: return "invalid data addr";
    case Result::InvalidInstAddr: return "invalid inst addr";
    case Result::InvalidJumpAddr: return "invalid jump addr";
    default:
        return "unknown runtime error";
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "usage: vm <text file with code>" << std::endl;
        return -1;
    }

    std::vector<Op> ops;
    std::string error_file;
    uint32_t error_line;
    if (!readAndCompile(ops, argv[1], error_file, error_line)) {
        std::cout << "error at " << error_file << " line " << error_line << std::endl;
        return -1;
    }
    Machine m;
    resetMachine(m, ops);
    Result res;
    while(true) {
        res = execute(m);
        if (res != Result::Continue)
            break;
    }
    dumpMachine(m, 128, 32);
    std::cout << getResult(res) << std::endl;
    return 0;
}
