extern int M5_inSimulator; /* M5 pseudo instructions disabled by default */
void SimStartup();
void SimRoiStart(int threadid);
void SimRoiEnd(int threadid, int in_fast_forward);
int workBegin(int workid, int threadid);
void workEnd(int workid, int threadid);
long getWorkItemCount();
