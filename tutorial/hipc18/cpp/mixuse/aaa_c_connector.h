#ifndef AAA_C_CONNECTOR_H 
#define AAA_C_CONNECTOR_H 

#ifdef __cplusplus
extern "C" {
#endif
 
void addSet(int set_index);
int findSet(int element);
void unionSet(int a, int b);
void printElementSets(int *element_array);
void merge(int a, int b);
void addnt(int task, int nt_task_id);
int ntcounts(int task_id);
int getlsa(int task_id);
void setlsa(int task_id, int lsa);
void printAll();
void printdsbyset();

#ifdef __cplusplus
}
#endif


#endif