#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ast.hpp"
#include "SemanticAnalyzer.hpp"
#include "IRGenerator.hpp" // 引入 IRGenerator

// --- 从外部文件链接的全局变量和函数 ---
extern FILE* yyin;
extern Program* g_root;
extern int yyparse(void);
extern int yydebug;
extern int yy_flex_debug;

// --- 辅助函数，用于将 IR 打印到控制台，方便调试 ---
std::string operand_to_string(const Operand& op) {
    switch (op.kind) {
    case Operand::VAR:   return op.name;
    case Operand::TEMP:  return "t" + std::to_string(op.id);
    case Operand::CONST: return std::to_string(op.value);
    case Operand::LABEL: return op.name.empty() ? ".L" + std::to_string(op.id) : op.name;
    default: return "??";
    }
}

void print_ir(const ModuleIR& module) {
    for (const auto& func : module.functions) {
        std::cout << "FUNCTION " << func.name << ":" << std::endl;
        for (const auto& block : func.blocks) {
            std::cout << block.label << ":" << std::endl;
            for (const auto& instr : block.instructions) {
                std::cout << "  ";
                switch (instr.opcode) {
                case Instruction::ADD:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " + " << operand_to_string(instr.arg2);
                    break;
                case Instruction::SUB:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " - " << operand_to_string(instr.arg2);
                    break;
                case Instruction::MUL:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " * " << operand_to_string(instr.arg2);
                    break;
                case Instruction::ASSIGN:
                    std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1);
                    break;
                case Instruction::RET:
                    std::cout << "RET " << (instr.arg1.kind == Operand::VAR || instr.arg1.kind == Operand::TEMP || instr.arg1.kind == Operand::CONST ? operand_to_string(instr.arg1) : "");
                    break;
                case Instruction::JUMP:
                    std::cout << "JUMP " << operand_to_string(instr.arg1);
                    break;
                case Instruction::JUMP_IF_ZERO:
                    std::cout << "JUMP_IF_ZERO " << operand_to_string(instr.arg1) << ", " << operand_to_string(instr.arg2);
                    break;
                case Instruction::JUMP_IF_NZERO:
                    std::cout << "JUMP_IF_NZERO " << operand_to_string(instr.arg1) << ", " << operand_to_string(instr.arg2);
                    break;
                case Instruction::CALL:
                    std::cout << operand_to_string(instr.result) << " = CALL " << operand_to_string(instr.arg1);
                    break;
                    // ... 在这里添加打印其他指令的逻辑 ...
                default:
                    std::cout << "OpCode(" << instr.opcode << ")";
                    break;
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("Error opening file");
            return 1;
        }
    }

    // yydebug = 1;
    // yy_flex_debug = 1;

    int parse_result = yyparse();

    if (parse_result == 0 && g_root != nullptr) {
        std::cout << "\nParsing successful! AST created." << std::endl;

        // --- 语义分析 ---
        SemanticAnalyzer analyzer;
        analyzer.analyze(g_root);
        std::cout << "Semantic analysis finished." << std::endl;

        // --- 中间代码生成 ---
        std::cout << "\n--- Starting IR Generation ---" << std::endl;
        IRGenerator ir_gen;
        ModuleIR ir_module = ir_gen.generate(g_root);
        std::cout << "--- IR Generation Finished ---" << std::endl;

        // --- 打印生成的 IR 以便调试 ---
        std::cout << "\n--- Generated Intermediate Representation ---" << std::endl;
        print_ir(ir_module);

        // --- (未来的步骤) 目标代码生成 ---
        // CodeGenerator code_gen;
        // std::string assembly_code = code_gen.generate(ir_module);
        // std::ofstream out_file("output.s");
        // out_file << assembly_code;
        // out_file.close();

    }
    else {
        std::cout << "\nParsing failed." << std::endl;
    }

    if (yyin) {
        fclose(yyin);
    }

    return parse_result;
}
