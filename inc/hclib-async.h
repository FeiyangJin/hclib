/* Copyright (c) 2016, Rice University

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1.  Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
3.  Neither the name of Rice University
     nor the names of its contributors may be used to endorse or
     promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * hclib-async.h
 *  
 *      Authors: Vivek Kumar (vivekk@rice.edu), Max Grossman (jmg3@rice.edu)
 *      Acknowledgments: https://wiki.rice.edu/confluence/display/HABANERO/People
 */
#include <functional>
#include <vector>

#include "hclib.h"
#include "hclib-async-struct.h"
#include "hclib_promise.h"
#include "hclib_future.h"
#include "aaa_c_connector.h"
#include "hclib-finish.h"


#ifndef HCLIB_ASYNC_H_
#define HCLIB_ASYNC_H_

namespace hclib {
static int task_id_unique = 0;

// void insert_DPST(enum node_type nodeType, hclib_task_t *task){
//     // insert a tree_node to DPST
//     // do not use this to insert a step node
//     tree_node *node= newtreeNode();
//     node->this_node_type = nodeType;
//     node->task = task;

//     if(nodeType == ROOT){
//         node->depth = 0;
//         node->parent = NULL;
//         DPST.root = node;
//     }
//     else{
//         hclib_worker_state *ws = current_ws();
//         hclib_task_t *curr_task = (hclib_task_t *)ws->curr_task;

//         // each task corresponds to an async or a future tree node
//         node->parent = curr_task->node_in_dpst;
//         node->depth = node->parent->depth + 1;

//         if(node->parent->children_list_head == NULL){
//             node->parent->children_list_head = node;
//             node->parent->children_list_tail = node;
//         }
//         else{
//             node->parent->children_list_tail->next_sibling = node;
//             node->parent->children_list_tail = node;
//         }   
//     }

//     task->node_in_dpst = node;
    

//     //DPST.current_tree_node = node;
//     //printDPST();
// }

/*
 * The C API to the HC runtime defines a task at its simplest as a function
 * pointer paired with a void* pointing to some user data. This file adds a C++
 * wrapper over that API by passing the C API a lambda-caller function and a
 * pointer to the lambda stored on the heap, which are then called.
 *
 * This does add more overheads in the C++ version (i.e. memory allocations).
 * TODO optimize that overhead.
 */

/*
 * At the lowest layer in the call stack before entering user code, this method
 * invokes the user-provided lambda.
 */
template <typename T>
inline void call_lambda(T* lambda) {
	const int wid = current_ws()->id;
	MARK_BUSY(wid);
	(*lambda)();
    delete lambda;
	MARK_OVH(wid);
}

/*
 * Call a lambda and place the output into a promise object.
 */
template <typename T, typename R>
struct call_and_put_wrapper {
    static void fn(T lambda, hclib::promise_t<R> *event) {
        event->put(lambda());
    }
};

/*
 * Specialize call_and_put_wrapper for void lambdas that don't return anything.
 */
template <typename T>
struct call_and_put_wrapper<T, void> {
    static void fn(T lambda, hclib::promise_t<void> *event) {
        lambda();
        event->put();
    }
};

/*
 * Store a reference to the type-specific function for calling the user lambda,
 * as well as a pointer to the lambda's location on the heap (through which we
 * can invoke it). async_arguments is stored as the args field in the task_t
 * object for a task, and passed to lambda_wrapper.
 */
template <typename Function, typename T1>
struct async_arguments {
    Function lambda_caller;
    T1 *lambda_on_heap;

    async_arguments(Function k, T1 *a) :
        lambda_caller(k), lambda_on_heap(a) { }
};

/*
 * The method called directly from the HC runtime, passed a pointer to an
 * async_arguments object. It then uses these async_arguments to call
 * call_lambda, passing the user-provided lambda.
 */
template<typename Function, typename T1>
void lambda_wrapper(void *args) {
    async_arguments<Function, T1> *a =
        (async_arguments<Function, T1> *)args;

    (*a->lambda_caller)(a->lambda_on_heap);
}

/*
 * Initialize a task_t for the C++ APIs, using a user-provided lambda.
 */
template<typename Function, typename T1>
inline hclib_task_t *initialize_task(Function lambda_caller, T1 *lambda_on_heap) {
    hclib_task_t *t = (hclib_task_t *)calloc(1, sizeof(*t));
    assert(t && lambda_on_heap);
    async_arguments<Function, T1> *args =
        new async_arguments<Function, T1>(lambda_caller, lambda_on_heap);
    t->_fp = lambda_wrapper<Function, T1>;
    t->args = args;

    t->task_id = task_id_unique;
    if(task_id_unique == 0){
        printf("main task created here \n");
        t->parent = NULL;

        // insert to DPST
        tree_node *the_node = insert_tree_node(ROOT,NULL);
        t->node_in_dpst = the_node;
        the_node->task = t;

        // disjoint set operation
        ds_addSet(task_id_unique);
        ds_addtask(task_id_unique,-1,the_node,t,0);
    }
    else{
        hclib_worker_state *ws = current_ws();
        hclib_task_t *curr_task = (hclib_task_t *)ws->curr_task;
        t->parent = curr_task;

        // disjoint set operation
        ds_addSet(task_id_unique);
        int parent_id = t->parent->task_id;
        int parent_nt_counts = ds_ntcounts(parent_id);
        if(parent_nt_counts > 0){
            ds_setlsa(task_id_unique,parent_id);
        }
        else{
            int parent_lsa = ds_getlsa(parent_id);
            ds_setlsa(task_id_unique,parent_lsa);
        }
    }
    
    task_id_unique ++;
    return t;
}

/*
 * lambda_on_heap is expected to be off-stack storage for a lambda object
 * (including its captured variables), which will be pointed to from the task_t.
 */
template <typename T>
inline hclib_task_t* _allocate_async(T *lambda) {
    // create off-stack storage for this task
    T *lambda_on_heap = (T *)malloc(sizeof(*lambda_on_heap));
    assert(lambda_on_heap);
    memcpy(lambda_on_heap, lambda, sizeof(*lambda_on_heap));

    hclib_task_t *task = initialize_task(call_lambda<T>, lambda_on_heap);
	return task;
}

template <typename T>
inline void async_await_at_helper(T&& lambda, hclib_future_t **futures,
        const int nfutures, hclib_locale_t *locale, const int non_blocking) {
    MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));
    task->non_blocking = non_blocking;
    spawn_await_at(task, futures, nfutures, locale);
}

template <typename T>
inline void async(T &&lambda) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t *task = initialize_task(call_lambda<U>, new U(lambda));

    // fj
    hclib_worker_state *ws = current_ws();
    hclib_task_t *curr_task = (hclib_task_t *)ws->curr_task;
    tree_node *the_node;
    // printf("current finish is null %s \n", curr_task->current_finish == NULL ? "true" : "false");
    // printf("%d %d \n",ws->current_finish->belong_to_task_id, curr_task->task_id);
    if(ws->current_finish != NULL && ws->current_finish->belong_to_task_id == curr_task->task_id){
        the_node = insert_tree_node(ASYNC,ws->current_finish->node_in_dpst);
    }
    else{
        the_node = insert_tree_node(ASYNC,curr_task->node_in_dpst);
    }
    
    task->node_in_dpst = the_node;
    the_node->task = task;

    // insert two step nodes as child and sibling
    insert_leaf(task->node_in_dpst->parent);
    insert_leaf(task->node_in_dpst);

    ds_addtask(task->task_id,curr_task->task_id,the_node,task,0);

    spawn(task);
}

template <typename T>
inline void async_at(T&& lambda, hclib_locale_t *locale) {
    MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    spawn_at(initialize_task(call_lambda<U>, new U(lambda)), locale);
}

template <typename T>
inline void async_nb(T&& lambda) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t *task = initialize_task(call_lambda<U>, new U(lambda));
    task->non_blocking = 1;
	spawn(task);
}

template <typename T>
inline void async_nb_at(T&& lambda, hclib_locale_t *locale) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t *task = initialize_task(call_lambda<U>, new U(lambda));
    task->non_blocking = 1;
	spawn_at(task, locale);
}

template <typename T>
inline void async_nb_await(T&& lambda, hclib_future_t *future) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
	hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));
    task->non_blocking = 1;
	spawn_await(task, future ? &future : NULL, future ? 1 : 0);
}

template <typename T>
inline void async_nb_await(T&& lambda, std::vector<hclib_future_t *> &futures) {
    async_await_at_helper(lambda, futures.data(), futures.size(), nullptr, 1);
}

template <typename T>
inline void async_nb_await(T&& lambda, std::vector<hclib_future_t *> &&futures) {
    async_await_at_helper(lambda, futures.data(), futures.size(), nullptr, 1);
}

template <typename T>
inline void async_nb_await_at(T&& lambda, hclib_future_t *fut,
        hclib_locale_t *locale) {
    MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t *task = initialize_task(call_lambda<U>, new U(lambda));
    task->non_blocking = 1;
    spawn_await_at(task, fut ? &fut : NULL, fut ? 1 : 0, locale);
}

template <typename T>
inline void async_nb_await_at(T&& lambda, std::vector<hclib_future_t *> &futures,
        hclib_locale_t *locale) {
    async_await_at_helper(lambda, futures.data(), futures.size(), locale, 1);
}

template <typename T>
inline void async_nb_await_at(T&& lambda, std::vector<hclib_future_t *> &&futures,
        hclib_locale_t *locale) {
    async_await_at_helper(lambda, futures.data(), futures.size(), locale, 1);
}

template <typename T>
inline void async_nb_await_at(T&& lambda, std::vector<hclib_future_t *> *futures,
        hclib_locale_t *locale) {
    async_await_at_helper(lambda, futures ? futures->data() : nullptr,
            futures ? futures->size() : 0, locale, 1);
}

template <typename T>
inline void async_nb_await(T&& lambda, std::vector<hclib_future_t *> *futures) {
    async_await_at_helper(lambda, futures ? futures->data() : nullptr,
            futures ? futures->size() : 0, nullptr, 1);
}

template <typename T>
inline void async_await(T&& lambda, hclib_future_t *future) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));
	spawn_await(task, future ? &future : NULL, future ? 1 : 0);
}

template <typename T>
inline void async_await(T&& lambda, hclib_future_t *future1,
        hclib_future_t *future2) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));

    int nfutures = 0;
    hclib_future_t *futures[2];
    if (future1) {
        futures[nfutures++] = future1;
    }
    if (future2) {
        futures[nfutures++] = future2;
    }

	spawn_await(task, futures, nfutures);
}

template <typename T>
inline void async_await(T&& lambda, hclib_future_t *future1,
        hclib_future_t *future2, hclib_future_t *future3,
        hclib_future_t *future4) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));

    int nfutures = 0;
    hclib_future_t *futures[4];
    if (future1) futures[nfutures++] = future1;
    if (future2) futures[nfutures++] = future2;
    if (future3) futures[nfutures++] = future3;
    if (future4) futures[nfutures++] = future4;

    spawn_await(task, futures, nfutures);
}

template <typename T>
inline void async_await(T&& lambda, std::vector<hclib_future_t *> &futures) {
    async_await_at_helper(lambda, futures.data(), futures.size(), nullptr, 0);
}

template <typename T>
inline void async_await(T&& lambda, std::vector<hclib_future_t *> &&futures) {
    async_await_at_helper(lambda, futures.data(), futures.size(), nullptr, 0);
}

template <typename T>
inline void async_await(T&& lambda, std::vector<hclib_future_t *> *futures) {
    async_await_at_helper(lambda, futures ? futures->data() : NULL,
            futures ? futures->size() : 0, nullptr, 0);
}

template <typename T>
inline void async_await_at(T&& lambda, hclib_future_t *future,
        hclib_locale_t *locale) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));
	spawn_await_at(task, future ? &future : NULL, future ? 1 : 0,
            locale);
}

template <typename T>
inline void async_await_at(T&& lambda, hclib_future_t *future1,
        hclib_future_t *future2, hclib_locale_t *locale) {
	MARK_OVH(current_ws()->id);
    typedef typename std::remove_reference<T>::type U;
    hclib_task_t* task = initialize_task(call_lambda<U>, new U(lambda));

    int nfutures = 0;
    hclib_future_t *futures[2];
    if (future1) {
        futures[nfutures++] = future1;
    }
    if (future2) {
        futures[nfutures++] = future2;
    }

	spawn_await_at(task, futures, nfutures, locale);
}

template <typename T>
inline void async_await_at(T&& lambda, std::vector<hclib_future_t *> &futures,
        hclib_locale_t *locale) {
    async_await_at_helper(lambda, futures.data(), futures.size(), locale, 0);
}

template <typename T>
inline void async_await_at(T&& lambda, std::vector<hclib_future_t *> &&futures,
        hclib_locale_t *locale) {
    async_await_at_helper(lambda, futures.data(), futures.size(), locale, 0);
}

/*
 * Some CUDA compilers trip over the following line:
 *
 *      call_and_put_wrapper<T, R>::fn(lambda, event);
 *
 * so we disable this code if we're compiling a CUDA file with nvcc.
 */
template <typename T>
auto async_future_await_at_helper(T&& lambda, hclib_future_t **futures,
        const int nfutures, hclib_locale_t *locale,
        const int non_blocking) -> hclib::future_t<decltype(lambda())>* {
    MARK_OVH(current_ws()->id);
    typedef decltype(lambda()) R;

    hclib::promise_t<R> *event = new hclib::promise_t<R>();
    /*
     * TODO creating this closure may be inefficient. While the capture list is
     * precise, if the user-provided lambda is large then copying it by value
     * will also take extra time.
     */
    auto wrapper = [event, lambda]() {
        call_and_put_wrapper<T, R>::fn(lambda, event);
    };
    typedef decltype(wrapper) U;

    hclib_task_t* task = initialize_task(call_lambda<U>, new U(wrapper));
    task->non_blocking = non_blocking;
    spawn_await_at(task, futures, nfutures, locale);
    return event->get_future();
}

template <typename T>
auto async_future(T&& lambda) -> hclib::future_t<decltype(lambda())>* {
    typedef decltype(lambda()) R;

    hclib::promise_t<R> *event = new hclib::promise_t<R>();
    /*
     * TODO creating this closure may be inefficient. While the capture list is
     * precise, if the user-provided lambda is large then copying it by value
     * will also take extra time.
     */
    auto wrapper = [event, lambda]() {
        call_and_put_wrapper<T, R>::fn(lambda, event);
    };
    typedef decltype(wrapper) U;

    hclib_task_t* task = initialize_task(call_lambda<U>, new U(wrapper));

    // fj: insert a FUTURE node to DPST, and a step node as child
    hclib_worker_state *ws = current_ws();
    hclib_task_t *curr_task = (hclib_task_t *)ws->curr_task;
    tree_node *the_node;
    if(ws->current_finish != NULL && ws->current_finish->belong_to_task_id == curr_task->task_id){
        the_node = insert_tree_node(FUTURE,ws->current_finish->node_in_dpst);
    }
    else{
        the_node = insert_tree_node(FUTURE,curr_task->node_in_dpst);
    }
    
    task->node_in_dpst = the_node;
    the_node->task = task;

    // insert two step nodes as child and sibling
    insert_leaf(task->node_in_dpst->parent);
    insert_leaf(task->node_in_dpst);

    // fj: connect the future with the task
    event->get_future()->corresponding_task_id = task->task_id;

    // fj: disjoint set operation
    ds_addtask(task->task_id,curr_task->task_id,the_node,task,0);

    spawn(task);
    return event->get_future();
}

template <typename T>
auto async_nb_future(T&& lambda) -> hclib::future_t<decltype(lambda())>* {
    typedef decltype(lambda()) R;

    hclib::promise_t<R> *event = new hclib::promise_t<R>();
    /*
     * TODO creating this closure may be inefficient. While the capture list is
     * precise, if the user-provided lambda is large then copying it by value
     * will also take extra time.
     */
    auto wrapper = [event, lambda]() {
        call_and_put_wrapper<T, R>::fn(lambda, event);
    };
    typedef decltype(wrapper) U;

    hclib_task_t* task = initialize_task(call_lambda<U>, new U(wrapper));
    task->non_blocking = 1;
    spawn(task);
    return event->get_future();
}


template <typename T>
auto async_future_await(T&& lambda, hclib_future_t *future) ->
        hclib::future_t<decltype(lambda())>* {
    typedef decltype(lambda()) R;

    hclib::promise_t<R> *event = new hclib::promise_t<R>();
    /*
     * TODO creating this closure may be inefficient. While the capture list is
     * precise, if the user-provided lambda is large then copying it by value
     * will also take extra time.
     */
    auto wrapper = [event, lambda]() {
        call_and_put_wrapper<T, R>::fn(lambda, event);
    };
    typedef decltype(wrapper) U;

    hclib_task_t* task = initialize_task(call_lambda<U>, new U(wrapper));
    spawn_await(task, future ? &future : NULL, future ? 1 : 0);
    return event->get_future();
}

#ifndef __CUDACC__
template <typename T>
auto async_future_await(T&& lambda, std::vector<hclib_future_t *> &futures) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, futures.data(), futures.size(),
            nullptr, 0);
}

template <typename T>
auto async_future_await(T&& lambda, std::vector<hclib_future_t *> &&futures) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, futures.data(), futures.size(),
            nullptr, 0);
}

template <typename T>
auto async_nb_future_await(T&& lambda, hclib_future_t * future) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, &future, 1, nullptr, 1);
}

template <typename T>
auto async_nb_future_await(T&& lambda, std::vector<hclib_future_t *> &futures) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, futures.data(), futures.size(),
            nullptr, 1);
}


template <typename T>
auto async_nb_future_await(T&& lambda, std::vector<hclib_future_t *> &&futures) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, futures.data(), futures.size(),
            nullptr, 1);
}



template <typename T>
auto async_future_at_helper(T& lambda, hclib_locale_t *locale,
        bool nb) -> hclib::future_t<decltype(lambda())>* {
    typedef decltype(lambda()) R;

    hclib::promise_t<R> *event = new hclib::promise_t<R>();
    /*
     * TODO creating this closure may be inefficient. While the capture list is
     * precise, if the user-provided lambda is large then copying it by value
     * will also take extra time.
     */
    auto wrapper = [event, lambda]() {
        call_and_put_wrapper<T, R>::fn(lambda, event);
    };
    typedef decltype(wrapper) U;

    hclib_task_t* task = initialize_task(call_lambda<U>, new U(wrapper));
    if (nb) task->non_blocking = 1;
    spawn_await_at(task, NULL, 0, locale);
    return event->get_future();
}

template <typename T>
auto async_future_at(T&& lambda, hclib_locale_t *locale) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_at_helper<T>(lambda, locale, false);
}

template <typename T>
auto async_nb_future_at(T &&lambda, hclib_locale_t *locale) ->
        hclib::future_t<decltype(lambda())>* {
    return async_future_at_helper<T>(lambda, locale, true);
}

template <typename T>
auto async_future_await_at(T&& lambda, hclib_future_t *future,
        hclib_locale_t *locale) -> hclib::future_t<decltype(lambda())>* {
    typedef decltype(lambda()) R;

    hclib::promise_t<R> *event = new hclib::promise_t<R>();
    /*
     * TODO creating this closure may be inefficient. While the capture list is
     * precise, if the user-provided lambda is large then copying it by value
     * will also take extra time.
     */
    auto wrapper = [event, lambda]() {
        call_and_put_wrapper<T, R>::fn(lambda, event);
    };
    typedef decltype(wrapper) U;

    hclib_task_t* task = initialize_task(call_lambda<U>, new U(wrapper));
    spawn_await_at(task, future ? &future : NULL, future ? 1 : 0,
            locale);
    return event->get_future();
}

template <typename T>
auto async_future_await_at(T&& lambda, std::vector<hclib_future_t *> &futures,
        hclib_locale_t *locale) -> hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, futures.data(), futures.size(), locale, 0);
}

template <typename T>
auto async_future_await_at(T&& lambda, std::vector<hclib_future_t *> &&futures,
        hclib_locale_t *locale) -> hclib::future_t<decltype(lambda())>* {
    return async_future_await_at_helper(lambda, futures.data(), futures.size(), locale, 0);
}
#endif

inline void finish(std::function<void()> &&lambda) {
    hclib_start_finish();
    lambda();
    hclib_end_finish();
}

inline hclib::future_t<void> *nonblocking_finish(
        std::function<void()> &&lambda) {
    hclib_start_finish();
    lambda();
    hclib::promise_t<void> *event = new hclib::promise_t<void>();
    hclib_end_finish_nonblocking_helper(event);
    return event->get_future();
}

inline void yield() {
    hclib_yield(NULL);
}

inline void yield_at(hclib_locale_t *locale) {
    hclib_yield(locale);
}

}

#endif /* HCLIB_ASYNC_H_ */
