#ifndef GREEDY_ALLOCATOR_HPP
#define GREEDY_ALLOCATOR_HPP

#include "RegisterAllocator.hpp"
#include "ir.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <sstream>

class GreedyAllocator : public RegisterAllocator {
public:
    GreedyAllocator();

    void prepare(const FunctionIR& func) override;
    std::string getPrologue() override;
    std::string getEpilogue() override;
    std::string getEpilogueLabel() const override;
    std::string loadOperand(const Operand& op, const std::string& dest_reg) override;
    std::string storeOperand(const Operand& op, const std::string& src_reg) override;
    int getTotalStackSize() const override;

    // 新增：获取当前正在被分配器使用的物理寄存器列表
    std::vector<std::string> getCurrentUsedRegs() const;

private:
    std::string getRegFor(const Operand& op, std::stringstream& ss, bool needs_load = true);
    void spillReg(const std::string& reg_to_spill, std::stringstream& ss);

    const FunctionIR* m_func;

    // key: 虚拟操作数ID -> value: 物理寄存器名称
    std::map<std::string, std::string> m_reg_map;
    // key: 物理寄存器名称 -> value: 虚拟操作数ID
    std::map<std::string, std::string> m_reg_owner;
    // key: 虚拟操作数ID -> value: 栈偏移量 (与fp的偏移)
    std::map<std::string, int> m_spill_map;

    std::vector<std::string> m_physical_regs;
    std::set<std::string> m_free_regs;
    int m_next_spill_offset = 0;
    size_t m_spill_victim_idx = 0;

    // 用于保存参数加载代码
    std::string m_param_init_code;
};

#endif // GREEDY_ALLOCATOR_HPP
