#include "DSSimul.h"
#include <sstream>
////At the beginning tried to do with switch(str) but it didnt work well and i gave up
//#include "str_switch.h"
int timeout = 12;
void informAll(Process *process){
    for(auto neib: process->neibs()){
        process->networkLayer->send(process->node, neib, Message("Coordinator"));
    }
}
void informContenders(Process *process){
    set<int> neibs = process->neibs();
    for (auto neib = neibs.upper_bound(process->node); neib != neibs.end(); ++neib) {
        process->networkLayer->send(process->node, *neib, Message("Elections"));
    }
}
//Implementing Bully Election Algorithm
int workFunction_BULLY(Process *process, Message message){
    NetworkLayer *networkLayer = process->networkLayer;
    set<int> neibs = process->neibs();
    string textInput = message.getString();
    if (textInput == "Elections"){
        printf("#%d: Elections message received from %d\n", process->node, message.from);
        process->contextBully.processing = true;
        auto start = neibs.upper_bound(process->node);
        if (start == neibs.end()) {
            //Easy win
            for(auto neib: process->neibs()){
                networkLayer->send(process->node, neib, Message("Coordinator"));
            }
            process->contextBully.processing = false;
        } else {
            //informing contenders
            for (auto neib = neibs.upper_bound(process->node); neib != neibs.end(); ++neib) {
                networkLayer->send(process->node, *neib, Message("Elections"));
            }
            if (message.from == -1)
                return true;
            networkLayer->send(process->node, message.from, Message("Alive"));
        }
    } else if (textInput == "Alive"){
        printf("#%d: Alive message received from %d\n", process->node, message.from);
        //logging whether somebody talked to current process or not
        process->contextBully.aliveRecieved = true;
    } else if (textInput == "Coordinator"){
        //Taking the Coordinator role
        printf("#%d: Coordinator message received from %d\n", process->node, message.from);
        if (message.from < process->node){
            for(auto neib: process->neibs()){
                networkLayer->send(process->node, neib, Message("Coordinator"));
            }
            process->contextBully.coordinatorId = process->node;
        } else {
            process->contextBully.coordinatorId = message.from;
        }
        printf("#%d: Coordinator is %d\n", process->node, process->contextBully.coordinatorId);
        process->contextBully.aliveRecieved = false;
        process->contextBully.timeStamp = -1;
        process->contextBully.processing = false;
    } else if (textInput == "*TIME") {
        //Checking timeout
        int time = message.getInt();
        if (process->contextBully.processing) {
            if (process->contextBully.timeStamp != -1)
                process->contextBully.timeStamp = time;
            //If nobody talked to current process he takes Coordinator role
            if (!process->contextBully.aliveRecieved && (time > process->contextBully.timeStamp + timeout)){
                for(auto neib: process->neibs()){
                    networkLayer->send(process->node, neib, Message("Coordinator"));
                }
                process->contextBully.coordinatorId = process->node;
                process->contextBully.processing = false;
                process->contextBully.timeStamp = -1;
                printf("#%d: Everybody is silent so it's my turn to be Coordinator\n", process->node);
                //If somebody talked to process he restarts elections
            } else if (process->contextBully.aliveRecieved && (time > process->contextBully.timeStamp + timeout)) {
                for (auto neib = neibs.upper_bound(process->node); neib != neibs.end(); ++neib) {
                    networkLayer->send(process->node, *neib, Message("Elections"));
                }                printf("#%d: Restarting elections\n", process->node);
                process->contextBully.timeStamp = -1;
            }
        }
    }
    return true;
}

int workFunction_TEST(Process *dp, Message m)
{
    string s = m.getString();
    NetworkLayer *nl = dp->networkLayer;
    if (!dp->isMyMessage("TEST", s)) return false;
    set<int> neibs = dp->neibs();
    if (s == "TEST_HELLO") {
        int val = m.getInt();
        printf("TEST[%d]: HELLO %d message received from %d\n", dp->node, val, m.from);
        // Рассылаем сообщение соседям
        if (val < 2) {
            for (auto n: neibs) {
                nl->send(dp->node, n, Message("TEST_HELLO", val+1));
            }
        } else {
            for (auto n: neibs) {
                nl->send(dp->node, n, Message("TEST_BYE"));
            }
        }
    } else if (s == "TEST_BYE") {
        printf("TEST[%d]: BYE message received from %d\n", dp->node, m.from);
    }
    return true;
}

int main(int argc, char **argv)
{
    string configFile = argc > 1 ? argv[1] : "config.data";
    World w;
    w.registerWorkFunction("BULLY", workFunction_BULLY);
    if (w.parseConfig(configFile)) {
        this_thread::sleep_for(chrono::milliseconds(3000000));
	} else {
        printf("can't open file '%s'\n", configFile.c_str());
    }
}

