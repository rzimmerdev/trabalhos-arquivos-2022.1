import os
import subprocess
import sys

# Roda dentro do diretório bin/,
# sugerível deixar arquivos nesse diretório, mas dá pra alterar o código pra poder rodar em outro lugar


def test_cases(trabalho, idx=None):
    input_dir = f"cases{trabalho}/in"
    output_dir = f"cases{trabalho}/out"

    if not idx:
        starting_file = 1
        total_files = len(os.listdir(f"{input_dir}"))
    else:
        starting_file = idx[0]
        total_files = idx[1]

    output = "saida.txt"
    comparison = "comparison.txt"

    subprocess.run([f"make all"], shell=True)

    subprocess.run(["rm bin/*"], shell=True)
    subprocess.run([f"cp files{trabalho}/* bin/"], shell=True)

    os.chdir("bin")
    for i in range(starting_file, total_files + 1):

        subprocess.run([f"../main < ../{input_dir}/{i}.in > {output}"], shell=True)
        subprocess.run([f"diff -w saida.txt ../{output_dir}/{i}.out > {comparison}", "-w"], shell=True)

        to_print = open("comparison.txt").read()
        case_input = open(f"../{input_dir}/{i}.in").read()
        if to_print:
            print(f"Caso {i}: Erro.")
            print(f"Entrada: {case_input}")
            print(to_print, end="")
        else:
            print(f"Caso {i}: Ok.")


def main():
    if len(sys.argv) > 3:
        test_cases(int(sys.argv[1]), (int(sys.argv[2]), int(sys.argv[3])))
    elif len(sys.argv) > 2:
        test_cases(int(sys.argv[1]), (int(sys.argv[2]), int(sys.argv[2])))


if __name__ == "__main__":
    main()
