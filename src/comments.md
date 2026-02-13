- Line 20: openlog("psirver", LOG\_PID | LOG_CONS, LOG_USER);
  * This is extremely inefficient. If you want to use _another_log, open it opene. 
- Line 290: std::stoi is more robust
- Line 237: std::cerr << "Error: PSIRVER_HOME is not set.\n" and the like
  * Use perror or strerror(errno) to report specific errors
- Line 250: ..., 0644);
  * Use predefind constants: S\_IWUSR | S\_IRUSR | S\_IRGRP | S\_IROTH
_ Line 277: Must exit here. 
