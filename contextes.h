
struct context_common_s {
};

struct context_common_s context_common;

// ATTN context
struct context_attn_s {
  int  ready;
  context_attn_s() {
     ready = 0;
  }
};

struct context_attn_s context_attn;

struct contextBullys {
    int coordinatorId;
    int timeStamp;
    bool processing;
    bool aliveRecieved;
    contextBullys() {
        coordinatorId = -1;
        timeStamp = -1;
        processing = false;
        aliveRecieved = false;
    }
};

struct contextBullys contextBully;
