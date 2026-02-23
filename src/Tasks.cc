#include "Tasks.hh"
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>
#include <vector>

struct ScriptEntry {
  int id;
  std::string name;
  std::string contents;
};

enum class JobStatus { running, finished, failed, unknown, timed_out, output_limited };

static const char* status_to_cstr(JobStatus s) {
  switch (s) {
    case JobStatus::running:        return "running";
    case JobStatus::finished:       return "finished";
    case JobStatus::failed:         return "failed";
    case JobStatus::unknown:        return "unknown";
    case JobStatus::timed_out:      return "timed_out";
    case JobStatus::output_limited: return "output_limited";
  }
  return "unknown";
}

struct JobEntry {
  int id;
  int script_id;
  std::string script_name;
  JobStatus status;
  std::string out;
  std::string err;
};

static std::map<int, ScriptEntry> g_scripts;
static std::map<int, JobEntry> g_jobs;
static int g_next_script_id = 1;
static int g_next_job_id = 1;

int HealthTask::execute()
{
  reply(client, "HTTP/1.1 200 OK", "OK\n");
  return 0;
}
 
int TeapotTask::execute()
{
  reply(client, "HTTP/1.1 418 I'm a teapot", "I'm a teapot\n");
  return 0;
}

int StderrTask::execute()
{
  auto it = g_jobs.find(job_id);
  if (it == g_jobs.end()) {
    reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
    return 0;
  }

  const JobEntry& j = it->second;
  if (j.status == JobStatus::running) {
    reply(client, "HTTP/1.1 202 Accepted", "");
    return 0;
  }

  reply(client, "HTTP/1.1 200 OK", j.err.c_str());
  return 0;
}

int DeleteTask::execute()
{
  auto it = g_scripts.find(script_id);
  if (it == g_scripts.end()) {
    reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
    return 0;
  }

  g_scripts.erase(it);
  reply(client, "HTTP/1.1 200 OK", "OK\n");
  return 0;
}

int RunTask::execute()
{
  // --> Implement later
  return 0;
}

int JobStatusTask::execute()
{
  auto it = g_jobs.find(job_id);
  if (it == g_jobs.end()) {
    reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
    return 0;
  }

  const JobEntry& j = it->second;
  std::string body = std::to_string(j.script_id) + "," + j.script_name + "," + status_to_cstr(j.status) + "\n";
  reply(client, "HTTP/1.1 200 OK", body.c_str());
  return 0;
};

int TerminateTask::execute()
{
  auto it = g_jobs.find(job_id);
  if (it == g_jobs.end()) {
    reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
    return 0;
  }

  // For now, just remove the job. Your partner can later actually kill processes.
  g_jobs.erase(it);
  reply(client, "HTTP/1.1 200 OK", "OK\n");
  return 0;
}

int StdoutTask::execute()
{
  auto it = g_jobs.find(job_id);
  if (it == g_jobs.end()) {
    reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
    return 0;
  }

  const JobEntry& j = it->second;
  if (j.status == JobStatus::running) {
    reply(client, "HTTP/1.1 202 Accepted", "");
    return 0;
  }

  reply(client, "HTTP/1.1 200 OK", j.out.c_str());
  return 0;
} 

int UploadTask::execute()
{
  // --> Implement later
  return 0;
}

int ScriptListTask::execute()
{
  std::string body;
  for (const auto& [id, s] : g_scripts) {
    body += std::to_string(s.id) + "," + s.name + "\n";
  }
  reply(client, "HTTP/1.1 200 OK", body.c_str());
  return 0;
}

int JobListTask::execute()
{
  std::string body;
  for (const auto& [id, j] : g_jobs) {
    body += std::to_string(j.id) + "," + std::to_string(j.script_id) + "," + j.script_name + "\n";
  }
  reply(client, "HTTP/1.1 200 OK", body.c_str());
  return 0;
}

// This function parses the headers and returns one of the GET task
// objects
Task *Task::construct(int client, std::string headers)
{
  std::string request_line;

  auto pos = headers.find("\r\n");
  if (pos != std::string::npos)
    request_line = headers.substr(0, pos);
  else
    request_line = headers;

  std::stringstream ss(request_line);
  std::string method, path, version;

  if (!(ss >> method >> path >> version)) {
    reply(client, "HTTP/1.1 400 Bad Request", "400 Bad Request\n");
    return nullptr;
  }

  std::vector<std::string> parts;
  std::stringstream ps(path);
  std::string item;

  while (std::getline(ps, item, '/')) {
    if (!item.empty())
      parts.push_back(item);
  }

  if (parts.size() == 1 && parts[0] == "health")
    return new HealthTask(client);

  if (parts.size() == 1 && parts[0] == "teapot")
    return new TeapotTask(client);

  if (parts.size() == 1 && parts[0] == "jobs")
    return new JobListTask(client);

  if (parts.size() == 1 && parts[0] == "scripts")
    return new ScriptListTask(client);

  if (parts.size() == 2 && parts[0] == "jobs") {
    if (parts[1].find_first_not_of("0123456789") != std::string::npos) {
      reply(client, "HTTP/1.1 400 Bad Request", "400 Bad Request\n");
      return nullptr;
    }
    return new JobStatusTask(client, std::stoi(parts[1]));
  }

  if (parts.size() == 3 && parts[0] == "jobs") {
    if (parts[1].find_first_not_of("0123456789") != std::string::npos) {
      reply(client, "HTTP/1.1 400 Bad Request", "400 Bad Request\n");
      return nullptr;
    }

    int id = std::stoi(parts[1]);

    if (parts[2] == "terminate")
      return new TerminateTask(client, id);

    if (parts[2] == "stdout")
      return new StdoutTask(client, id);

    if (parts[2] == "stderr")
      return new StderrTask(client, id);

    reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
    return nullptr;
  }

  if (parts.size() == 3 && parts[0] == "scripts" && parts[2] == "delete") {
    if (parts[1].find_first_not_of("0123456789") != std::string::npos) {
      reply(client, "HTTP/1.1 400 Bad Request", "400 Bad Request\n");
      return nullptr;
    }

    return new DeleteTask(client, std::stoi(parts[1]));
  }

  reply(client, "HTTP/1.1 404 Not Found", "404 Not Found\n");
  return nullptr;
}

// This function parses the headers and the body returns one of the
// POST task objects
Task *Task::construct(int client, std::string headers, std::string body)
{
  // Content-Type: application/x-www-form-urlencoded
  // Body:
  // ......data.........

  // or (note that the boundary is the same in all three places):

  // Content-Type: multipart/form-data; boundary=------------------------67c1112af97a18b9
  // Body: 
  // --------------------------67c1112af97a18b9
  // Content-Disposition: form-data; name="file"; filename="Makefile"
  // Content-Type: application/octet-stream
  // ......data.........
  // --------------------------67c1112af97a18b9--

  // --> Implement & remove debug printout
  std::cerr << "POST" << std::endl; // DEBUG
  std::cerr << "Headers:\n" << headers << std::endl; // DEBUG
  std::cerr << "Body:\n" << body << std::endl; // DEBUG
  
  // Return a new Task
  return nullptr;
}

