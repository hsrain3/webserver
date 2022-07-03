//
// Created by 王澄雨 on 2022/6/28.
//

#include "heaptimer.h"

bool HeapTimer::siftdown(size_t index, size_t n) {
    assert(index >= 0 && index < heap.size());
    assert(n >= 1&&n <= heap.size());
    size_t i = index;
    size_t j = i*2 + 1;
    while(j < n) {
        if(j + 1< n&&heap[j + 1] < heap[j]) j ++;
        if(heap[i] < heap[j]) break;
        swapNode(i,j);
        i = j;
        j = i*2 + 1;
    }
    return i > index;
}

void HeapTimer::siftup(size_t i) {
    assert(i >= 0&&i < heap.size());
    size_t j = (i-1) / 2;
    while(j >= 0) {
        if(heap[j] < heap[i]) {break;}
        swapNode(i,j);
        i = j;
        j = (i-1)/2;
    }

}

void HeapTimer::swapNode(size_t i, size_t j) {
    assert(i >= 0&&j < heap.size());
    assert(j >= 0&& j < heap.size());
    std::swap(heap[i],heap[j]);
    ref[heap[i].id] = i;
    ref[heap[j].id] = j;
}

void HeapTimer::add(int id, int timeOut, const TimeoutCallback &cb) {
    assert(id >= 0);
    size_t i;
    if(ref.count(id) == 0) {
        //new node
        i = heap.size();
        ref[id] = i;
        heap.push_back({id,Clock ::now() + MS(timeOut)});
        siftup(i);
    } else {
        //已有节点，更新expire 调整堆
        i = ref[id];
        heap[i].expires = Clock::now() + MS(timeOut);
        heap[i].cb = cb;
        if(!siftdown(i,heap.size())) {
            siftup(i);
        }
    }
}
void HeapTimer::doWork(int id) {
    //删除指定id，并触发回调函数
    if(heap.empty()||ref.count(id) == 0) return;
    size_t i = ref[id];
    TimerNode node = heap[i];
    node.cb();
    del(i);
}

void HeapTimer:del(size_t index) {
    assert(!heap.empty() && index >= 0&& index < heap.size());
    size_t i = index;
    size_t n = heap.size() - 1;
    assert(i <= n);
    if(i < n) {
        swapNode(i,n);
        if(!siftdown(i,n)) {
            siftup(i);
        }
    }
    ref.erase(heap.back().id);
    heap.pop_back();

}

void HeapTimer::adjust(int id, int newExpires) {
    //调整指定id的节点
    assert(!heap.empty()&&ref.count(id)>0);
    heap[ref[id]].expires = Clock::now() + MS(newExpires);
    siftdown(ref[id],heap.size());

}

void HeapTimer::tick() {
    //清除超时节点
    if(heap.empty()) return;
    while(!heap.empty()) {
        TimerNode node = heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.cb();
        pop();
    }
}

void HeapTimer::pop() {
    assert(!heap.empty());
    del(0);
}

void HeapTimer::clear() {
    ref.clear();
    heap.clear();
}

int HeapTimer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap.empty()) {
        res = std::chrono::duration_cast<MS>(node.expires - Clock::now()).count();
        if(res < 0) {res = 0;} //?
    }
    return res;
}