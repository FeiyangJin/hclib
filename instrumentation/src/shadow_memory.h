#include <unordered_map>
#include <vector>
#include <set>

using namespace std;

typedef struct access_info{
    int hclib_task_id;
    void* task_step;
};

class shadow_memory{
    private:
        unordered_map<int*,access_info> location_last_writer;

        unordered_map<int*,unordered_map<int,void*>> location_readers;

    public:
        shadow_memory();
        void updateWriter(int* location, access_info new_writer);
        void removeReader(int* location, int old_reader_id);
        void addReader(int* location, access_info new_reader);
        void printhello();
};