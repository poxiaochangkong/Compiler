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

    // ��������ȡ��ǰ���ڱ�������ʹ�õ�����Ĵ����б�
    std::vector<std::string> getCurrentUsedRegs() const;

private:
    std::string getRegFor(const Operand& op, std::stringstream& ss, bool needs_load = true);
    void spillReg(const std::string& reg_to_spill, std::stringstream& ss);

    const FunctionIR* m_func;

    // key: ���������ID -> value: ����Ĵ�������
    std::map<std::string, std::string> m_reg_map;
    // key: ����Ĵ������� -> value: ���������ID
    std::map<std::string, std::string> m_reg_owner;
    // key: ���������ID -> value: ջƫ���� (��fp��ƫ��)
    std::map<std::string, int> m_spill_map;

    std::vector<std::string> m_physical_regs;
    std::set<std::string> m_free_regs;
    int m_next_spill_offset = 0;
    size_t m_spill_victim_idx = 0;

    // ���ڱ���������ش���
    std::string m_param_init_code;
};

#endif // GREEDY_ALLOCATOR_HPP
