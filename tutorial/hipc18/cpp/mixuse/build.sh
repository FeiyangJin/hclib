g++ -fpic -shared aaa.cpp -o libaaa.so

g++ -fpic -shared aaa_c_connector.cpp -L. -laaa -o libaaa_c_connector.so

gcc main.c -L. -laaa_c_connector -o c_aaa