#include "hclib_cpp.h"
#include "Channel.h"
#include <vector>

template <typename T>
class ChannelReader{
    public:
        ChannelReader(hclib::promise_t<void*> position){
            this->position = position;
        };

    private:
        hclib::promise_t<void*> position;

};

int main(int argc, char **argv){    
	return 0;
}