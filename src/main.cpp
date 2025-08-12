#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ast.hpp"
#include "SemanticAnalyzer.hpp"
#include "IRGenerator.hpp"
#include "CodeGenerator.hpp" 
#include "Optimizer.hpp"

// --- 从外部文件链接的全局变量和函数 ---
extern FILE* yyin;
extern Program* g_root;
extern int yyparse(void);
extern int yydebug;
extern int yy_flex_debug;

// --- 辅助函数，用于将 IR 打印到控制台，方便调试 ---

// 将 Operand 结构体转换为可读字符串
std::string operand_to_string(const Operand& op) {
    switch (op.kind) {
    case Operand::VAR:   return op.name;
    case Operand::TEMP:  return "t" + std::to_string(op.id);
    case Operand::CONST: return std::to_string(op.value);
    case Operand::LABEL:
        // 如果标签有名字（如函数入口），用名字；否则用ID
        return op.name.empty() ? ".L" + std::to_string(op.id) : op.name;
    default: return "??";
    }
}

// 打印整个模块的 IR
void print_ir(const ModuleIR& module) {
    for (const auto& func : module.functions) {
        std::cout << "FUNCTION " << func.name << ":" << std::endl;
        for (const auto& block : func.blocks) {
            std::cout << block.label << ":" << std::endl;
            for (const auto& instr : block.instructions) {
                std::cout << "  ";
                switch (instr.opcode) {
                    // --- 算术运算 ---
                case Instruction::ADD:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " + " << operand_to_string(instr.arg2);
                    break;
                case Instruction::SUB:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " - " << operand_to_string(instr.arg2);
                    break;
                case Instruction::MUL:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " * " << operand_to_string(instr.arg2);
                    break;
                case Instruction::DIV:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " / " << operand_to_string(instr.arg2);
                    break;
                case Instruction::MOD:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " % " << operand_to_string(instr.arg2);
                    break;

                    // --- 逻辑与关系运算 ---
                case Instruction::NOT:
                    std::cout << operand_to_string(instr.result) << " = NOT " << operand_to_string(instr.arg1);
                    break;
                case Instruction::EQ:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " == " << operand_to_string(instr.arg2);
                    break;
                case Instruction::NEQ:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " != " << operand_to_string(instr.arg2);
                    break;
                case Instruction::LT:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " < " << operand_to_string(instr.arg2);
                    break;
                case Instruction::GT:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " > " << operand_to_string(instr.arg2);
                    break;
                case Instruction::LE:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " <= " << operand_to_string(instr.arg2);
                    break;
                case Instruction::GE:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " >= " << operand_to_string(instr.arg2);
                    break;

                    // --- 赋值 ---
                case Instruction::ASSIGN:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1);
                    break;

                
                // --- 函数 ---
                case Instruction::PARAM: // <-- 新增
                    std::cout << "PARAM " << operand_to_string(instr.arg1);
                    break;
                case Instruction::CALL:
                    // 现在也打印参数数量
                    std::cout << operand_to_string(instr.result) << " = CALL " << operand_to_string(instr.arg1) << ", " << operand_to_string(instr.arg2);
                    break;
                case Instruction::RET:
                    std::cout << "RET " << (instr.arg1.kind != Operand::NONE ? operand_to_string(instr.arg1) : "");
                    break;

                    // --- 分支与标签 ---
                case Instruction::JUMP:
                    std::cout << "JUMP " << operand_to_string(instr.arg1);
                    break;
                case Instruction::JUMP_IF_ZERO:
                    std::cout << "IF " << operand_to_string(instr.arg1) << " == 0 JUMP " << operand_to_string(instr.arg2);
                    break;
                case Instruction::JUMP_IF_NZERO:
                    std::cout << "IF " << operand_to_string(instr.arg1) << " != 0 JUMP " << operand_to_string(instr.arg2);
                    break;
                case Instruction::LABEL:
                    // 标签本身就是块的名字，通常不需要作为指令打印
                    continue; // 跳过打印
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    

    /* yydebug = 1;
     yy_flex_debug = 1;*/

    int parse_result = yyparse();

    if (parse_result == 0 && g_root != nullptr) {
        //std::cout << "\nParsing successful! AST created." << std::endl;

        SemanticAnalyzer analyzer;
        analyzer.analyze(g_root);
        //std::cout << "Semantic analysis finished." << std::endl;

        //std::cout << "\n--- Starting IR Generation ---" << std::endl;
        IRGenerator ir_gen;
        ModuleIR ir_module = ir_gen.generate(g_root);
        //std::cout << "--- IR Generation Finished ---" << std::endl;
        //std::cout << "--- Starting Optimization ---" << std::endl;
        Optimizer optimizer;
        optimizer.optimize(ir_module);
        //std::cout << "--- Optimization Finished ---\n" << std::endl;

        //std::cout << "\n--- Generated Intermediate Representation ---" << std::endl;
        //print_ir(ir_module);
        CodeGenerator code_gen;
        std::string assembly_code = code_gen.generate(ir_module);

         //--- 【核心修改】将汇编代码输出到标准输出流 ---
        std::cout << assembly_code;

    }
    else {
        std::cout << "\nParsing failed." << std::endl;
    }

    if (yyin) {
        fclose(yyin);
    }

    return parse_result;
}
