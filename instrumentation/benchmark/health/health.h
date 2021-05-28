#ifndef _HEALTH_H
#define _HEALTH_H
/* random defines */
#define IA 16807
#define IM 2147483647
#define AM (1.0 / IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

struct Results {
   long hosps_number;
   long hosps_personnel;
   long total_patients;
   long total_in_village;
   long total_waiting;
   long total_assess;
   long total_inside;
   long total_time;
   long total_hosps_v;
};

struct Patient {
   int id;
   int32_t seed;
   int time;
   int time_left;
   int hosps_visited;
   struct Village *home_village;
   struct Patient *back;
   struct Patient *forward;
};
struct Hosp {
   int personnel;
   int free_personnel;
   struct Patient *waiting;
   struct Patient *assess;
   struct Patient *inside;
   struct Patient *realloc;
};

struct Village {
   int id;
   struct Village *back;
   struct Village *next;
   struct Village *forward;
   struct Patient *population;
   struct Hosp hosp;
   int level;
   int32_t  seed;
};

float my_rand(int32_t *seed);

struct Patient *generate_patient(struct Village *village);
void put_in_hosp(struct Hosp *hosp, struct Patient *patient);

void addList(struct Patient **list, struct Patient *patient);
void removeList(struct Patient **list, struct Patient *patient);

void check_patients_inside(struct Village *village);
void check_patients_waiting(struct Village *village);
void check_patients_realloc(struct Village *village);

void check_patients_assess_par(struct Village *village);

float get_num_people(struct Village *village);
float get_total_time(struct Village *village);
float get_total_hosps(struct Village *village);

struct Results get_results(struct Village *village);

void sim_village_par(struct Village *village);

extern int sim_level;

void read_input_data(char *filename);
void allocate_village( struct Village **capital, struct Village *back, struct Village *next, int level, int32_t vid);
void sim_village_main(struct Village *top);

int check_village(struct Village *top);

void check_patients_assess(struct Village *village);
void check_patients_population(struct Village *village);
void sim_village(struct Village *village);
void my_print(struct Village *village);

#endif