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
void printAll();

#ifdef __cplusplus
}
#endif


#endif