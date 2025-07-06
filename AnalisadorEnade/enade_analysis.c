// Análise ENADE 2021 - Versão Corrigida - Processamento agregado por curso
// Compilar: mpicc enade_analysis_fixed.c -o enade_analysis_fixed
// Executar: mpirun -np 4 ./enade_analysis_fixed

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINE_LENGTH 1024
#define MAX_CURSOS 1000
#define NUM_PROCESSOS 4
#define GRUPO_FILTRO 4006  // Sistemas de Informação

// Estrutura para armazenar dados agregados por curso
typedef struct {
    int ano;
    int codigo_curso;
    int co_grupo;
    int total_estudantes;
    int estudantes_feminino;
    int estudantes_ensino_tecnico;
    int estudantes_acoes_afirmativas;
    int incentivo_ninguem;          // Opção A
    int incentivo_pais;             // Opção B
    int incentivo_outros_familiares; // Opção C
    int incentivo_professores;      // Opção D
    int incentivo_lider_religioso;  // Opção E
    int incentivo_colegas_amigos;   // Opção F
    int incentivo_outras_pessoas;   // Opção G
    int estudantes_familia_superior;
    int total_livros_lidos;
    int total_horas_estudo;
} CursoDados;

// Busca curso no vetor pelo código
int buscar_curso(CursoDados* cursos, int total_cursos, int codigo_curso) {
    for (int i = 0; i < total_cursos; i++) {
        if (cursos[i].codigo_curso == codigo_curso) {
            return i;
        }
    }
    return -1;
}

// Lê arquivo 1 e identifica cursos do grupo especificado
int ler_cursos_grupo(const char* filename, CursoDados* cursos, int grupo_filtro) {
    FILE* arquivo = fopen(filename, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo: %s\n", filename);
        return 0;
    }

    char linha[MAX_LINE_LENGTH];
    int total_cursos = 0;
    
    // Pula cabeçalho
    if (!fgets(linha, sizeof(linha), arquivo)) {
        fclose(arquivo);
        return 0;
    }

    while (fgets(linha, sizeof(linha), arquivo) && total_cursos < MAX_CURSOS) {
        char* token;
        int ano, curso, grupo;

        // Lê ano
        token = strtok(linha, ";");
        if (!token) continue;
        ano = atoi(token);

        // Lê código do curso
        token = strtok(NULL, ";");
        if (!token) continue;
        curso = atoi(token);

        // Pula CO_IES, CO_CATEGAD, CO_ORGACAD
        for (int i = 0; i < 3; i++) {
            token = strtok(NULL, ";");
            if (!token) break;
        }

        // Lê CO_GRUPO
        token = strtok(NULL, ";\n\r");
        if (!token) continue;
        
        if (token[0] == '"' && strlen(token) > 1) {
            char temp[16];
            strcpy(temp, token + 1);
            if (temp[strlen(temp)-1] == '"') {
                temp[strlen(temp)-1] = '\0';
            }
            grupo = atoi(temp);
        } else {
            grupo = atoi(token);
        }

        // Apenas cursos do grupo especificado
        if (grupo == grupo_filtro) {
            // Verifica se curso já existe
            int pos = buscar_curso(cursos, total_cursos, curso);
            if (pos == -1) {
                // Novo curso
                cursos[total_cursos].ano = ano;
                cursos[total_cursos].codigo_curso = curso;
                cursos[total_cursos].co_grupo = grupo;
                cursos[total_cursos].total_estudantes = 0;
                cursos[total_cursos].estudantes_feminino = 0;
                cursos[total_cursos].estudantes_ensino_tecnico = 0;
                cursos[total_cursos].estudantes_acoes_afirmativas = 0;
                cursos[total_cursos].incentivo_ninguem = 0;
                cursos[total_cursos].incentivo_pais = 0; 
                cursos[total_cursos].incentivo_outros_familiares = 0;
                cursos[total_cursos].incentivo_professores = 0;
                cursos[total_cursos].incentivo_lider_religioso = 0;
                cursos[total_cursos].incentivo_colegas_amigos = 0;
                cursos[total_cursos].incentivo_outras_pessoas = 0;
                cursos[total_cursos].estudantes_familia_superior = 0;
                cursos[total_cursos].total_livros_lidos = 0;
                cursos[total_cursos].total_horas_estudo = 0;
                total_cursos++;
            }
        }
    }
    
    fclose(arquivo);
    return total_cursos;
}

// Conta estudantes por curso em um arquivo
int contar_estudantes_por_curso(const char* filename, CursoDados* cursos, int total_cursos) {
    FILE* arquivo = fopen(filename, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo: %s\n", filename);
        return 0;
    }

    char linha[MAX_LINE_LENGTH];
    
    // Pula cabeçalho
    if (!fgets(linha, sizeof(linha), arquivo)) {
        fclose(arquivo);
        return 0;
    }

    while (fgets(linha, sizeof(linha), arquivo)) {
        char* token;
        int curso;

        // Pula ano
        token = strtok(linha, ";");
        if (!token) continue;

        // Lê código do curso
        token = strtok(NULL, ";");
        if (!token) continue;
        curso = atoi(token);

        // Busca curso nos dados
        int pos = buscar_curso(cursos, total_cursos, curso);
        if (pos != -1) {
            cursos[pos].total_estudantes++;
        }
    }
    
    fclose(arquivo);
    return 1;
}

// Processa arquivo de dados específicos
int processar_arquivo_dados(const char* filename, CursoDados* cursos, int total_cursos, int tipo_dados) {
    FILE* arquivo = fopen(filename, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo: %s\n", filename);
        return 0;
    }

    char linha[MAX_LINE_LENGTH];
    
    // Pula cabeçalho
    if (!fgets(linha, sizeof(linha), arquivo)) {
        fclose(arquivo);
        return 0;
    }

    while (fgets(linha, sizeof(linha), arquivo)) {
        char* token;
        int curso;
        char valor[16] = {0};

        // Pula ano
        token = strtok(linha, ";");
        if (!token) continue;

        // Lê código do curso
        token = strtok(NULL, ";");
        if (!token) continue;
        curso = atoi(token);

        // Lê valor
        token = strtok(NULL, ";\n\r");
        if (!token) continue;
        
        if (token[0] == '"' && strlen(token) > 1) {
            strcpy(valor, token + 1);
            if (valor[strlen(valor)-1] == '"') {
                valor[strlen(valor)-1] = '\0';
            }
        } else {
            strcpy(valor, token);
        }

        // Busca curso nos dados
        int pos = buscar_curso(cursos, total_cursos, curso);
        if (pos == -1) continue;

        // Processa dados baseado no tipo
        switch (tipo_dados) {
            case 5:  // Sexo
                if (valor[0] == 'F') {
                    cursos[pos].estudantes_feminino++;
                }
                break;
            case 23: // Tipo de ensino médio
                if (valor[0] == 'B') { // Profissionalizante técnico
                    cursos[pos].estudantes_ensino_tecnico++;
                }
                break;
            case 21: // Ações afirmativas
                if (valor[0] >= 'B' && valor[0] <= 'F') {
                    cursos[pos].estudantes_acoes_afirmativas++;
                }
                break;
           case 25: // Incentivo para graduação (Q025)
                if (strlen(valor) > 0 && valor[0] != ' ' && valor[0] != '\n' && valor[0] != '\r') {
                switch (valor[0]) {
                   case 'A': // Ninguém
                   cursos[pos].incentivo_ninguem++;
                break;
                   case 'B': // Meus pais
                cursos[pos].incentivo_pais++;
                break;
                   case 'C': // Outros familiares
                cursos[pos].incentivo_outros_familiares++;
                break;
                   case 'D': // Professores do ensino médio
                cursos[pos].incentivo_professores++;
                break;
                   case 'E': // Líder religioso
                cursos[pos].incentivo_lider_religioso++;
                break;
                   case 'F': // Colegas/Amigos
                cursos[pos].incentivo_colegas_amigos++;
                break;
                   case 'G': // Outras pessoas
                cursos[pos].incentivo_outras_pessoas++;
                break;
                default:
                break;
        }
    }
                break;
            case 27: // Família com superior
                if (valor[0] == 'A') { // Sim
                    cursos[pos].estudantes_familia_superior++;
                }
                break;
            case 28: // Livros lidos
                if (valor[0] >= 'A' && valor[0] <= 'E') {
                    cursos[pos].total_livros_lidos += (valor[0] - 'A');
                }
                break;
            case 29: // Horas de estudo
                if (valor[0] >= 'A' && valor[0] <= 'E') {
                    cursos[pos].total_horas_estudo += (valor[0] - 'A');
                }
                break;
        }
    }
    
    fclose(arquivo);
    return 1;
}

// Processa uma fatia dos cursos
void processar_fatia_cursos(CursoDados* cursos, int inicio, int fim, int* resultados) {
    for (int i = inicio; i < fim; i++) {
        resultados[0] += cursos[i].total_estudantes;
        resultados[1] += cursos[i].estudantes_feminino;
        resultados[2] += cursos[i].estudantes_ensino_tecnico;
        resultados[3] += cursos[i].estudantes_acoes_afirmativas;
        resultados[4] += cursos[i].incentivo_ninguem;
        resultados[5] += cursos[i].incentivo_pais;
        resultados[6] += cursos[i].incentivo_outros_familiares;
        resultados[7] += cursos[i].incentivo_professores;
        resultados[8] += cursos[i].incentivo_lider_religioso;
        resultados[9] += cursos[i].incentivo_colegas_amigos;
        resultados[10] += cursos[i].incentivo_outras_pessoas;
        resultados[11] += cursos[i].estudantes_familia_superior; 
        resultados[12] += cursos[i].total_livros_lidos;        
        resultados[13] += cursos[i].total_horas_estudo;        
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != NUM_PROCESSOS) {
        if (rank == 0) {
            printf("Execute com exatamente %d processos.\n", NUM_PROCESSOS);
        }
        MPI_Finalize();
        return 1;
    }

    CursoDados* cursos = (CursoDados*)calloc(MAX_CURSOS, sizeof(CursoDados));
    int total_cursos = 0;

    // Processo 0 lê e processa todos os dados
    if (rank == 0) {
        printf("Iniciando análise ENADE 2021...\n");
        printf("Filtrando por CO_GRUPO: %d (Sistemas de Informação)\n", GRUPO_FILTRO);
        
        // Identifica cursos do grupo
        total_cursos = ler_cursos_grupo("DADOS/microdados2021_arq1.txt", cursos, GRUPO_FILTRO);
        if (total_cursos == 0) {
            printf("Nenhum curso encontrado para o grupo %d\n", GRUPO_FILTRO);
            MPI_Finalize();
            return 1;
        }
        
        printf("Encontrados %d cursos do grupo %d\n", total_cursos, GRUPO_FILTRO);
        
        // Conta total de estudantes por curso
        if (!contar_estudantes_por_curso("DADOS/microdados2021_arq5.txt", cursos, total_cursos)) {
            printf("Falha ao contar estudantes\n");
        }
        
        // Processa cada tipo de dados
        printf("Processando dados de sexo...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq5.txt", cursos, total_cursos, 5);
        
        printf("Processando dados de ensino médio...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq23.txt", cursos, total_cursos, 23);
        
        printf("Processando dados de ações afirmativas...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq21.txt", cursos, total_cursos, 21);
        
        printf("Processando dados de incentivo...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq25.txt", cursos, total_cursos, 25);
        
        printf("Processando dados de família com superior...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq27.txt", cursos, total_cursos, 27);
        
        printf("Processando dados de livros lidos...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq28.txt", cursos, total_cursos, 28);
        
        printf("Processando dados de horas de estudo...\n");
        processar_arquivo_dados("DADOS/microdados2021_arq29.txt", cursos, total_cursos, 29);
        
        printf("Processamento de dados concluído.\n");
    }

    // Distribui dados para todos os processos
    MPI_Bcast(&total_cursos, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(cursos, total_cursos * sizeof(CursoDados), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Cada processo trabalha com uma fatia dos cursos
    int inicio = (rank * total_cursos) / size;
    int fim = ((rank + 1) * total_cursos) / size;

    // Processa dados localmente
    int resultados_locais[14] = {0}; 
    processar_fatia_cursos(cursos, inicio, fim, resultados_locais);

    // Combina resultados de todos os processos
    int resultados_globais[14] = {0}; 
    MPI_Reduce(resultados_locais, resultados_globais, 14, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); 

    // Processo 0 apresenta resultados
    if (rank == 0) {
        printf("\n=== RESULTADOS ENADE 2021 - GRUPO %d ===\n", GRUPO_FILTRO);
        printf("\n1. Quantos alunos se matricularam no curso: %d\n", resultados_globais[0]);
        
        if (resultados_globais[0] > 0) {
            printf("\n2. Porcentagem de estudantes do sexo Feminino que se formaram: %.2f%%\n", 
                   100.0 * resultados_globais[1] / resultados_globais[0]);
            
            printf("\n3. Porcentagem de estudantes que cursaram o ensino técnico no ensino médio: %.2f%%\n", 
                   100.0 * resultados_globais[2] / resultados_globais[0]);
            
            printf("\n4. Percentual de alunos provenientes de ações afirmativas: %.2f%%\n", 
                   100.0 * resultados_globais[3] / resultados_globais[0]);
            
             printf("\nDos estudantes, quem deu incentivo para este estudante cursar o ADS: \n");
             printf("A. Ninguém: %d\n", resultados_globais[4]); 
             printf("B. Meus pais: %d\n", resultados_globais[5]); 
             printf("C. Outros familiares: %d\n", resultados_globais[6]); 
             printf("D. Professores do ensino médio: %d\n", resultados_globais[7]); 
             printf("E. Líder religioso: %d\n", resultados_globais[8]); 
             printf("F. Colegas/Amigos: %d\n", resultados_globais[9]); 
             printf("G. Outras pessoas: %d\n", resultados_globais[10]); 
            
            printf("\n6. Estudantes que apresentaram familiares com o curso superior concluído: %d\n", resultados_globais[11]);
            
            printf("\n7. Total de livros que os alunos leram no ano do ENADE: %d livros.\n", resultados_globais[12]);
            
           printf("\n8. Total de horas na semana em que os estudantes se dedicaram aos estudos: %d horas.\n ", resultados_globais[13]);
        }
        
        printf("\nProcessamento concluído com %d processos MPI.\n", size);
    }

    free(cursos);
    MPI_Finalize();
    return 0;
}