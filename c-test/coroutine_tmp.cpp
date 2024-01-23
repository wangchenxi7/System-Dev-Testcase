#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std;

/**
* Define before create a user-level thread through the std:thread/std:jthread?
*/
struct task
{
    struct promise_type
    {
        task get_return_object() {  cout<<"get_return_object \n";  return {}; }
        std::suspend_never initial_suspend() { cout<<"initial_suspend \n";  return {}; }
        std::suspend_never final_suspend() noexcept { cout<<"final_suspend \n";  return {}; }
        void return_void() {cout<<"return_void \n"; }
        void unhandled_exception() {cout<<"unhandled_exception \n"; }
    };
};

 
auto switch_to_new_thread(std::jthread& out)
{
    struct awaitable
    {
        std::jthread* p_out;
        
        

        bool await_ready() { 
            std::cout << "Execute await_ready. \n";
            return false; 
        }
        void await_suspend(std::coroutine_handle<> h)
        {
            std::cout << "Execute await_suspend. \n";
            std::jthread& out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output jthread parameter not empty");




            // create a new std:jthread and start it.
            // start executing the coroutine

            // Create the second coroutine
            std::coroutine_handle<> h2;
            // h2.done() always lead  to Segmentation fault.
            if (!h2) { 
              std::cout << "coroutine h2 is non-null. \n";
            } else {
              std::cout << "coroutine h2 is done. \n";
            }

            out = std::jthread([h2] { h2.resume(); });
            std::cout << "New thread ID [2]: " << out.get_id() << '\n';
            if (!h2) {
              std::cout << "Destroy the coroutine h2. \n";
              //h2.destroy();  // any invovation after the h2.resume leads to Segmentation fault.

            } else {
              std::cout << "coroutine h2 is done. \n";
            }

            std::cout << "check #1 \n\n";


            // Create the second coroutine h3
            std::coroutine_handle<> h3;
            // h3.done() always lead  to Segmentation fault.
            if (!h3) { 
              std::cout << "coroutine h3 is non-null. \n";
            } else {
              std::cout << "coroutine h3 is done. \n";
            }

            out = std::jthread([h3] { h3.resume(); });  // the 2nd invocation leads to Segmentation fault
            std::cout << "New thread ID [2]: " << out.get_id() << '\n';
            if (!h3) {
              std::cout << "Destroy the coroutine h3. \n";
              //h3.destroy();  // any invovation after the h3.resume leads to Segmentation fault.

            } else {
              std::cout << "coroutine h3 is done. \n";
            }

            std::cout << "check #2 \n\n";


            out = std::jthread([h] { h.resume(); });  // ?? Use a coroutine to intialize std:thread ??
            std::cout << "New thread ID: " << p_out->get_id() << '\n';
            std::cout << "New thread ID [1]: " << out.get_id() << '\n'; // this is OK
            //h.destroy();

     
        }


        void await_resume() {
            std::cout << "Execute await_resume. \n";
        }
    };

    // Start executing from here.
    std::cout << "Into  struct awaitable. \n";

    // Current thread ID: thread::id of a non-executing thread
    std::cout << " Current thread ID: " << out.get_id() << '\n'; 

    // Leading to execute the previous defined functions.
    return awaitable{&out};
}
 


 
/**
* What's the conenction between the coroutine, std::coroutine_handle<> h, and the user-level thread, std:thread ?
*/
task resuming_on_new_thread(std::jthread& out)
{
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out);
    // awaiter destroyed here
    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}
 
int main()
{
    std::jthread out;
    resuming_on_new_thread(out);
}
