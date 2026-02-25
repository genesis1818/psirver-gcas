#pragma once
#include <string>

// Declarations so other files can use these functions
std::string extract_filename(const std::string& body);
std::string extract_content(const std::string& body);

// Your partner's existing declarations (if any)
void usage(const char* prog);
int16_t select_port(int argc, char **argv);
void add_sigint_handler();
std::string init_pid_file();
void on_sigint(int signum);
