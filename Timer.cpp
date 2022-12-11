//============================================================================
// Name        : Timer.cpp
// Author      : Xu Zihang
// Version     : Singel thread (std:C++17)
// Copyright   : Do not copy without permission
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <chrono>
#include <functional>
#include <set>
#include <memory>
// #include <sys/epoll.h>

using namespace std;

struct TimerNodeBase {
	time_t expire;
	int64_t id;
};

struct TimerNode : public TimerNodeBase {
	using Callback = std::function<void(const TimerNode &node)>;
	Callback func;
	//定时器扩展功能添加在这，比如永久定时任务，不需要反复的删除和添加，可以添加 int16_t flag等
	//在后面的逻辑中进行判断
};

bool operator < (const TimerNodeBase &leftNode, const TimerNodeBase &rightNode) {
	if(leftNode.expire < rightNode.expire)  return true;
	if(leftNode.expire > rightNode.expire)	return false;
	return leftNode.id < rightNode.id;
}

class Timer {
public:
	static time_t GetTick() {
		auto sc = chrono::time_point_cast<chrono::milliseconds>(chrono::steady_clock::now());
		auto temp = chrono::duration_cast<chrono::milliseconds>(sc.time_since_epoch());
		return temp.count();
	}
	static uint64_t GenID() {
		return ++gid;
	}
	TimerNodeBase AddTimer(time_t msec, TimerNode::Callback func) {
		TimerNode tnode;
		tnode.expire = GetTick() + msec;
		tnode.func = func;
		tnode.id = GenID();
		timermap.insert(tnode);
		return static_cast<TimerNodeBase>(tnode);
	}
	bool DelTimer(TimerNodeBase &node) {
		auto iter = timermap.find(node);
		if(iter != timermap.end()) {
			timermap.erase(iter);
			return true;
		}
		return false;
	}
	bool CheckTimer() {
		auto iter = timermap.begin();
		if(iter != timermap.end() && iter->expire <= GetTick()) {
			iter->func(*iter);
			timermap.erase(iter);
			return true;
		}
		return false;
	}
	time_t TimeToSleep() {
		auto iter = timermap.begin();
		if(iter == timermap.end()) {
			return -1;
		}
		time_t dis = iter->expire-GetTick();
		return dis > 0? dis : 0;
	}

private:
	static uint64_t gid;
	set<TimerNode, less<>> timermap;
};
uint64_t Timer::gid = 0;

int main() {
	unique_ptr<Timer> timer = make_unique<Timer>();
	/*测试代码
	int i=0;
	timer -> AddTimer(1000, [&](const TimerNode &node){
		cout<<"node id:"<<node.id<<" sysclock:"<<Timer::GetTick()<<" ex_time"<<i<<endl;
	});
	timer -> AddTimer(1000, [&](const TimerNode &node){
			cout<<"node id:"<<node.id<<" sysclock:"<<Timer::GetTick()<<" ex_time"<<i<<endl;
		});
	auto myNode = timer -> AddTimer(5000, [&](const TimerNode &node){
			cout<<"node id:"<<node.id<<" sysclock:"<<Timer::GetTick()<<" ex_time"<<i<<endl;
		});
	timer -> AddTimer(5000, [&](const TimerNode &node){
			cout<<"node id:"<<node.id<<" sysclock:"<<Timer::GetTick()<<" ex_time"<<i<<endl;
		});
	timer->DelTimer(myNode);
	//e.x. 与网络模块协同，同时处理网络事件和服务器定时事件
	int epfd = epoll_create(1);
	epoll_event ev[64] = {0};
	while(true) {
		int32_t n = epoll_wait(epfd, ev, 64, timer->TimeToSleep());
		for(int i=0;i<n;i++){
			//do something
		}
		while(timer->CheckTimer());
	}
	*/

	return 0;
}
