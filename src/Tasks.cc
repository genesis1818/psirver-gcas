#include "Tasks.hh"
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Do the job, reply to the client, and return to the main loop

int HealthTask::execute()
{
  const std::string response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 7\r\n"
      "\r\n"
      "Running";

  send(client, response.c_str(), response.size(), 0);
  return 0;
}
 
int TeapotTask::execute()
{
  const std::string response =
      "HTTP/1.1 418 I'm a Teapot\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 7\r\n"
      "\r\n"
      "Running";

  send(client, response.c_str(), response.size(), 0);
  return 0;
}

int StderrTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will report stderr of job " << job_id << "\n";
  return 0;
}

int DeleteTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will delete script " << script_id << "\n";
  return 0;
}

int RunTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will run script " << script_id << "\n";
  return 0;
}

int JobStatusTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will report status of job " << job_id << "\n";
  return 0;
};

int TerminateTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will terminate job " << job_id << "\n";
  return 0;
}

int StdoutTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will report stdout of job " << job_id << "\n";
  return 0;
} 

int UploadTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will upload file " << filename << "; script length " << script.size() << "\n";
  return 0;
}

int ScriptListTask::execute()
{
  DIR *dir = opendir("scripts");
  if (!dir) {
    const std::string response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 0\r\n"
      "\r\n";
    send(client, response.c_str(), response.size(), 0);
    return 0;
  }

  std::vector<std::string> lines;

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {

    if (entry->d_name[0] == '.') continue;

    std::string id = entry->d_name;
    std::string folder = std::string("scripts/") + id;

    DIR *script_dir = opendir(folder.c_str());
    if (!script_dir) continue;

    struct dirent *file_entry;
    while ((file_entry = readdir(script_dir)) != nullptr) {

      if (file_entry->d_name[0] == '.') continue;

      std::string filename = file_entry->d_name;
      std::string path = folder + "/" + filename;

      struct stat st;
      stat(path.c_str(), &st);

      char timebuf[64];
      strftime(timebuf, sizeof(timebuf),
               "%m/%d/%Y %H:%M:%S",
               localtime(&st.st_mtime));

      std::stringstream line;
      line << id << "," << filename << "," << timebuf;

      lines.push_back(line.str());
    }

    closedir(script_dir);
  }

  closedir(dir);

  std::sort(lines.begin(), lines.end());

  std::stringstream body;
  for (auto &l : lines) {
    body << l << "\n";
  }

  std::string body_str = body.str();

  std::stringstream response;
  response << "HTTP/1.1 200 OK\r\n";
  response << "Content-Type: text/plain\r\n";
  response << "Content-Length: " << body_str.size() << "\r\n\r\n";
  response << body_str;

  std::string resp = response.str();
  send(client, resp.c_str(), resp.size(), 0);

  return 0;
}
int JobListTask::execute()
{
  // --> To be implemented later
  std::cerr << "I will report list of jobs\n";
  return 0;
}
