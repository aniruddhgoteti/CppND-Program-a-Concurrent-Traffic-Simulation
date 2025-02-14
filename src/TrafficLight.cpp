#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

using std::chrono::high_resolution_clock;

/* Implementation of class "MessageQueue" */


TrafficLightPhase MessageQueue::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> ulck(_mutex);
    _condition.wait(ulck, [this] {return !_queue.empty();});

    auto msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

void MessageQueue::send(TrafficLightPhase &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        auto state = _messages.receive();
        if (state == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    auto t1 = high_resolution_clock::now();
    std::random_device rand_dev;
    std::uniform_int_distribution<int> dist(4000, 6000);
    double duration_of_cycle = dist(rand_dev);
 
    while(true)
    {
        auto t2 = high_resolution_clock::now();
        auto time_diff =  std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if(time_diff >= duration_of_cycle )
        {
            if (getCurrentPhase() == TrafficLightPhase::red){
                setCurrentPhase(TrafficLightPhase::green);
                std::cout << "Changing to green" << std::endl;
            }
            else
            {
                setCurrentPhase(TrafficLightPhase::red);
                std::cout << "Changing to red" << std::endl;
            }
            t1 = high_resolution_clock::now();
        }
        _messages.send(std::move(getCurrentPhase()));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}