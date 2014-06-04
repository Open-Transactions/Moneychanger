//
//  BitMessageQueue.cpp
//

#include "BitMessageQueue.h"

#include<boost/tokenizer.hpp>



bool BitMessageQueue::start() {
    
    if(m_stop){
        m_stop = false;
        m_thread = OT_THREAD(&BitMessageQueue::run, this);
        return true;
    }
    else{
        std::cout << "BitMessageQueue is already running!" << std::endl;
        return false;
    }
}


bool BitMessageQueue::stop() {
    
    if(!m_stop){
        INSTANTIATE_MLOCK(m_processing); // Don't stop the thread in the middle of processing
        m_stop = true;
        m_thread.join();
        mlock.unlock();
        return true;
    }
    else{
        std::cout << "BitMessageQueue is already stopped!" << std::endl;
        return false;
    }
}


bool BitMessageQueue::processing(){
    
    return OT_ATOMIC_ISTRUE(m_working);
    
}


void BitMessageQueue::addToQueue(OT_STD_FUNCTION(void()) command){
    
    MasterQueue.push(command);
    
}




int BitMessageQueue::queueSize(){
    
    return MasterQueue.size();
    
}


void BitMessageQueue::clearQueue(){
    
    MasterQueue.clear();
    
}




bool BitMessageQueue::parseNextMessage(){
    
    if(queueSize() == 0){
        return false;
    }
    
    INSTANTIATE_MLOCK(m_processing);  // Don't let other functions interfere with our message parsing
    
    //    m_working = true; // Notify our atomic boolean that we are in the middle of a process
    
    OT_STD_FUNCTION(void()) message = MasterQueue.pop();  // Pull out our function to run
    
    message();
    
    mlock.unlock();
    m_conditional.notify_one(); // Let other functions know that we're done and they can continue.
    // This is primarily for when a request comes in to shut down the queue
    // While an action is in progress. This will notify our stop handler that it is safe
    // To shut down the thread.
    
    //    m_working = false; // Notify our atomic boolean that we are done with our processing
    
    
    return true;
}


BitMessageQueue::~BitMessageQueue(){
    
    try{
        stop();
    }
    
    catch(...){
        /* Will need to refactor this */
    }
    
}