#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>

using namespace std;

vector<string> separar_pipes(const string& input, int& cantidad_pipes) {
    vector<string> tokens;
    cantidad_pipes = 0;
    stringstream ss(input);
    string token;
    
    while (getline(ss, token, '|')) { 
        tokens.push_back(token);       
        cantidad_pipes++;                  
    }
    
    cantidad_pipes--; 
    return tokens;
}

vector<string> separar(const string& input) {
    stringstream ss(input);
    string token;
    vector<string> tokens;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}


int main() {
    vector<vector<string>> comando_completo;
    string input_usuario;
    int cantidad_pipes = 0;
    
    while(true){
        cout << "$ ";
        getline(cin, input_usuario);
        vector<string> comandos = separar_pipes(input_usuario, cantidad_pipes);

        int pipes[cantidad_pipes][2];
        int cantidad_comandos = cantidad_pipes + 1;
        
        for(int i = 0; i < cantidad_pipes; i++){
            pipe(pipes[i]);
        }

        for (const auto& cmd : comandos) {
            vector<string> args = separar(cmd);
            comando_completo.push_back(args);    
        }

        //for (const auto& cmd : comandos) {
        //    cout << "Comando: " << cmd << endl;
        //}

        if (comandos.empty()) {
            continue;
        }

        if (!comandos.empty() && comandos[0] == "exit") {
            break;
        }

        if (comando_completo[0][0] == "cd") {
            if (comando_completo[0].size() < 2) {
                chdir(getenv("HOME"));
            } else {
                if (chdir(comando_completo[0][1].c_str()) != 0) {
                    perror("cd failed");
                }
            }
            comando_completo.clear();
            continue;
        }


        for(int i = 0; i < cantidad_comandos; i++) {
            if(fork() == 0) {
                vector<char*> exec_args;
                for (auto& arg : comando_completo[i]) {
                    exec_args.push_back(&arg[0]);
                }
                exec_args.push_back(nullptr);


                //Si no es el primer comando, lee del comando anterior
                if (i > 0) {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                
                // Si no es el último comando, escribe al siguiente comando
                if (i < cantidad_pipes) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                
                // Cierra las pipes
                for (int j = 0; j < cantidad_pipes; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                if (execvp(exec_args[0], exec_args.data()) == -1) {
                    perror("Commando no encontrado");
                    exit(1);
                }
                exit(1);
            }
        }

        for (int j = 0; j < cantidad_pipes; j++) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
        for (int i = 0; i < cantidad_comandos; i++) {
            wait(nullptr);
        }
        comando_completo.clear();
    }
}