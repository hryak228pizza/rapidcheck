    import os
    import shutil
    import subprocess
    import re

    SOURCE_FILE = "main.cpp"
    MUTANTS_DIR = "mutants"
    BINARY_NAME = "mutant_exec"

    # мутации
    MUTATIONS = [
        ("&&", "||", "Logical AND -> OR"),
        ("||", "&&", "Logical OR -> AND"),
        ("==", "!=", "Equality -> Inequality"),
        ("!=", "==", "Inequality -> Equality"),
        (">", "<", "Greater -> Less"),
        ("<", ">", "Less -> Greater"),
        ("<=", ">=", "LessEqual -> GreaterEqual"),
        (">=", "<=", "GreaterEqual -> LessEqual"),
        ("+", "-", "Addition -> Subtraction"),
        ("-", "+", "Subtraction -> Addition"),
        ("*", "/", "Multiplication -> Division"),
        ("/", "*", "Division -> Multiplication"),
        ("head && head->next", "head", "Removed head->next check"),
        ("iter->next = customSwap", "//iter->next = customSwap", "Removed swap call"),
    ]

    def apply_mutation(src: str, original: str, mutated: str) -> str:
        return src.replace(original, mutated, 1)

    def run_test(file_path: str) -> bool:
        try:
            subprocess.check_call(
                ["g++", file_path, "-std=c++17", "-o", BINARY_NAME],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            result = subprocess.run(f"./{BINARY_NAME}", capture_output=True, text=True, timeout=5)
            return result.returncode == 0
        except Exception as e:
            return False
        finally:
            if os.path.exists(BINARY_NAME):
                os.remove(BINARY_NAME)

    def main():
        if not os.path.exists(MUTANTS_DIR):
            os.mkdir(MUTANTS_DIR)

        with open(SOURCE_FILE, "r", encoding="utf-8") as f:
            original_code = f.read()

        mutant_count = 0
        for i, (target, mutation, comment) in enumerate(MUTATIONS):
            if target not in original_code:
                continue

            mutated_code = apply_mutation(original_code, target, mutation)
            mutant_file = os.path.join(MUTANTS_DIR, f"mutant_{i}.cpp")
            with open(mutant_file, "w", encoding="utf-8") as f:
                f.write(f"// Mutation: {comment}\n")
                f.write(mutated_code)

            print(f"[+] Testing mutant {i}: {comment}")
            killed = not run_test(mutant_file)
            print(f"    {'[KILLED]' if killed else '[SURVIVED]'}")

            mutant_count += 1

        print(f"\nTotal mutants tested: {mutant_count}")

    if __name__ == "__main__":
        main()
