#pragma once

// Utility and initialization functions
void usage(const char* prog);
int16_t select_port(int argc, char **argv);
void on_sigint(int /*signum*/);
std::string init_pid_file();
void add_sigint_handler();

// Global variables
extern int server_socket;
extern std::string pid_path;

// Add these to the bottom of utils.hh
std::string extract_filename(const std::string& body);
std::string extract_content(const std::string& body);
