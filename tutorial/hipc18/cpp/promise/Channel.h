#include "hclib_cpp.h"
#include <vector>

template <typename T> 
class Channel{
    template <typename S>
    class Payload{
        public: 
            T value;
            hclib::promise_t<void*> *next = new hclib::promise_t<void*>();
            //private:
            Payload(T v){
                value = v;
            };
    };

    private:
        hclib::promise_t<void*> *latest = new hclib::promise_t<void*>();
    
    public:
        std::vector<hclib::promise_t<T>*> getPromises(){
            if (latest == NULL){
                return new std::vector<hclib::promise_t<void>*>();
            }
            std::vector<hclib::promise_t<T>*> result;
            result.push_back(latest);
            return result;
        };

        void put(T value){
            Payload<T> *p = new Payload<T>(value);
            latest->put(p);
            latest = p->next;
        }

        void terminate(){
            latest->put(NULL);
            latest = NULL;
        }

};

// int main(int argc, char **argv){
//     Channel<int> *out = new Channel<int>();
//     out->put(10);

// 	return 0;
// }